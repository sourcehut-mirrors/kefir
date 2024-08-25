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

#ifndef KEFIR_AST_TRANSLATOR_ENVIRONMENT_H_
#define KEFIR_AST_TRANSLATOR_ENVIRONMENT_H_

#include "kefir/ir/platform.h"
#include "kefir/ast/type.h"
#include "kefir/ast/type_layout.h"
#include "kefir/ast/target_environment.h"

typedef struct kefir_ast_translator_configuration {
    kefir_bool_t empty_structs;
    kefir_bool_t precise_bitfield_load_store;
} kefir_ast_translator_configuration_t;

typedef struct kefir_ast_translator_environment_type {
    const struct kefir_ast_type *ast_type;
    struct kefir_ir_type type;
    struct kefir_ast_type_layout *layout;
    kefir_ir_target_platform_type_handle_t target_type;
} kefir_ast_translator_environment_type_t;

typedef struct kefir_ast_translator_environment {
    struct kefir_ast_target_environment target_env;
    struct kefir_ir_target_platform *target_platform;
    const struct kefir_ast_translator_configuration *configuration;
} kefir_ast_translator_environment_t;

kefir_result_t kefir_ast_translator_environment_new_type(struct kefir_mem *, const struct kefir_ast_context *,
                                          const struct kefir_ast_translator_environment *,
                                          const struct kefir_ast_type *,
                                          struct kefir_ast_translator_environment_type *,
                                          const struct kefir_source_location *);

kefir_result_t kefir_ast_translator_environment_free_type(struct kefir_mem *, const struct kefir_ast_translator_environment *,
                                           struct kefir_ast_translator_environment_type *);

kefir_result_t kefir_ast_translator_configuration_default(struct kefir_ast_translator_configuration *);

kefir_result_t kefir_ast_translator_environment_init(struct kefir_ast_translator_environment *,
                                                     struct kefir_ir_target_platform *);

#endif
