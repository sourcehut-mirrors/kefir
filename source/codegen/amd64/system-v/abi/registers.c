/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "kefir/codegen/amd64/system-v/abi/registers.h"

kefir_asm_amd64_xasmgen_register_t KEFIR_AMD64_SYSV_INTEGER_REGISTERS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_RDI, KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDX,
    KEFIR_AMD64_XASMGEN_REGISTER_RCX, KEFIR_AMD64_XASMGEN_REGISTER_R8,  KEFIR_AMD64_XASMGEN_REGISTER_R9};

const kefir_size_t KEFIR_AMD64_SYSV_INTEGER_REGISTER_COUNT =
    sizeof(KEFIR_AMD64_SYSV_INTEGER_REGISTERS) / sizeof(KEFIR_AMD64_SYSV_INTEGER_REGISTERS[0]);

kefir_asm_amd64_xasmgen_register_t KEFIR_AMD64_SYSV_SSE_REGISTERS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_XMM0, KEFIR_AMD64_XASMGEN_REGISTER_XMM1, KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM3, KEFIR_AMD64_XASMGEN_REGISTER_XMM4, KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM6, KEFIR_AMD64_XASMGEN_REGISTER_XMM7};

const kefir_size_t KEFIR_AMD64_SYSV_SSE_REGISTER_COUNT =
    sizeof(KEFIR_AMD64_SYSV_SSE_REGISTERS) / sizeof(KEFIR_AMD64_SYSV_SSE_REGISTERS[0]);
kefir_asm_amd64_xasmgen_register_t KEFIR_AMD64_SYSV_INTEGER_RETURN_REGISTERS[] = {KEFIR_AMD64_XASMGEN_REGISTER_RAX,
                                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RDX};

const kefir_size_t KEFIR_AMD64_SYSV_INTEGER_RETURN_REGISTER_COUNT =
    sizeof(KEFIR_AMD64_SYSV_INTEGER_RETURN_REGISTERS) / sizeof(KEFIR_AMD64_SYSV_INTEGER_RETURN_REGISTERS[0]);

kefir_asm_amd64_xasmgen_register_t KEFIR_AMD64_SYSV_SSE_RETURN_REGISTERS[] = {KEFIR_AMD64_XASMGEN_REGISTER_XMM0,
                                                                              KEFIR_AMD64_XASMGEN_REGISTER_XMM1};

const kefir_size_t KEFIR_AMD64_SYSV_SSE_RETURN_REGISTER_COUNT =
    sizeof(KEFIR_AMD64_SYSV_SSE_RETURN_REGISTERS) / sizeof(KEFIR_AMD64_SYSV_SSE_RETURN_REGISTERS[0]);
