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

#ifndef KEFIR_COMPILER_PROFILE_H_
#define KEFIR_COMPILER_PROFILE_H_

#include <stdio.h>
#include "kefir/core/basic-types.h"
#include "kefir/lexer/context.h"
#include "kefir/ast/type.h"
#include "kefir/codegen/codegen.h"
#include "kefir/ir/platform.h"

typedef struct kefir_compiler_profile {
    struct kefir_lexer_context lexer_context;
    const struct kefir_ast_type_traits type_traits;
    struct kefir_ir_target_platform ir_target_platform;
    kefir_bool_t optimizer_enabled;
    const char *runtime_include_dirname;
    kefir_bool_t runtime_hooks_enabled;

    kefir_result_t (*new_codegen)(struct kefir_mem *, FILE *, const struct kefir_codegen_configuration *,
                                  const struct kefir_codegen_runtime_hooks *, struct kefir_codegen **);
    kefir_result_t (*free_codegen)(struct kefir_mem *, struct kefir_codegen *);
} kefir_compiler_profile_t;

typedef enum kefir_compiler_profile_char_signedness {
    KEFIR_COMPILER_PROFILE_CHAR_SIGNEDNESS_DEFAULT,
    KEFIR_COMPILER_PROFILE_CHAR_SIGNED,
    KEFIR_COMPILER_PROFILE_CHAR_UNSIGNED
} kefir_compiler_profile_char_signedness_t;

typedef struct kefir_compiler_profile_configuration {
    kefir_compiler_profile_char_signedness_t char_signedness;
} kefir_compiler_profile_configuration_t;

kefir_result_t kefir_compiler_profile(struct kefir_mem *, struct kefir_compiler_profile *, const char *,
                                      const struct kefir_compiler_profile_configuration *);

#endif
