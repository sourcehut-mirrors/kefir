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

#include "kefir/codegen/target-ir/amd64/constructor.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t amd64_is_jump(kefir_asmcmp_instruction_opcode_t asmcmp_opcode, kefir_bool_t *is_jump_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(is_jump_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *is_jump_ptr = false;
    switch (asmcmp_opcode){
#define DEF_OPCODE_NOOP(...)
#define DEF_OPCODE1(_opcode, _mnemonic, _branch, ...) CASE_IS_##_branch(_opcode)
#define CASE_IS_BRANCH(_opcode) \
        case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
            *is_jump_ptr = KEFIR_ASMCMP_AMD64_OPCODE(_opcode) != KEFIR_ASMCMP_AMD64_OPCODE(call); \
            break;
#define CASE_IS_(...)

        KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE_NOOP, DEF_OPCODE1, DEF_OPCODE_NOOP, DEF_OPCODE_NOOP,)
#undef DEF_OPCODE_NOOP
#undef DEF_OPCODE1

        case KEFIR_ASMCMP_AMD64_OPCODE(ret):
        case KEFIR_ASMCMP_AMD64_OPCODE(ud2):
            *is_jump_ptr = true;
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t amd64_classify_instruction(const struct kefir_asmcmp_instruction *instruction,
    struct kefir_codegen_target_ir_asmcmp_instruction_classification *classification, void *payload) {
    UNUSED(payload);
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp instruction"));
    REQUIRE(classification != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp instruction"));

    classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_SPECIAL_NONE;
    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        classification->operands[i].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE;
    }
    classification->modifies_flags = false;

    if (instruction->opcode == KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link)) {
        classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_VIRTUAL_REGISTER_LINK;
        classification->operands[0].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE;
        classification->operands[0].index = 0;
        classification->operands[1].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ;
        classification->operands[1].index = 1;
        return KEFIR_OK;
    }

    if (instruction->opcode == KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register) ||
        instruction->opcode == KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_begin) ||
        instruction->opcode == KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_end) ||
        instruction->opcode == KEFIR_ASMCMP_AMD64_OPCODE(noop)) {
        classification->special = KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_SKIP;
        return KEFIR_OK;
    }

#define CLASSIFY_OP(_op, _index) \
    do { \
        if (((_op) & KEFIR_AMD64_INSTRDB_READ) && ((_op) & KEFIR_AMD64_INSTRDB_WRITE)) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE; \
            classification->operands[(_index)].index = (_index); \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_READ) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ; \
            classification->operands[(_index)].index = (_index); \
        } else if ((_op) & KEFIR_AMD64_INSTRDB_WRITE) { \
            classification->operands[(_index)].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE; \
            classification->operands[(_index)].index = (_index); \
        } \
    } while (0)

    switch (instruction->opcode) {
#define INSTR0(_opcode, _mnemonic, _variant, _flags)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        break;

#define INSTR1(_opcode, _mnemonic, _variant, _flags, _op1)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        break;

#define INSTR2(_opcode, _mnemonic, _variant, _flags, _op1, _op2)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        CLASSIFY_OP(_op2, 1); \
        break;

#define INSTR3(_opcode, _mnemonic, _variant, _flags, _op1, _op2, _op3)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        classification->opcode = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        CLASSIFY_OP(_op1, 0); \
        CLASSIFY_OP(_op2, 1); \
        CLASSIFY_OP(_op2, 2); \
        break;

        KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODES(INSTR0, )
        KEFIR_AMD64_INSTRUCTION_DATABASE(INSTR0, INSTR1, INSTR2, INSTR3, )
#undef INSTR0
#undef INSTR1
#undef INSTR2
#undef INSTR3

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }

    for (kefir_size_t i = 0; i < KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS; i++) {
        if (classification->operands[i].class == KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE &&
            instruction->args[i].type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER &&
            (instruction->args[i].vreg.variant == KEFIR_ASMCMP_OPERAND_VARIANT_8BIT || instruction->args[i].vreg.variant == KEFIR_ASMCMP_OPERAND_VARIANT_16BIT)) {
            classification->operands[i].class = KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE;
        }
    }

    return KEFIR_OK;
}

const struct kefir_codegen_target_ir_code_constructor_class KEFIR_TARGET_AMD64_CODE_CONSTRUCTOR_CLASS = {
    .is_jump = amd64_is_jump,
    .classify_instruction = amd64_classify_instruction,
    .payload = NULL
};
