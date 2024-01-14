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

#ifndef KEFIR_PARSER_SCOPE_H_
#define KEFIR_PARSER_SCOPE_H_

#include "kefir/parser/base.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/string_pool.h"
#include "kefir/core/list.h"

typedef struct kefir_parser_block_scope {
    struct kefir_string_pool *symbols;
    struct kefir_hashtree identifier_declarations;
} kefir_parser_block_scope_t;

kefir_result_t kefir_parser_block_scope_init(struct kefir_parser_block_scope *, struct kefir_string_pool *);
kefir_result_t kefir_parser_block_scope_free(struct kefir_mem *, struct kefir_parser_block_scope *);
kefir_result_t kefir_parser_block_scope_declare_typedef(struct kefir_mem *, struct kefir_parser_block_scope *,
                                                        const char *);
kefir_result_t kefir_parser_block_scope_declare_variable(struct kefir_mem *, struct kefir_parser_block_scope *,
                                                         const char *);
kefir_result_t kefir_parser_block_scope_is_typedef(struct kefir_parser_block_scope *, const char *, kefir_bool_t *);

typedef struct kefir_parser_scope {
    struct kefir_list block_scopes;
    struct kefir_string_pool *symbols;
} kefir_parser_scope_t;

kefir_result_t kefir_parser_scope_init(struct kefir_mem *, struct kefir_parser_scope *, struct kefir_string_pool *);
kefir_result_t kefir_parser_scope_free(struct kefir_mem *, struct kefir_parser_scope *);
kefir_result_t kefir_parser_scope_push_block(struct kefir_mem *, struct kefir_parser_scope *);
kefir_result_t kefir_parser_scope_pop_block(struct kefir_mem *, struct kefir_parser_scope *);
kefir_result_t kefir_parser_scope_declare_typedef(struct kefir_mem *, struct kefir_parser_scope *, const char *);
kefir_result_t kefir_parser_scope_declare_variable(struct kefir_mem *, struct kefir_parser_scope *, const char *);
kefir_result_t kefir_parser_scope_is_typedef(struct kefir_parser_scope *, const char *, kefir_bool_t *);

#endif
