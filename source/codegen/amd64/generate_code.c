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
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/target/asm/amd64/xasmgen.h"

struct instruction_argument_state {
    struct kefir_asm_amd64_xasmgen_operand base_operands[3];
    const struct kefir_asm_amd64_xasmgen_operand *operand;
};

static kefir_result_t build_operand(const struct kefir_codegen_amd64_stack_frame *stack_frame,
                                    const struct kefir_asmcmp_value *value,
                                    struct instruction_argument_state *arg_state) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
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

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register");

        case KEFIR_ASMCMP_VALUE_TYPE_LABEL:
            arg_state->operand = kefir_asm_amd64_xasmgen_operand_label(&arg_state->base_operands[0], value->label);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT: {
            const struct kefir_asm_amd64_xasmgen_operand *base_ptr = NULL;

            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register");

                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0], kefir_asm_amd64_xasmgen_operand_reg(value->indirect.base.phreg),
                        value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_LABEL_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(&arg_state->base_operands[1], value->indirect.base.label),
                        value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        stack_frame->offsets.local_area + value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        stack_frame->offsets.spill_area + value->indirect.base.spill_index * KEFIR_AMD64_ABI_QWORD +
                            value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        stack_frame->offsets.temporary_area + value->indirect.offset);
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

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT:
            switch (value->rip_indirection.variant) {
                case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_BYTE,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1],
                                                                        value->rip_indirection.base));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_WORD,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1],
                                                                        value->rip_indirection.base));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1],
                                                                        value->rip_indirection.base));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1],
                                                                        value->rip_indirection.base));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[0],
                                                                                         value->rip_indirection.base);
                    break;
            }
            break;
    }

    if (value->segment.present) {
        arg_state->operand = kefir_asm_amd64_xasmgen_operand_segment(
            &arg_state->base_operands[2], (kefir_asm_amd64_xasmgen_segment_register_t) value->segment.reg,
            arg_state->operand);
    }

    return KEFIR_OK;
}

static kefir_result_t generate_instr(struct kefir_amd64_xasmgen *xasmgen, const struct kefir_asmcmp_amd64 *target,
                                     const struct kefir_codegen_amd64_stack_frame *stack_frame,
                                     kefir_asmcmp_instruction_index_t index) {
    const struct kefir_asmcmp_instruction *instr;
    REQUIRE_OK(kefir_asmcmp_context_instr_at(&target->context, index, &instr));
    for (kefir_asmcmp_label_index_t label = kefir_asmcmp_context_instr_label_head(&target->context, index);
         label != KEFIR_ASMCMP_INDEX_NONE; label = kefir_asmcmp_context_instr_label_next(&target->context, label)) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, KEFIR_AMD64_LABEL, target->function_name, label));
    }

    struct instruction_argument_state arg_state[3] = {0};
    switch (instr->opcode) {
#define DEF_OPCODE_virtual(_opcode, _xasmgen)
#define DEF_OPCODE_0(_opcode, _xasmgen)                            \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                       \
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen)); \
        break;
#define DEF_OPCODE_repeat(_opcode, _xasmgen)                             \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                             \
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen, true)); \
        break;
#define DEF_OPCODE_1(_opcode, _xasmgen)                                                  \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                             \
        REQUIRE_OK(build_operand(stack_frame, &instr->args[0], &arg_state[0]));          \
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen, arg_state[0].operand)); \
        break;
#define DEF_OPCODE_2(_opcode, _xasmgen)                                                                        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                   \
        REQUIRE_OK(build_operand(stack_frame, &instr->args[0], &arg_state[0]));                                \
        REQUIRE_OK(build_operand(stack_frame, &instr->args[1], &arg_state[1]));                                \
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen, arg_state[0].operand, arg_state[1].operand)); \
        break;
#define DEF_OPCODE_helper2(_argc, _opcode, _xasmgen) DEF_OPCODE_##_argc(_opcode, _xasmgen)
#define DEF_OPCODE_helper(_argc, _opcode, _xasmgen) DEF_OPCODE_helper2(_argc, _opcode, _xasmgen)
#define DEF_OPCODE(_opcode, _xasmgen, _argclass) \
    DEF_OPCODE_helper(KEFIR_ASMCMP_AMD64_ARGUMENT_COUNT_FOR(_argclass), _opcode, _xasmgen)

        KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, ;);
#undef DEF_OPCODE
#undef DEF_OPCODE_helper
#undef DEF_OPCODE_helper2
#undef DEF_OPCODE_virtual
#undef DEF_OPCODE_0
#undef DEF_OPCODE_repeat
#undef DEF_OPCODE_1
#undef DEF_OPCODE_2

        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link):
            if (!(instr->args[0].type == KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER &&
                  instr->args[1].type == KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER &&
                  instr->args[0].phreg == instr->args[1].phreg)) {
                REQUIRE_OK(build_operand(stack_frame, &instr->args[0], &arg_state[0]));
                REQUIRE_OK(build_operand(stack_frame, &instr->args[1], &arg_state[1]));
                if ((arg_state[0].operand->klass == KEFIR_AMD64_XASMGEN_OPERAND_REGISTER &&
                     kefir_asm_amd64_xasmgen_register_is_floating_point(arg_state[0].operand->reg)) ||
                    (arg_state[1].operand->klass == KEFIR_AMD64_XASMGEN_OPERAND_REGISTER &&
                     kefir_asm_amd64_xasmgen_register_is_floating_point(arg_state[1].operand->reg))) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(xasmgen, arg_state[0].operand, arg_state[1].operand));
                } else {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(xasmgen, arg_state[0].operand, arg_state[1].operand));
                }
            }
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(function_prologue):
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_prologue(xasmgen, target->abi_variant, stack_frame));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(function_epilogue):
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_epilogue(xasmgen, target->abi_variant, stack_frame));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(data_word):
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&arg_state[0].base_operands[0], instr->args[0].uint_immediate)));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
        case KEFIR_ASMCMP_AMD64_OPCODE(noop):
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(stash_activate):
        case KEFIR_ASMCMP_AMD64_OPCODE(stash_deactivate):
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected stash (de-)activation instruction");
    }

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_generate_code(struct kefir_amd64_xasmgen *xasmgen,
                                                const struct kefir_asmcmp_amd64 *target,
                                                const struct kefir_codegen_amd64_stack_frame *stack_frame) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 assembly generator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp amd64 target"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    for (kefir_asmcmp_instruction_index_t idx = kefir_asmcmp_context_instr_head(&target->context);
         idx != KEFIR_ASMCMP_INDEX_NONE; idx = kefir_asmcmp_context_instr_next(&target->context, idx)) {
        REQUIRE_OK(generate_instr(xasmgen, target, stack_frame, idx));
    }

    return KEFIR_OK;
}
