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

#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_H_

#include "kefir/core/basic-types.h"
#include "kefir/codegen/codegen.h"
#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/target/abi/system-v-amd64/platform.h"

typedef struct kefir_codegen_opt_amd64 {
    struct kefir_codegen codegen;
    const struct kefir_codegen_configuration *config;
    struct kefir_amd64_xasmgen xasmgen;
    struct kefir_asm_amd64_xasmgen_helpers xasmgen_helpers;
} kefir_codegen_opt_amd64_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_init(struct kefir_mem *, struct kefir_codegen_opt_amd64 *, FILE *,
                                                 const struct kefir_codegen_configuration *);

extern const char KefirOptSysVAmd64RuntimeCode[];

#endif
