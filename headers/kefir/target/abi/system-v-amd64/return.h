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

#ifndef KEFIR_TARGET_ABI_SYSTEM_V_AMD64_RETURN_H_
#define KEFIR_TARGET_ABI_SYSTEM_V_AMD64_RETURN_H_

#include "kefir/core/basic-types.h"
#include "kefir/target/asm/amd64/xasmgen.h"

extern kefir_asm_amd64_xasmgen_register_t KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS[];
extern const kefir_size_t KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTER_COUNT;
extern kefir_asm_amd64_xasmgen_register_t KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS[];
extern const kefir_size_t KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTER_COUNT;

#endif
