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

static kefir_result_t amd64_opcode_mnemonic(kefir_asmcmp_instruction_opcode_t asmcmp_opcode, kefir_codegen_target_ir_opcode_t *opcode_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(opcode_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR opcode"));

    switch (asmcmp_opcode) {
#define CASE(_opcode, _mnemonic, ...)        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        *opcode_ptr = KEFIR_TARGET_IR_AMD64_OPCODE(_opcode);            \
        break;

        KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODES(CASE, )
        KEFIR_AMD64_INSTRUCTION_DATABASE(CASE, CASE, CASE, CASE, )
#undef CASE

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }
    return KEFIR_OK;
}

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

const struct kefir_codegen_target_ir_code_constructor_class KEFIR_TARGET_AMD64_CODE_CONSTRUCTOR_CLASS = {
    .map_opcode = amd64_opcode_mnemonic,
    .is_jump = amd64_is_jump,
    .payload = NULL
};
