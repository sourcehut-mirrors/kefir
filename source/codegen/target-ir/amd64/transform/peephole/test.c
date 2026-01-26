/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#define KEFIR_CODEGEN_TARGET_IR_AMD64_PEEPHOLE_INTERNAL
#include "kefir/codegen/target-ir/amd64/transform.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_peephole_test(struct kefir_mem *mem,
                                                           struct kefir_codegen_target_ir_code *code,
                                                           const struct kefir_codegen_target_ir_instruction *instr,
                                                           kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;
    REQUIRE(classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
                classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
                classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                instr->operation.parameters[classification.operands[0].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                instr->operation.parameters[classification.operands[1].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
            KEFIR_OK);

    if (kefir_codegen_target_ir_code_control_prev(code, instr_ref) != KEFIR_ID_NONE &&
        kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_control_prev(code, instr_ref)) !=
            KEFIR_ID_NONE) {
        const struct kefir_codegen_target_ir_instruction *prev_instr, *prev2_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(
            code, kefir_codegen_target_ir_code_control_prev(code, instr_ref), &prev_instr));
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(
            code,
            kefir_codegen_target_ir_code_control_prev(code, kefir_codegen_target_ir_code_control_prev(code, instr_ref)),
            &prev2_instr));

        if (prev2_instr->operation.opcode == instr->operation.opcode) {
            struct kefir_codegen_target_ir_tie_classification prev2_classification;
            REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, prev2_instr->instr_ref, &prev2_classification));
            if (prev2_classification.classification.operands[0].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
                prev2_classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&
                prev2_classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                prev2_classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                prev2_instr->operation.parameters[prev2_classification.operands[0].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                prev2_instr->operation.parameters[prev2_classification.operands[1].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(
                    &instr->operation.parameters[classification.operands[0].read_index].direct.value_ref) ==
                    KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(
                        &prev2_instr->operation.parameters[prev2_classification.operands[0].read_index]
                             .direct.value_ref) &&
                KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(
                    &instr->operation.parameters[classification.operands[1].read_index].direct.value_ref) ==
                    KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(
                        &prev2_instr->operation.parameters[prev2_classification.operands[1].read_index]
                             .direct.value_ref) &&
                instr->operation.parameters[classification.operands[0].read_index].direct.variant ==
                    prev2_instr->operation.parameters[prev2_classification.operands[0].read_index].direct.variant &&
                instr->operation.parameters[classification.operands[1].read_index].direct.variant ==
                    prev2_instr->operation.parameters[prev2_classification.operands[1].read_index].direct.variant) {
                switch (prev_instr->operation.opcode) {
                    case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnp):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setp):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setns):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(sets):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setb):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnb):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setae):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setg):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setge):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setl):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setle):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(seta):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setbe):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(seto):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setno):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setc):
                    case KEFIR_TARGET_IR_AMD64_OPCODE(setnc):
                        REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, prev2_instr->instr_ref,
                                                                                    instr_ref));
                        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
                        *replaced = true;
                        return KEFIR_OK;

                    default:
                        // Intentionally left blank
                        break;
                }
            }
        }
    }

    REQUIRE(instr->operation.parameters[classification.operands[0].read_index].direct.variant ==
                    instr->operation.parameters[classification.operands[1].read_index].direct.variant &&
                instr->operation.parameters[classification.operands[0].read_index].direct.variant !=
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT &&
                instr->operation.parameters[classification.operands[0].read_index].direct.value_ref.instr_ref ==
                    instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.instr_ref &&
                instr->operation.parameters[classification.operands[0].read_index].direct.value_ref.aspect ==
                    instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.aspect,
            KEFIR_OK);

    const struct kefir_codegen_target_ir_instruction *producer_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(
        code, instr->operation.parameters[classification.operands[0].read_index].direct.value_ref.instr_ref,
        &producer_instr));

    switch (producer_instr->operation.opcode) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(movsx):
        case KEFIR_TARGET_IR_AMD64_OPCODE(movzx): {
            struct kefir_codegen_target_ir_tie_classification producer_classification;
            REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, producer_instr->instr_ref, &producer_classification));
            REQUIRE(producer_classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                        producer_instr->operation.parameters[producer_classification.operands[1].read_index].type ==
                            KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                        producer_instr->operation.parameters[producer_classification.operands[1].read_index]
                                .direct.variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT,
                    KEFIR_OK);

            const struct kefir_codegen_target_ir_instruction *base_producer_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(
                code,
                producer_instr->operation.parameters[producer_classification.operands[1].read_index]
                    .direct.value_ref.instr_ref,
                &base_producer_instr));
            switch (base_producer_instr->operation.opcode) {
                case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setnp):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setp):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setns):
                case KEFIR_TARGET_IR_AMD64_OPCODE(sets):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setb):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setnb):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setae):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setg):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setge):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setl):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setle):
                case KEFIR_TARGET_IR_AMD64_OPCODE(seta):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setbe):
                case KEFIR_TARGET_IR_AMD64_OPCODE(seto):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setno):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setc):
                case KEFIR_TARGET_IR_AMD64_OPCODE(setnc): {
                    struct kefir_codegen_target_ir_operation oper = instr->operation;
                    oper.parameters[classification.operands[0].read_index].direct =
                        producer_instr->operation.parameters[producer_classification.operands[1].read_index].direct;
                    oper.parameters[classification.operands[1].read_index].direct =
                        producer_instr->operation.parameters[producer_classification.operands[1].read_index].direct;

                    REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, NULL));
                    *replaced = true;
                } break;

                default:
                    // Intentionally left blank
                    break;
            }
        } break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(sete):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setne):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setnp):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setp):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setns):
        case KEFIR_TARGET_IR_AMD64_OPCODE(sets):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setnb):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setae):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setg):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setge):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setl):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setle):
        case KEFIR_TARGET_IR_AMD64_OPCODE(seta):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setbe):
        case KEFIR_TARGET_IR_AMD64_OPCODE(seto):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setno):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setc):
        case KEFIR_TARGET_IR_AMD64_OPCODE(setnc): {
            struct kefir_codegen_target_ir_tie_classification producer_classification;
            REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, producer_instr->instr_ref, &producer_classification));
            REQUIRE(
                producer_classification.classification.operands[0].class ==
                        KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE &&
                    producer_classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                    producer_instr->operation.parameters[producer_classification.operands[0].read_index].type ==
                        KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF,
                KEFIR_OK);

            const struct kefir_codegen_target_ir_instruction *base_producer_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(
                code,
                producer_instr->operation.parameters[producer_classification.operands[0].read_index]
                    .direct.value_ref.instr_ref,
                &base_producer_instr));

            REQUIRE(base_producer_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(mov) ||
                        base_producer_instr->operation.opcode == code->klass->assign_opcode,
                    KEFIR_OK);
            const struct kefir_codegen_target_ir_value_type *base_producer_value_type;
            kefir_result_t res = kefir_codegen_target_ir_code_instruction_output(code, base_producer_instr->instr_ref,
                                                                                 0, NULL, &base_producer_value_type);
            REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
            REQUIRE_OK(res);
            REQUIRE(base_producer_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT ||
                        base_producer_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
                        base_producer_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT,
                    KEFIR_OK);
            REQUIRE(base_producer_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER,
                    KEFIR_OK);
            REQUIRE(base_producer_instr->operation.parameters[0].immediate.int_immediate == 0, KEFIR_OK);

            struct kefir_codegen_target_ir_operation oper = instr->operation;
            oper.parameters[classification.operands[0].read_index].direct.variant =
                KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT;
            oper.parameters[classification.operands[1].read_index].direct.variant =
                KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT;

            REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, NULL));
            *replaced = true;
        } break;

        default:
            // Intentionally left blank
            break;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_amd64_peephole_cmp(struct kefir_mem *mem,
                                                          struct kefir_codegen_target_ir_code *code,
                                                          const struct kefir_codegen_target_ir_instruction *instr,
                                                          kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    REQUIRE_OK(kefir_codegen_target_ir_amd64_peephole_const_operand(mem, code, instr, true, replaced));
    return KEFIR_OK;
}
