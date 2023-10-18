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

#include "kefir/target/abi/amd64/return.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

const kefir_asm_amd64_xasmgen_register_t KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_RAX, KEFIR_AMD64_XASMGEN_REGISTER_RDX};

const kefir_size_t KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS_LENGTH =
    sizeof(KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS) / sizeof(KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS[0]);

const kefir_asm_amd64_xasmgen_register_t KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_XMM0, KEFIR_AMD64_XASMGEN_REGISTER_XMM1};

const kefir_size_t KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS_LENGTH =
    sizeof(KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS) / sizeof(KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS[0]);
