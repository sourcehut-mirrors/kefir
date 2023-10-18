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

#ifndef KEFIR_CODEGEN_SYSTEM_V_AMD64_ABI_H_
#define KEFIR_CODEGEN_SYSTEM_V_AMD64_ABI_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/vector.h"
#include "kefir/core/mem.h"
#include "kefir/codegen/system-v-amd64.h"
#include "kefir/codegen/system-v-amd64/registers.h"
#include "kefir/codegen/system-v-amd64/function.h"
#include "kefir/codegen/system-v-amd64/static_data.h"

typedef enum kefir_amd64_sysv_internal_fields {
    KEFIR_AMD64_SYSV_INTERNAL_RETURN_ADDRESS = 0,
    // Auxilary
    KEFIR_AMD64_SYSV_INTERNAL_COUNT = 2
} kefir_amd64_sysv_internal_fields_t;

_Static_assert(KEFIR_AMD64_SYSV_INTERNAL_COUNT % 2 == 0, "KEFIR_AMD64_SYSV_INTERNAL_COUNT must be divisible by 2");

#define KEFIR_AMD64_SYSV_INTERNAL_BOUND (KEFIR_AMD64_SYSV_INTERNAL_COUNT * KEFIR_AMD64_ABI_QWORD)

#define KEFIR_AMD64_SYSV_ABI_PROGRAM_REG KEFIR_AMD64_XASMGEN_REGISTER_RBX
#define KEFIR_AMD64_SYSV_ABI_TMP_REG KEFIR_AMD64_XASMGEN_REGISTER_R11
#define KEFIR_AMD64_SYSV_ABI_DATA_REG KEFIR_AMD64_XASMGEN_REGISTER_R12
#define KEFIR_AMD64_SYSV_ABI_DATA2_REG KEFIR_AMD64_XASMGEN_REGISTER_R13
#define KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG KEFIR_AMD64_XASMGEN_REGISTER_R14

#define KEFIR_AMD64_SYSV_ABI_ERROR_PREFIX "AMD64 System-V ABI: "

#endif
