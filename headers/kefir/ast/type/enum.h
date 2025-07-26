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

#ifndef KEFIR_AST_TYPE_ENUM_H_
#define KEFIR_AST_TYPE_ENUM_H_

#include "kefir/core/list.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/string_pool.h"
#include "kefir/ast/base.h"
#include "kefir/ast/type/base.h"
#include "kefir/ast/constant_expression.h"

typedef struct kefir_ast_enum_enumerator {
    const char *identifier;
    kefir_bool_t has_value;
    kefir_ast_constant_expression_int_t value;
} kefir_ast_enum_enumerator_t;

typedef struct kefir_ast_enum_type {
    kefir_bool_t complete;
    const char *identifier;
    const struct kefir_ast_type *underlying_type;
    struct kefir_list enumerators;
    struct kefir_hashtree enumerator_index;

    struct {
        kefir_bool_t no_discard;
        const char *no_discard_message;
    } flags;
} kefir_ast_enum_type_t;

kefir_result_t kefir_ast_enumeration_get(const struct kefir_ast_enum_type *, const char *, kefir_bool_t *,
                                         kefir_ast_constant_expression_int_t *);

const struct kefir_ast_type *kefir_ast_type_incomplete_enumeration(struct kefir_mem *, struct kefir_ast_type_bundle *,
                                                                   const char *, const struct kefir_ast_type *);

kefir_result_t kefir_ast_enumeration_type_constant(struct kefir_mem *, struct kefir_string_pool *,
                                                   struct kefir_ast_enum_type *, const char *,
                                                   kefir_ast_constant_expression_int_t);

kefir_result_t kefir_ast_enumeration_type_constant_auto(struct kefir_mem *, struct kefir_string_pool *,
                                                        struct kefir_ast_enum_type *, const char *);

const struct kefir_ast_type *kefir_ast_enumeration_underlying_type(const struct kefir_ast_enum_type *);

const struct kefir_ast_type *kefir_ast_type_enumeration(struct kefir_mem *, struct kefir_ast_type_bundle *,
                                                        const char *, const struct kefir_ast_type *,
                                                        struct kefir_ast_enum_type **);

#endif
