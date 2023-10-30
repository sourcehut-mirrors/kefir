/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/register_allocator.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/target/asm/amd64/xasmgen.h"

#define LABEL_FORMAT "_kefir_func_%s_label%" KEFIR_ID_FMT

struct instruction_argument_state {
    struct kefir_asm_amd64_xasmgen_operand base_operands[2];
    const struct kefir_asm_amd64_xasmgen_operand *operand;

    struct {
        kefir_bool_t present;
        const struct kefir_asm_amd64_xasmgen_operand *underlying_operand;
    } spill;
};

struct generator_state {
    struct kefir_amd64_xasmgen *xasmgen;
    const struct kefir_codegen_amd64_register_allocator *register_allocator;
    const struct kefir_codegen_amd64_stack_frame *stack_frame;
    struct {
        struct kefir_list stack;
        struct kefir_hashtreeset regs;
    } evicted;

    struct {
        struct kefir_hashtreeset vregs;
        struct kefir_hashtreeset regs;
    } alive;

    struct {
        struct kefir_hashtreeset vregs;
        struct kefir_hashtreeset regs;
    } current_instr;
};

static kefir_result_t remove_dead_regs(struct kefir_mem *mem, struct generator_state *state,
                                       kefir_size_t linear_position) {
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state->current_instr.vregs));
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state->current_instr.regs));

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&state->alive.vregs, &iter); res == KEFIR_OK;) {

        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, iter.entry);
        const struct kefir_codegen_amd64_register_allocation *reg_alloc;
        REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator, vreg_idx, &reg_alloc));

        if (reg_alloc->lifetime.end <= linear_position) {
            REQUIRE_OK(kefir_hashtreeset_delete(mem, &state->alive.vregs, iter.entry));
            if (reg_alloc->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER) {
                REQUIRE_OK(kefir_hashtreeset_delete(mem, &state->alive.vregs, reg_alloc->direct_reg));
            }
            res = kefir_hashtreeset_iter(&state->alive.vregs, &iter);
        } else {
            res = kefir_hashtreeset_next(&iter);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t update_live_regs(struct kefir_mem *mem, struct generator_state *state,
                                       kefir_size_t linear_position, const struct kefir_asmcmp_value *value) {
    const struct kefir_codegen_amd64_register_allocation *reg_alloc;

    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            REQUIRE_OK(
                kefir_codegen_amd64_register_allocation_of(state->register_allocator, value->vreg.index, &reg_alloc));
            REQUIRE(reg_alloc->lifetime.begin <= linear_position && reg_alloc->lifetime.end >= linear_position,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register lifetime"));
            REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.vregs, (kefir_hashtreeset_entry_t) value->vreg.index));
            REQUIRE_OK(
                kefir_hashtreeset_add(mem, &state->current_instr.vregs, (kefir_hashtreeset_entry_t) value->vreg.index));
            if (reg_alloc->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER) {
                REQUIRE_OK(
                    kefir_hashtreeset_add(mem, &state->alive.regs, (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                REQUIRE_OK(kefir_hashtreeset_add(mem, &state->current_instr.regs,
                                                 (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator,
                                                                          value->indirect.base.vreg, &reg_alloc));
                    REQUIRE(reg_alloc->lifetime.begin <= linear_position && reg_alloc->lifetime.end >= linear_position,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register lifetime"));
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.vregs,
                                                     (kefir_hashtreeset_entry_t) value->indirect.base.vreg));
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &state->current_instr.vregs,
                                                     (kefir_hashtreeset_entry_t) value->indirect.base.vreg));
                    if (reg_alloc->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER) {
                        REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.regs,
                                                         (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                        REQUIRE_OK(kefir_hashtreeset_add(mem, &state->current_instr.regs,
                                                         (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                    }
                    break;

                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t obtain_temporary_register(struct kefir_mem *mem, struct generator_state *state,
                                                kefir_asm_amd64_xasmgen_register_t *reg) {
    for (kefir_size_t i = 0; i < state->register_allocator->internal.num_of_gp_registers; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate =
            state->register_allocator->internal.gp_register_allocation_order[i];

        if (!kefir_hashtreeset_has(&state->current_instr.regs, (kefir_hashtreeset_entry_t) candidate) &&
            !kefir_hashtreeset_has(&state->alive.regs, (kefir_hashtreeset_entry_t) candidate) &&
            kefir_hashtreeset_has(&state->register_allocator->used_registers, (kefir_hashtreeset_entry_t) candidate)) {
            *reg = candidate;
            return KEFIR_OK;
        }
    }

    for (kefir_size_t i = 0; i < state->register_allocator->internal.num_of_gp_registers; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate =
            state->register_allocator->internal.gp_register_allocation_order[i];

        if (!kefir_hashtreeset_has(&state->current_instr.regs, (kefir_hashtreeset_entry_t) candidate)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(state->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(candidate)));
            REQUIRE_OK(kefir_hashtreeset_add(mem, &state->evicted.regs, (kefir_hashtreeset_entry_t) candidate));
            REQUIRE_OK(kefir_list_insert_after(mem, &state->evicted.stack, kefir_list_tail(&state->evicted.stack),
                                               (void *) (kefir_uptr_t) candidate));
            *reg = candidate;
            return KEFIR_OK;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t build_operand(struct kefir_mem *mem, struct generator_state *state,
                                    const struct kefir_asmcmp_value *value,
                                    struct instruction_argument_state *arg_state) {
    UNUSED(mem);

    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 asmcmp value type");

        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
            arg_state->operand =
                kefir_asm_amd64_xasmgen_operand_imm(&arg_state->base_operands[0], value->int_immediate);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
            arg_state->operand =
                kefir_asm_amd64_xasmgen_operand_immu(&arg_state->base_operands[1], value->uint_immediate);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            arg_state->operand = kefir_asm_amd64_xasmgen_operand_reg((kefir_asm_amd64_xasmgen_register_t) value->phreg);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER: {
            kefir_asm_amd64_xasmgen_register_t reg;
            const struct kefir_codegen_amd64_register_allocation *reg_alloc;
            REQUIRE_OK(
                kefir_codegen_amd64_register_allocation_of(state->register_allocator, value->vreg.index, &reg_alloc));
            switch (reg_alloc->type) {
                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register");

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                    switch (value->vreg.variant) {
                        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(reg_alloc->direct_reg, &reg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(reg_alloc->direct_reg, &reg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(reg_alloc->direct_reg, &reg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(reg_alloc->direct_reg, &reg));
                            break;
                    }
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_reg(reg);
                    break;

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                    switch (value->vreg.variant) {
                        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                            arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                                &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_BYTE,
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &arg_state->base_operands[1],
                                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                    state->stack_frame->offsets.spill_area +
                                        reg_alloc->spill_area_index * KEFIR_AMD64_ABI_QWORD));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                            arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                                &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_WORD,
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &arg_state->base_operands[1],
                                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                    state->stack_frame->offsets.spill_area +
                                        reg_alloc->spill_area_index * KEFIR_AMD64_ABI_QWORD));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                            arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                                &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &arg_state->base_operands[1],
                                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                    state->stack_frame->offsets.spill_area +
                                        reg_alloc->spill_area_index * KEFIR_AMD64_ABI_QWORD));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                            arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                                &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &arg_state->base_operands[1],
                                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                    state->stack_frame->offsets.spill_area +
                                        reg_alloc->spill_area_index * KEFIR_AMD64_ABI_QWORD));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                            arg_state->operand = kefir_asm_amd64_xasmgen_operand_indirect(
                                &arg_state->base_operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                state->stack_frame->offsets.spill_area +
                                    reg_alloc->spill_area_index * KEFIR_AMD64_ABI_QWORD);
                            break;
                    }
                    break;
            }
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT: {
            const struct kefir_asm_amd64_xasmgen_operand *base_ptr = NULL;

            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS: {
                    const struct kefir_codegen_amd64_register_allocation *reg_alloc;
                    REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator,
                                                                          value->indirect.base.vreg, &reg_alloc));

                    switch (reg_alloc->type) {
                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register");

                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                            base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                                &arg_state->base_operands[0],
                                kefir_asm_amd64_xasmgen_operand_reg(reg_alloc->direct_reg), value->indirect.offset);
                            break;

                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA: {
                            kefir_asm_amd64_xasmgen_register_t reg;
                            REQUIRE_OK(obtain_temporary_register(mem, state, &reg));
                            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                                state->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                kefir_asm_amd64_xasmgen_operand_indirect(
                                    &arg_state->base_operands[0],
                                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                    state->stack_frame->offsets.spill_area +
                                        reg_alloc->spill_area_index * KEFIR_AMD64_ABI_QWORD)));

                            base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                                &arg_state->base_operands[0], kefir_asm_amd64_xasmgen_operand_reg(reg),
                                value->indirect.offset);
                        } break;
                    }
                } break;

                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0], kefir_asm_amd64_xasmgen_operand_reg(value->indirect.base.phreg),
                        value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        state->stack_frame->offsets.local_area + value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        state->stack_frame->offsets.spill_area +
                            value->indirect.base.spill_index * KEFIR_AMD64_ABI_QWORD + value->indirect.offset);
                    break;
            }

            switch (value->indirect.variant) {
                case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[1], KEFIR_AMD64_XASMGEN_POINTER_BYTE, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[1], KEFIR_AMD64_XASMGEN_POINTER_WORD, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[1], KEFIR_AMD64_XASMGEN_POINTER_DWORD, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[1], KEFIR_AMD64_XASMGEN_POINTER_QWORD, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                    arg_state->operand = base_ptr;
                    break;
            }
        } break;
    }

    return KEFIR_OK;
}

