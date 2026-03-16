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

#ifndef KEFIR_PARSER_PRAGMA_H_
#define KEFIR_PARSER_PRAGMA_H_

#include "kefir/core/basic-types.h"
#include "kefir/ast/pragma.h"
#include "kefir/lexer/lexem.h"

typedef struct kefir_parser_pragmas_pack_entry {
    kefir_size_t prev;
    kefir_bool_t has_value;
    kefir_int64_t value;
} kefir_parser_pragmas_pack_entry_t;

typedef struct kefir_parser_pragmas {
    struct kefir_ast_pragma_state file_scope;

    kefir_bool_t in_function_scope;

    struct {
        struct kefir_parser_pragmas_pack_entry *stack;
        kefir_size_t stack_length;
        kefir_size_t stack_top;
    } pack;
} kefir_parser_pragmas_t;

kefir_result_t kefir_parser_pragmas_init(struct kefir_parser_pragmas *);
kefir_result_t kefir_parser_pragmas_free(struct kefir_mem *, struct kefir_parser_pragmas *);
kefir_result_t kefir_parser_pragmas_collect(struct kefir_ast_pragma_state *, const struct kefir_parser_pragmas *);

kefir_result_t kefir_parser_scan_pragma(struct kefir_mem *, struct kefir_parser_pragmas *,
                                        struct kefir_ast_pragma_state *, kefir_pragma_token_type_t,
                                        struct kefir_pragma_token_parameter, const struct kefir_source_location *);

#endif
