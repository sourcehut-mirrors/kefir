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

#ifndef KEFIR_PREPROCESSOR_USER_MACRO_H_
#define KEFIR_PREPROCESSOR_USER_MACRO_H_

#include "kefir/preprocessor/macro.h"
#include "kefir/core/hashtreeset.h"

typedef struct kefir_preprocessor_user_macro {
    struct kefir_preprocessor_macro macro;
    struct kefir_list parameters;
    kefir_bool_t vararg;
    const char *vararg_parameter;
    struct kefir_token_buffer replacement;
} kefir_preprocessor_user_macro_t;

struct kefir_preprocessor_user_macro *kefir_preprocessor_user_macro_new_object(struct kefir_mem *,
                                                                               struct kefir_string_pool *,
                                                                               const char *);
struct kefir_preprocessor_user_macro *kefir_preprocessor_user_macro_new_function(struct kefir_mem *,
                                                                                 struct kefir_string_pool *,
                                                                                 const char *);
kefir_result_t kefir_preprocessor_user_macro_free(struct kefir_mem *, struct kefir_preprocessor_user_macro *);

typedef struct kefir_preprocessor_user_macro_scope {
    struct kefir_preprocessor_macro_scope scope;
    const struct kefir_preprocessor_user_macro_scope *parent;
    struct kefir_hashtree macro_index;
    struct kefir_hashtreeset macros;
} kefir_preprocessor_user_macro_scope_t;

kefir_result_t kefir_preprocessor_user_macro_scope_init(const struct kefir_preprocessor_user_macro_scope *,
                                                        struct kefir_preprocessor_user_macro_scope *);
kefir_result_t kefir_preprocessor_user_macro_scope_free(struct kefir_mem *,
                                                        struct kefir_preprocessor_user_macro_scope *);
kefir_result_t kefir_preprocessor_user_macro_scope_insert(struct kefir_mem *,
                                                          struct kefir_preprocessor_user_macro_scope *,
                                                          struct kefir_preprocessor_user_macro *);
kefir_result_t kefir_preprocessor_user_macro_scope_at(const struct kefir_preprocessor_user_macro_scope *, const char *,
                                                      const struct kefir_preprocessor_user_macro **);
kefir_bool_t kefir_preprocessor_user_macro_scope_has(const struct kefir_preprocessor_user_macro_scope *, const char *);
kefir_result_t kefir_preprocessor_user_macro_scope_remove(struct kefir_mem *,
                                                          struct kefir_preprocessor_user_macro_scope *, const char *);

#endif
