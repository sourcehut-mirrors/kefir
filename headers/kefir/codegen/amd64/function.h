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

#ifndef KEFIR_CODEGEN_AMD64_FUNCTION_H_
#define KEFIR_CODEGEN_AMD64_FUNCTION_H_

#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/codegen/amd64/translator.h"
#include "kefir/codegen/amd64/target_ir.h"
#include "kefir/codegen/function.h"
#include "kefir/core/basic-types.h"
#include "kefir/target/abi/amd64/function.h"
#include "kefir/optimizer/module.h"

typedef struct kefir_codegen_amd64_module kefir_codegen_amd64_module_t;

typedef struct kefir_codegen_amd64_function {
    struct kefir_codegen_function generic;
    struct kefir_codegen_amd64 *codegen;
    struct kefir_codegen_amd64_module *codegen_module;
    struct kefir_abi_amd64_function_decl abi_function_declaration;
    struct kefir_asmcmp_amd64 code;
    struct kefir_codegen_amd64_stack_frame stack_frame;

    struct kefir_codegen_amd64_function_translator_state translator;
    struct kefir_codegen_amd64_function_target_ir target_ir;
} kefir_codegen_amd64_function_t;

kefir_result_t kefir_codegen_amd64_function_init(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                 struct kefir_codegen_amd64_module *, const struct kefir_opt_module *,
                                                 struct kefir_opt_function *);
kefir_result_t kefir_codegen_amd64_function_free(struct kefir_mem *, struct kefir_codegen_amd64_function *);
kefir_result_t kefir_codegen_amd64_function_translate(struct kefir_mem *, struct kefir_codegen_amd64_function *);

#endif
