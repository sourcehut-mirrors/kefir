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

#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t amd64_opcode_mnemonic(kefir_target_ir_amd64_opcode_t opcode, const char **mnemonic_ptr, void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to mnemonic"));

    switch (opcode) {
#define CASE(_opcode, _mnemonic, ...)        \
    case KEFIR_TARGET_IR_AMD64_OPCODE(_opcode): \
        *mnemonic_ptr = #_opcode;            \
        break;

        KEFIR_ASMCMP_AMD64_VIRTUAL_OPCODES(CASE, )
        KEFIR_AMD64_INSTRUCTION_DATABASE(CASE, CASE, CASE, CASE, )
#undef CASE

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t register_mnemonic(kefir_asmcmp_physical_register_index_t reg, const char **mnemonic_ptr,
                                        void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to opcode mnemonic"));

    *mnemonic_ptr = kefir_asm_amd64_xasmgen_register_symbolic_name((kefir_asm_amd64_xasmgen_register_t) reg);
    REQUIRE(*mnemonic_ptr != NULL, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unknown amd64 register"));
    return KEFIR_OK;
}

const struct kefir_codegen_target_ir_code_class KEFIR_TARGET_AMD64_CODE_CLASS = {
    .opcode_mnemonic = amd64_opcode_mnemonic,
    .register_mnemonic = register_mnemonic,
    .payload = NULL
};