static kefir_result_t build_arg2(struct kefir_mem *mem, struct generator_state *state,
                                 struct instruction_argument_state instr_state[2],
                                 const struct kefir_asmcmp_instruction *instr) {
    REQUIRE_OK(build_operand(mem, state, &instr->args[0], &instr_state[0]));
    REQUIRE_OK(build_operand(mem, state, &instr->args[1], &instr_state[1]));

    if (instr_state[0].operand->klass == KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION &&
        instr_state[1].operand->klass == KEFIR_AMD64_XASMGEN_OPERAND_INDIRECTION) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(obtain_temporary_register(mem, state, &reg));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(state->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                                 instr_state[0].operand));

        instr_state[0].spill.present = true;
        instr_state[0].spill.underlying_operand = instr_state[0].operand;
        instr_state[0].operand = kefir_asm_amd64_xasmgen_operand_reg(reg);
    }
    return KEFIR_OK;
}

static kefir_result_t dispose_arg2(struct generator_state *state, struct instruction_argument_state instr_state[2]) {
    if (instr_state[0].spill.present) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(state->xasmgen, instr_state[0].spill.underlying_operand,
                                                 instr_state[0].operand));
    }
    return KEFIR_OK;
}

static kefir_result_t generate_instr(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen,
                                     const struct kefir_asmcmp_amd64 *target,
                                     const struct kefir_codegen_amd64_register_allocator *register_allocation,
                                     const struct kefir_codegen_amd64_stack_frame *stack_frame,
                                     struct generator_state *state, kefir_asmcmp_instruction_index_t index) {
    const struct kefir_asmcmp_instruction *instr;
    REQUIRE_OK(kefir_asmcmp_context_instr_at(&target->context, index, &instr));
    for (kefir_asmcmp_label_index_t label = kefir_asmcmp_context_instr_label_head(&target->context, index);
         label != KEFIR_ASMCMP_INDEX_NONE; label = kefir_asmcmp_context_instr_label_next(&target->context, label)) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, LABEL_FORMAT, target->function_name, label));
    }

    kefir_size_t instr_linear_position;
    REQUIRE_OK(
        kefir_codegen_amd64_register_allocator_linear_position_of(register_allocation, index, &instr_linear_position));

    REQUIRE_OK(remove_dead_regs(mem, state, instr_linear_position));

    struct instruction_argument_state arg_state[3] = {0};
    switch (instr->opcode) {
#define DEF_OPCODE_virtual(_opcode, _xasmgen)
#define DEF_OPCODE_arg0(_opcode, _xasmgen)                         \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                       \
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen)); \
        break;
