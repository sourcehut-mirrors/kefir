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
#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/tie.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_peephole_fxch(struct kefir_mem *mem,
                                                           struct kefir_codegen_target_ir_code *code,
                                                           const struct kefir_codegen_target_ir_instruction *instr,
                                                           kefir_bool_t *replaced) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR instruction"));
    REQUIRE(replaced != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_codegen_target_ir_instruction_ref_t instr_ref = instr->instr_ref;

    struct kefir_codegen_target_ir_tie_classification classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, instr->instr_ref, &classification));

    REQUIRE(classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
                instr->operation.parameters[classification.operands[0].read_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87 &&
                instr->operation.parameters[classification.untied_parameter_index].type ==
                    KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
                KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(
                    instr->operation.parameters[classification.untied_parameter_index].direct.value_ref.aspect) &&
                KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(
                    instr->operation.parameters[classification.untied_parameter_index].direct.value_ref.aspect) ==
                    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK,
            KEFIR_OK);

    kefir_codegen_target_ir_instruction_ref_t next_instr_ref =
        kefir_codegen_target_ir_code_control_next(code, instr->instr_ref);
    const struct kefir_codegen_target_ir_instruction *next_instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, next_instr_ref, &next_instr));

    REQUIRE(next_instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(fxch), KEFIR_OK);

    struct kefir_codegen_target_ir_tie_classification next_classification;
    REQUIRE_OK(kefir_codegen_target_ir_tie_operands(code, next_instr_ref, &next_classification));

    REQUIRE(
        next_classification.operands[0].read_index != KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE &&
            next_instr->operation.parameters[next_classification.operands[0].read_index].type ==
                KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_X87 &&
            next_instr->operation.parameters[next_classification.operands[0].read_index].x87 ==
                instr->operation.parameters[classification.operands[0].read_index].x87 &&
            next_instr->operation.parameters[next_classification.untied_parameter_index].type ==
                KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_VALUE_REF &&
            next_instr->operation.parameters[next_classification.untied_parameter_index].direct.value_ref.instr_ref ==
                instr->instr_ref &&
            KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(
                next_instr->operation.parameters[next_classification.untied_parameter_index].direct.value_ref.aspect) &&
            KEFIR_CODEGEN_TARGET_IR_VALUE_GET_OUTPUT_INDEX(
                next_instr->operation.parameters[next_classification.untied_parameter_index].direct.value_ref.aspect) ==
                KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK,
        KEFIR_OK);

    kefir_codegen_target_ir_instruction_ref_t replacement_ref =
        instr->operation.parameters[classification.untied_parameter_index].direct.value_ref.instr_ref;
    REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, replacement_ref, instr->instr_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_replace_instruction(mem, code, replacement_ref, next_instr_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, instr_ref));
    REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, next_instr_ref));
    *replaced = true;
    return KEFIR_OK;
}
