/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#include "kefir/codegen/target-ir/tie.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_tie_operands(const struct kefir_codegen_target_ir_code *code,
    kefir_codegen_target_ir_instruction_ref_t instr_ref, struct kefir_codegen_target_ir_tie_classification *classification) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(classification != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR tie classification"));

    REQUIRE_OK(code->klass->classify_instruction(code, instr_ref, &classification->classification, code->klass->payload));

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, instr_ref, &instr));

    kefir_size_t output_index = 0, parameter_idx = 0;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        classification->operands[i].read_index = KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE;
        classification->operands[i].output.instr_ref = KEFIR_ID_NONE;
        classification->operands[i].output.aspect = 0;

        switch (classification->classification.operands[i].class) {
            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE:
                // Intentionally left blank
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ:
                classification->operands[i].read_index = parameter_idx++;
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE:
                if (instr->operation.opcode == code->klass->upsilon_opcode && i == 0) {
                    classification->operands[i].output = instr->operation.parameters[0].upsilon_ref;
                } else {
                    REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(code, instr_ref, output_index++, &classification->operands[i].output, NULL));
                    if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(classification->operands[i].output.aspect)) {
                        classification->operands[i].read_index = parameter_idx++;
                    }
                }
                break;

            case KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE:
                REQUIRE_OK(kefir_codegen_target_ir_code_instruction_output(code, instr_ref, output_index++, &classification->operands[i].output, NULL));
                classification->operands[i].read_index = parameter_idx++;
                break;
        }
    }

    classification->untied_parameter_index = parameter_idx;
    classification->untied_output_index = output_index;
    return KEFIR_OK;
}
