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

#ifndef KEFIR_CODEGEN_AMD64_SYSTEM_V_PLATFORM_H_
#define KEFIR_CODEGEN_AMD64_SYSTEM_V_PLATFORM_H_

#include "kefir/ir/platform.h"
#include "kefir/codegen/amd64/system-v/abi/module.h"
#include "kefir/target/abi/system-v-amd64/data_layout.h"

typedef struct kefir_codegen_amd64_sysv_type {
    const struct kefir_ir_type *ir_type;
    struct kefir_abi_sysv_amd64_type_layout layout;
} kefir_codegen_amd64_sysv_type_t;

kefir_result_t kefir_codegen_amd64_sysv_target_platform(struct kefir_ir_target_platform *);

#endif
