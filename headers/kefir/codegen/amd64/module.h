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

#ifndef KEFIR_CODEGEN_AMD64_MODULE_H_
#define KEFIR_CODEGEN_AMD64_MODULE_H_

#include "kefir/codegen/amd64/function.h"

typedef struct kefir_codegen_amd64_module {
    struct kefir_codegen_amd64 *codegen;
    struct kefir_opt_module *module;
    struct kefir_hashtree functions;
} kefir_codegen_amd64_module_t;

kefir_result_t kefir_codegen_amd64_module_init(struct kefir_codegen_amd64_module *, struct kefir_codegen_amd64 *,
                                               struct kefir_opt_module *);
kefir_result_t kefir_codegen_amd64_module_free(struct kefir_mem *, struct kefir_codegen_amd64_module *);

kefir_result_t kefir_codegen_amd64_module_insert_function(struct kefir_mem *, struct kefir_codegen_amd64_module *,
                                                          const struct kefir_opt_function *,
                                                          const struct kefir_opt_code_analysis *,
                                                          struct kefir_codegen_amd64_function **);

kefir_result_t kefir_codegen_amd64_module_function(const struct kefir_codegen_amd64_module *, const char *,
                                                   struct kefir_codegen_amd64_function **);

#endif
