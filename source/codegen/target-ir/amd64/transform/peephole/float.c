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
#include "kefir/codegen/target-ir/amd64/util.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define FP_PEEPHOLE(_mov, _variant)                                                                                 \
    do {                                                                                                            \
        struct kefir_codegen_target_ir_tie_classification classification;                                           \
        REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));                  \
                                                                                                                    \
        REQUIRE(classification.classification.operands[1].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ &&   \
                    classification.operands[1].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&        \
                    instr->operation.parameters[classification.operands[1].read_index].type ==                      \
                        KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&                                           \
                    instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.aspect ==   \
                        KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0) &&                                           \
                    (instr->operation.parameters[classification.operands[1].read_index].direct.variant ==           \
                     KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT),                                              \
                KEFIR_OK);                                                                                          \
                                                                                                                    \
        kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;                                     \
                                                                                                                    \
        const struct kefir_codegen_target_ir_instruction *arg_instr;                                                \
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(                                                        \
            code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref.instr_ref,    \
            &arg_instr));                                                                                           \
                                                                                                                    \
        REQUIRE(arg_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(_mov), KEFIR_OK);                       \
        const struct kefir_codegen_target_ir_value_type *arg_value_type;                                            \
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(                                                        \
            code, instr->operation.parameters[classification.operands[1].read_index].direct.value_ref,              \
            &arg_value_type));                                                                                      \
        REQUIRE(arg_value_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT &&                       \
                    arg_value_type->kind == KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT,                      \
                KEFIR_OK);                                                                                          \
        REQUIRE(                                                                                                    \
            (arg_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_RIP_INDIRECT_NATIVE && \
             (arg_instr->operation.parameters[0].rip_indirection.variant ==                                         \
                  KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||                                                \
              arg_instr->operation.parameters[0].rip_indirection.variant == (_variant))) ||                         \
                (arg_instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INDIRECT &&        \
                 arg_instr->operation.parameters[0].indirect.type ==                                                \
                     KEFIR_CODEGEN_TARGET_IR_INDIRECT_NATIVE_LABEL_BASIS &&                                         \
                 (arg_instr->operation.parameters[0].indirect.variant ==                                            \
                      KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||                                            \
                  arg_instr->operation.parameters[0].indirect.variant == (_variant))),                              \
            KEFIR_OK);                                                                                              \
                                                                                                                    \
        struct kefir_codegen_target_ir_operation oper = instr->operation;                                           \
        oper.parameters[classification.operands[1].read_index] = arg_instr->operation.parameters[0];                \
                                                                                                                    \
        REQUIRE_OK(kefir_codegen_target_ir_code_replace_operation(mem, code, instr_ref, &oper, NULL));              \
        *replaced = true;                                                                                           \
    } while (0)

kefir_result_t kefir_codegen_target_ir_amd64_peephole_fp_single_arith(
    struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    FP_PEEPHOLE(movd, KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_SINGLE);
    return KEFIR_OK;
}
kefir_result_t kefir_codegen_target_ir_amd64_peephole_fp_double_arith(
    struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code,
    const struct kefir_codegen_target_ir_instruction *instr, kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    FP_PEEPHOLE(movq, KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_FP_DOUBLE);
    return KEFIR_OK;
}
