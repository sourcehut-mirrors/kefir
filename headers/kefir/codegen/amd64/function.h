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

#ifndef KEFIR_CODEGEN_AMD64_FUNCTION_H_
#define KEFIR_CODEGEN_AMD64_FUNCTION_H_

#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/optimizer/module.h"

typedef struct kefir_codegen_amd64_function {
    struct kefir_codegen_amd64 *codegen;
    const struct kefir_opt_module *module;
    const struct kefir_opt_function *function;
    const struct kefir_opt_code_analysis *function_analysis;
    struct kefir_asmcmp_amd64 code;

    struct kefir_hashtree labels;
} kefir_codegen_amd64_function_t;

kefir_result_t kefir_codegen_amd64_function_translate(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                                      const struct kefir_opt_module *,
                                                      const struct kefir_opt_function *,
                                                      const struct kefir_opt_code_analysis *);

#endif
