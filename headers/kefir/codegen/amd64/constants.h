/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#ifndef KEFIR_CODEGEN_AMD64_CONSTANTS_H_
#define KEFIR_CODEGEN_AMD64_CONSTANTS_H_

#include "kefir/optimizer/module.h"
#include "kefir/optimizer/module_liveness.h"
#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/module.h"
#include "kefir/codegen/amd64/translator.h"

kefir_result_t kefir_codegen_amd64_generate_local_constants(const struct kefir_codegen_amd64_function_translator_state *, const struct kefir_opt_module *, const struct kefir_opt_function *, struct kefir_codegen_amd64 *);
kefir_result_t kefir_codegen_amd64_generate_global_constants(const struct kefir_codegen_amd64_module *, kefir_bool_t *);
kefir_result_t kefir_codegen_amd64_generate_strings(struct kefir_codegen_amd64 *, struct kefir_opt_module *,
                                        struct kefir_opt_module_liveness *, kefir_bool_t *);

#endif