#define DEF_OPCODE_arg2(_opcode, _xasmgen)                                                                     \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                   \
        REQUIRE_OK(update_live_regs(mem, state, instr_linear_position, &instr->args[0]));                      \
        REQUIRE_OK(update_live_regs(mem, state, instr_linear_position, &instr->args[1]));                      \
        REQUIRE_OK(build_arg2(mem, state, arg_state, instr));                                                  \
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen, arg_state[0].operand, arg_state[1].operand)); \
        REQUIRE_OK(dispose_arg2(state, arg_state));                                                            \
        break;
#define DEF_OPCODE(_opcode, _xasmgen, _argtp) DEF_OPCODE_##_argtp(_opcode, _xasmgen)

        KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, ;);
#undef DEF_OPCODE
#undef DEF_OPCODE_virtual
#undef DEF_OPCODE_arg0
#undef DEF_OPCODE_arg2

        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link): {
            const struct kefir_codegen_amd64_register_allocation *target_allocation, *source_allocation;
            REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(register_allocation, instr->args[0].vreg.index,
                                                                  &target_allocation));
            REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(register_allocation, instr->args[1].vreg.index,
                                                                  &source_allocation));

            if (!((target_allocation->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER &&
                   source_allocation->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER &&
                   target_allocation->direct_reg == source_allocation->direct_reg)) ||
                (target_allocation->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA &&
                 source_allocation->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA &&
                 target_allocation->spill_area_index == source_allocation->spill_area_index)) {
                REQUIRE_OK(update_live_regs(mem, state, instr_linear_position, &instr->args[0]));
                REQUIRE_OK(update_live_regs(mem, state, instr_linear_position, &instr->args[1]));
                REQUIRE_OK(build_arg2(mem, state, arg_state, instr));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(xasmgen, arg_state[0].operand, arg_state[1].operand));
                REQUIRE_OK(dispose_arg2(state, arg_state));
            }
        } break;

        case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(function_prologue):
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_prologue(xasmgen, target->abi_variant, register_allocation,
                                                                stack_frame));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(function_epilogue):
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_epilogue(xasmgen, target->abi_variant, register_allocation,
                                                                stack_frame));
            break;
    }

    for (const struct kefir_list_entry *iter = kefir_list_tail(&state->evicted.stack); iter != NULL;
         iter = kefir_list_tail(&state->evicted.stack)) {
        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, (kefir_uptr_t) iter->value);

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(state->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg)));

        REQUIRE_OK(kefir_list_pop(mem, &state->evicted.stack, (struct kefir_list_entry *) iter));
        REQUIRE_OK(kefir_hashtreeset_delete(mem, &state->evicted.regs, (kefir_hashtreeset_entry_t) reg));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_generate_code(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen,
                                                const struct kefir_asmcmp_amd64 *target,
                                                const struct kefir_codegen_amd64_register_allocator *register_allocator,
                                                const struct kefir_codegen_amd64_stack_frame *stack_frame) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 assembly generator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp amd64 target"));
    REQUIRE(register_allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocation"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    struct generator_state state = {
        .xasmgen = xasmgen, .register_allocator = register_allocator, .stack_frame = stack_frame};
    REQUIRE_OK(kefir_list_init(&state.evicted.stack));
    REQUIRE_OK(kefir_hashtreeset_init(&state.evicted.regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.alive.regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.alive.vregs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.current_instr.regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.current_instr.vregs, &kefir_hashtree_uint_ops));

    kefir_result_t res = KEFIR_OK;
    for (kefir_asmcmp_instruction_index_t idx = kefir_asmcmp_context_instr_head(&target->context);
         res == KEFIR_OK && idx != KEFIR_ASMCMP_INDEX_NONE;
         idx = kefir_asmcmp_context_instr_next(&target->context, idx)) {
        REQUIRE_CHAIN(&res, generate_instr(mem, xasmgen, target, register_allocator, stack_frame, &state, idx));
        REQUIRE_CHAIN_SET(
            &res, kefir_list_length(&state.evicted.stack) == 0,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                            "All evicted registers shall be restored after the instruction was generated"));
    }

    REQUIRE_ELSE(res == KEFIR_OK, { goto on_error6; });
    res = kefir_hashtreeset_free(mem, &state.current_instr.regs);
    REQUIRE_ELSE(res == KEFIR_OK, { goto on_error5; });
    res = kefir_hashtreeset_free(mem, &state.current_instr.vregs);
    REQUIRE_ELSE(res == KEFIR_OK, { goto on_error4; });
    res = kefir_hashtreeset_free(mem, &state.alive.regs);
    REQUIRE_ELSE(res == KEFIR_OK, { goto on_error3; });
    res = kefir_hashtreeset_free(mem, &state.alive.vregs);
    REQUIRE_ELSE(res == KEFIR_OK, { goto on_error2; });
    res = kefir_hashtreeset_free(mem, &state.evicted.regs);
    REQUIRE_ELSE(res == KEFIR_OK, { goto on_error1; });
    REQUIRE_OK(kefir_list_free(mem, &state.evicted.stack));

    return KEFIR_OK;

on_error6:
    kefir_hashtreeset_free(mem, &state.current_instr.regs);
on_error5:
    kefir_hashtreeset_free(mem, &state.current_instr.vregs);
on_error4:
    kefir_hashtreeset_free(mem, &state.alive.regs);
on_error3:
    kefir_hashtreeset_free(mem, &state.alive.vregs);
on_error2:
    kefir_hashtreeset_free(mem, &state.evicted.regs);
on_error1:
    kefir_list_free(mem, &state.evicted.stack);
    return res;
}
