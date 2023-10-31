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

#include "kefir/codegen/amd64/devirtualize.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/list.h"

struct devirtualize_state {
    struct kefir_asmcmp_amd64 *target;
    struct kefir_codegen_amd64_register_allocator *register_allocator;
    struct kefir_codegen_amd64_stack_frame *stack_frame;

    struct {
        struct kefir_hashtreeset physical_regs;
        struct kefir_hashtreeset virtual_regs;
        struct kefir_bitset spill_area;
    } alive;

    struct {
        struct kefir_hashtreeset physical_regs;
    } current_instr;

    struct {
        struct kefir_hashtree physical_regs;
    } evicted;
};

static kefir_result_t remove_dead_virtual_regs(struct kefir_mem *mem, struct devirtualize_state *state,
                                               kefir_size_t linear_position) {
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state->current_instr.physical_regs));

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&state->alive.virtual_regs, &iter); res == KEFIR_OK;) {

        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, iter.entry);
        const struct kefir_codegen_amd64_register_allocation *reg_alloc;
        REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator, vreg_idx, &reg_alloc));

        if (reg_alloc->lifetime.end <= linear_position) {
            REQUIRE_OK(kefir_hashtreeset_delete(mem, &state->alive.virtual_regs, iter.entry));
            switch (reg_alloc->type) {
                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register allocation");

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                    REQUIRE_OK(kefir_hashtreeset_delete(mem, &state->alive.physical_regs, reg_alloc->direct_reg));
                    break;

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                    REQUIRE_OK(kefir_bitset_set(&state->alive.spill_area, reg_alloc->spill_area_index, false));
                    break;
            }
            res = kefir_hashtreeset_iter(&state->alive.virtual_regs, &iter);
        } else {
            res = kefir_hashtreeset_next(&iter);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t update_live_virtual_regs(struct kefir_mem *mem, struct devirtualize_state *state,
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
            REQUIRE_OK(
                kefir_hashtreeset_add(mem, &state->alive.virtual_regs, (kefir_hashtreeset_entry_t) value->vreg.index));
            switch (reg_alloc->type) {
                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register allocation");

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.physical_regs,
                                                     (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &state->current_instr.physical_regs,
                                                     (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                    break;

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                    REQUIRE_OK(kefir_bitset_set(&state->alive.spill_area, reg_alloc->spill_area_index, true));
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator,
                                                                          value->indirect.base.vreg, &reg_alloc));
                    REQUIRE(reg_alloc->lifetime.begin <= linear_position && reg_alloc->lifetime.end >= linear_position,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register lifetime"));
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.virtual_regs,
                                                     (kefir_hashtreeset_entry_t) value->indirect.base.vreg));
                    switch (reg_alloc->type) {
                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                   "Unallocated amd64 virtual register allocation");

                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                            REQUIRE_OK(kefir_hashtreeset_add(mem, &state->alive.physical_regs,
                                                             (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                            REQUIRE_OK(kefir_hashtreeset_add(mem, &state->current_instr.physical_regs,
                                                             (kefir_hashtreeset_entry_t) reg_alloc->direct_reg));
                            break;

                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                            REQUIRE_OK(kefir_bitset_set(&state->alive.spill_area, reg_alloc->spill_area_index, true));
                            break;
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

static kefir_result_t obtain_temporary_register(struct kefir_mem *mem, struct devirtualize_state *state,
                                                kefir_asmcmp_instruction_index_t position,
                                                kefir_asm_amd64_xasmgen_register_t *reg) {
    for (kefir_size_t i = 0; i < state->register_allocator->internal.num_of_gp_registers; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate =
            state->register_allocator->internal.gp_register_allocation_order[i];

        if (!kefir_hashtreeset_has(&state->current_instr.physical_regs, (kefir_hashtreeset_entry_t) candidate) &&
            !kefir_hashtreeset_has(&state->alive.physical_regs, (kefir_hashtreeset_entry_t) candidate) &&
            !kefir_hashtree_has(&state->evicted.physical_regs, (kefir_hashtree_key_t) candidate) &&
            kefir_hashtreeset_has(&state->register_allocator->used_registers, (kefir_hashtreeset_entry_t) candidate)) {
            REQUIRE_OK(kefir_hashtree_insert(mem, &state->evicted.physical_regs, (kefir_hashtree_key_t) candidate,
                                             (kefir_hashtree_value_t) (kefir_size_t) -1));
            *reg = candidate;
            return KEFIR_OK;
        }
    }
    for (kefir_size_t i = 0; i < state->register_allocator->internal.num_of_gp_registers; i++) {
        const kefir_asm_amd64_xasmgen_register_t candidate =
            state->register_allocator->internal.gp_register_allocation_order[i];

        if (!kefir_hashtreeset_has(&state->current_instr.physical_regs, (kefir_hashtreeset_entry_t) candidate) &&
            !kefir_hashtree_has(&state->evicted.physical_regs, (kefir_hashtree_key_t) candidate)) {

            kefir_size_t temp_spill_slot;
            kefir_result_t res = kefir_bitset_find(&state->alive.spill_area, false, 0, &temp_spill_slot);
            if (res == KEFIR_NOT_FOUND) {
                kefir_size_t num_of_slots;
                REQUIRE_OK(kefir_bitset_length(&state->alive.spill_area, &num_of_slots));
                REQUIRE_OK(kefir_bitset_resize(mem, &state->alive.spill_area, ++num_of_slots));
                REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_spill_area(state->stack_frame, num_of_slots));
                REQUIRE_OK(kefir_bitset_find(&state->alive.spill_area, false, 0, &temp_spill_slot));
            } else {
                REQUIRE_OK(res);
            }
            REQUIRE_OK(kefir_bitset_set(&state->alive.spill_area, temp_spill_slot, true));

            REQUIRE_OK(kefir_hashtree_insert(mem, &state->evicted.physical_regs, (kefir_hashtree_key_t) candidate,
                                             (kefir_hashtree_value_t) temp_spill_slot));

            kefir_asmcmp_instruction_index_t new_position;
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, position),
                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(temp_spill_slot - 1, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                &KEFIR_ASMCMP_MAKE_PHREG(candidate), &new_position));
            REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, position));
            *reg = candidate;
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find a temporary register for eviction");
}

static kefir_result_t devirtualize_value(struct kefir_mem *mem, struct devirtualize_state *state,
                                         kefir_asmcmp_instruction_index_t position, struct kefir_asmcmp_value *value) {
    UNUSED(mem);
    const struct kefir_codegen_amd64_register_allocation *reg_alloc;
    kefir_asm_amd64_xasmgen_register_t phreg;
    kefir_asmcmp_instruction_index_t new_position;

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

            switch (reg_alloc->type) {
                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register allocation");

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                    switch (value->vreg.variant) {
                        case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(reg_alloc->direct_reg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(reg_alloc->direct_reg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(reg_alloc->direct_reg, &phreg));
                            break;

                        case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                        case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register64(reg_alloc->direct_reg, &phreg));
                            break;
                    }

                    *value = KEFIR_ASMCMP_MAKE_PHREG(phreg);
                    break;

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                    *value = KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc->spill_area_index, 0, value->vreg.variant);
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;

                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(state->register_allocator,
                                                                          value->indirect.base.vreg, &reg_alloc));

                    switch (reg_alloc->type) {
                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                   "Unallocated amd64 virtual register allocation");

                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(reg_alloc->direct_reg, value->indirect.offset,
                                                                         value->indirect.variant);
                            break;

                        case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                            REQUIRE_OK(obtain_temporary_register(mem, state, position, &phreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                mem, state->target, kefir_asmcmp_context_instr_prev(&state->target->context, position),
                                &KEFIR_ASMCMP_MAKE_PHREG(phreg),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(reg_alloc->spill_area_index, 0,
                                                                  KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                &new_position));
                            REQUIRE_OK(
                                kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, position));
                            *value = KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(phreg, value->indirect.offset,
                                                                         value->indirect.variant);
                            break;
                    }
                    break;
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t devirtualize_arg2(struct kefir_mem *mem, struct devirtualize_state *state,
                                        kefir_asmcmp_instruction_index_t position,
                                        struct kefir_asmcmp_instruction *instr) {
    UNUSED(mem);
    UNUSED(state);

    if (instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
        instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT) {
        kefir_asm_amd64_xasmgen_register_t phreg;
        kefir_asmcmp_instruction_index_t new_position;
        struct kefir_asmcmp_value original = instr->args[0];
        REQUIRE_OK(obtain_temporary_register(mem, state, position, &phreg));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, state->target,
                                          kefir_asmcmp_context_instr_prev(&state->target->context, position),
                                          &KEFIR_ASMCMP_MAKE_PHREG(phreg), &original, &new_position));
        REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &state->target->context, new_position, position));
        instr->args[0] = KEFIR_ASMCMP_MAKE_PHREG(phreg);
        REQUIRE_OK(
            kefir_asmcmp_amd64_mov(mem, state->target, position, &original, &KEFIR_ASMCMP_MAKE_PHREG(phreg), NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t devirtualize_arg2w(struct kefir_mem *mem, struct devirtualize_state *state,
                                         kefir_asmcmp_instruction_index_t position,
                                         struct kefir_asmcmp_instruction *instr) {
    UNUSED(mem);
    UNUSED(state);

    if (instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
        instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT) {
        kefir_asm_amd64_xasmgen_register_t phreg;
        struct kefir_asmcmp_value original = instr->args[0];
        REQUIRE_OK(obtain_temporary_register(mem, state, position, &phreg));
        instr->args[0] = KEFIR_ASMCMP_MAKE_PHREG(phreg);
        REQUIRE_OK(
            kefir_asmcmp_amd64_mov(mem, state->target, position, &original, &KEFIR_ASMCMP_MAKE_PHREG(phreg), NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t devirtualize_impl(struct kefir_mem *mem, struct devirtualize_state *state) {
    UNUSED(mem);
    UNUSED(state);

    kefir_size_t num_of_spill_slots;
    REQUIRE_OK(kefir_bitset_length(&state->register_allocator->internal.spill_area, &num_of_spill_slots));
    REQUIRE_OK(kefir_bitset_resize(mem, &state->alive.spill_area, num_of_spill_slots));

    for (kefir_asmcmp_instruction_index_t idx = kefir_asmcmp_context_instr_head(&state->target->context);
         idx != KEFIR_ASMCMP_INDEX_NONE; idx = kefir_asmcmp_context_instr_next(&state->target->context, idx)) {

        const struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(&state->target->context, idx, &instr));

        kefir_size_t instr_linear_position;
        kefir_result_t res = kefir_codegen_amd64_register_allocator_linear_position_of(state->register_allocator, idx,
                                                                                       &instr_linear_position);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        } else {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(remove_dead_virtual_regs(mem, state, instr_linear_position));
        REQUIRE_OK(update_live_virtual_regs(mem, state, instr_linear_position, &instr->args[0]));
        REQUIRE_OK(update_live_virtual_regs(mem, state, instr_linear_position, &instr->args[1]));
        REQUIRE_OK(update_live_virtual_regs(mem, state, instr_linear_position, &instr->args[2]));

        struct kefir_asmcmp_instruction devirtualized_instr = *instr;
        REQUIRE_OK(devirtualize_value(mem, state, idx, &devirtualized_instr.args[0]));
        REQUIRE_OK(devirtualize_value(mem, state, idx, &devirtualized_instr.args[1]));
        REQUIRE_OK(devirtualize_value(mem, state, idx, &devirtualized_instr.args[2]));

        switch (devirtualized_instr.opcode) {
#define CASE_IMPL_virtual(_opcode)
#define CASE_IMPL_arg0(_opcode)              \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        /* Intentionally left blank */       \
        break;
#define CASE_IMPL_arg1(_opcode)              \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        /* Intentionally left blank */       \
        break;
#define CASE_IMPL_arg2(_opcode)                                               \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                  \
        REQUIRE_OK(devirtualize_arg2(mem, state, idx, &devirtualized_instr)); \
        break;
#define CASE_IMPL_arg2w(_opcode)                                               \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                   \
        REQUIRE_OK(devirtualize_arg2w(mem, state, idx, &devirtualized_instr)); \
        break;
#define CASE_IMPL(_opcode, _gen, _type) CASE_IMPL_##_type(_opcode)
            KEFIR_ASMCMP_AMD64_OPCODES(CASE_IMPL, )
#undef CASE_IMPL
#undef CASE_IMPL_virtual
#undef CASE_IMPL_arg0
#undef CASE_IMPL_arg2
#undef CASE_IMPL_arg2w

            case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link):
                REQUIRE_OK(devirtualize_arg2w(mem, state, idx, &devirtualized_instr));
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
            case KEFIR_ASMCMP_AMD64_OPCODE(function_prologue):
            case KEFIR_ASMCMP_AMD64_OPCODE(function_epilogue):
                // Intentionally left blank
                break;
        }

        REQUIRE_OK(kefir_asmcmp_context_instr_replace(&state->target->context, idx, &devirtualized_instr));

        struct kefir_hashtree_node_iterator iter;
        for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&state->evicted.physical_regs, &iter);
             node != NULL; node = kefir_hashtree_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, node->key);
            ASSIGN_DECL_CAST(kefir_size_t, spill_index, node->value);
            if (spill_index != (kefir_size_t) -1) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, state->target, idx, &KEFIR_ASMCMP_MAKE_PHREG(reg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_SPILL(spill_index, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
                REQUIRE_OK(kefir_bitset_set(&state->alive.spill_area, spill_index, false));
            }
        }
        REQUIRE_OK(kefir_hashtree_clean(mem, &state->evicted.physical_regs));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_devirtualize(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                struct kefir_codegen_amd64_register_allocator *register_allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp target"));
    REQUIRE(register_allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));

    struct devirtualize_state state = {.target = target, .register_allocator = register_allocator};
    REQUIRE_OK(kefir_hashtreeset_init(&state.alive.physical_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.alive.virtual_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.current_instr.physical_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&state.evicted.physical_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_bitset_init(&state.alive.spill_area));

    kefir_result_t res = devirtualize_impl(mem, &state);
    kefir_bitset_free(mem, &state.alive.spill_area);
    kefir_hashtree_free(mem, &state.evicted.physical_regs);
    kefir_hashtreeset_free(mem, &state.alive.physical_regs);
    kefir_hashtreeset_free(mem, &state.alive.virtual_regs);
    kefir_hashtreeset_free(mem, &state.current_instr.physical_regs);
    return res;
}
