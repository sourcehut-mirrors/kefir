/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#ifndef KEFIR_TEST_UTIL_H_
#define KEFIR_TEST_UTIL_H_

#include "kefir/ast-translator/environment.h"
#include "kefir/ast/context.h"
#include "kefir/ast/global_context.h"
#include "kefir/ast/local_context.h"

struct kefir_ir_target_platform *kft_util_get_ir_target_platform(void);
struct kefir_ast_translator_environment *kft_util_get_translator_environment(void);
const struct kefir_ast_type_traits *kefir_util_default_type_traits(void);
const struct kefir_data_model_descriptor *kefir_util_default_data_model(void);

typedef struct kefir_ast_context_manager {
    struct kefir_ast_global_context *global;
    struct kefir_ast_local_context *local;
    struct kefir_ast_context *current;
} kefir_ast_context_manager_t;

kefir_result_t kefir_ast_context_manager_init(struct kefir_ast_global_context *, struct kefir_ast_context_manager *);

kefir_result_t kefir_ast_context_manager_attach_local(struct kefir_ast_local_context *,
                                                      struct kefir_ast_context_manager *);

kefir_result_t kefir_ast_context_manager_detach_local(struct kefir_ast_context_manager *);

#endif
