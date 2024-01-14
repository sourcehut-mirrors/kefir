/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/target/abi/amd64/system-v/return.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_size_t kefir_abi_amd64_num_of_general_purpose_return_registers(kefir_abi_amd64_variant_t variant) {
    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            return KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS_LENGTH;

        default:
            return 0;
    }
}

kefir_result_t kefir_abi_amd64_general_purpose_return_register(kefir_abi_amd64_variant_t variant, kefir_size_t index,
                                                               kefir_asm_amd64_xasmgen_register_t *reg) {
    REQUIRE(reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(index < kefir_abi_amd64_num_of_general_purpose_return_registers(variant),
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested amd64 abi return register is out of bounds"));
            *reg = KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS[index];
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_size_t kefir_abi_amd64_num_of_sse_return_registers(kefir_abi_amd64_variant_t variant) {
    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            return KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS_LENGTH;

        default:
            return 0;
    }
}

kefir_result_t kefir_abi_amd64_sse_return_register(kefir_abi_amd64_variant_t variant, kefir_size_t index,
                                                   kefir_asm_amd64_xasmgen_register_t *reg) {
    REQUIRE(reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(index < kefir_abi_amd64_num_of_sse_return_registers(variant),
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested amd64 abi return register is out of bounds"));
            *reg = KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS[index];
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}
