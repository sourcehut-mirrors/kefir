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

#ifndef KEFIR_CODEGEN_AMD64_SYSTEM_V_ABI_REGISTERS_H_
#define KEFIR_CODEGEN_AMD64_SYSTEM_V_ABI_REGISTERS_H_

#include <stdbool.h>
#include "kefir/core/basic-types.h"
#include "kefir/core/vector.h"
#include "kefir/core/mem.h"
#include "kefir/codegen/amd64/system-v/abi/data.h"
#include "kefir/target/abi/system-v-amd64/data_layout.h"
#include "kefir/target/abi/system-v-amd64/qwords.h"
#include "kefir/target/abi/system-v-amd64/parameters.h"

extern kefir_amd64_xasmgen_register_t KEFIR_AMD64_SYSV_INTEGER_REGISTERS[];
extern const kefir_size_t KEFIR_AMD64_SYSV_INTEGER_REGISTER_COUNT;
extern kefir_amd64_xasmgen_register_t KEFIR_AMD64_SYSV_SSE_REGISTERS[];
extern const kefir_size_t KEFIR_AMD64_SYSV_SSE_REGISTER_COUNT;

extern kefir_amd64_xasmgen_register_t KEFIR_AMD64_SYSV_INTEGER_RETURN_REGISTERS[];
extern const kefir_size_t KEFIR_AMD64_SYSV_INTEGER_RETURN_REGISTER_COUNT;
extern kefir_amd64_xasmgen_register_t KEFIR_AMD64_SYSV_SSE_RETURN_REGISTERS[];
extern const kefir_size_t KEFIR_AMD64_SYSV_SSE_RETURN_REGISTER_COUNT;

#endif
