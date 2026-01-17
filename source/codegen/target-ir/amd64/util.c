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

#include "kefir/codegen/target-ir/amd64/util.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_amd64_match_immediate(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_value_ref_t value_ref, kefir_uint64_t *int_value_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    REQUIRE(value_ref.aspect == KEFIR_CODEGEN_TARGET_IR_VALUE_DIRECT_OUTPUT(0), KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match target IR integral assign instruction"));

    const struct kefir_codegen_target_ir_instruction *instr;
    REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, value_ref.instr_ref, &instr));

    const struct kefir_codegen_target_ir_value_type *output_type;
    kefir_result_t res = kefir_codegen_target_ir_code_value_props(code, value_ref, &output_type);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match target IR integral assign instruction"));
    REQUIRE_OK(res);

    REQUIRE((instr->operation.opcode == code->klass->assign_opcode || instr->operation.opcode == KEFIR_TARGET_IR_AMD64_OPCODE(mov)) &&
        (output_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_DEFAULT ||
            output_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_64BIT ||
            output_type->variant == KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT) &&
        (instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_INTEGER ||
            instr->operation.parameters[0].type == KEFIR_CODEGEN_TARGET_IR_OPERAND_TYPE_UINTEGER), KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match target IR integral assign instruction"));
    
    kefir_int64_t value = instr->operation.parameters[0].immediate.int_immediate;
    switch (instr->operation.parameters[0].immediate.variant) {
        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_8BIT:
            value = (kefir_int8_t) value;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_16BIT:
            value = (kefir_int16_t) value;
            break;

        case KEFIR_CODEGEN_TARGET_IR_OPERAND_VARIANT_32BIT:
            value = (kefir_int32_t) value;
            break;

        default:
            // Intentionally left blank
            break;
    }

    ASSIGN_PTR(int_value_ptr, value);
    return KEFIR_OK;
}
