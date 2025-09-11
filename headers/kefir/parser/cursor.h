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

#ifndef KEFIR_PARSER_CURSOR_H_
#define KEFIR_PARSER_CURSOR_H_

#include "kefir/lexer/lexem.h"

typedef struct kefir_parser_token_cursor {
    const struct kefir_token_cursor_handle *handle;
    kefir_size_t index;
    struct kefir_token sentinel;

    struct {
        struct kefir_token_cursor_handle handle;
    } direct_cursor;
} kefir_parser_token_cursor_t;

kefir_result_t kefir_parser_token_cursor_init(struct kefir_parser_token_cursor *,
                                              const struct kefir_token_cursor_handle *);
kefir_result_t kefir_parser_token_cursor_init_direct(struct kefir_parser_token_cursor *, struct kefir_token *,
                                                     kefir_size_t);
const struct kefir_token *kefir_parser_token_cursor_at(const struct kefir_parser_token_cursor *, kefir_size_t,
                                                       kefir_bool_t);
kefir_result_t kefir_parser_token_cursor_reset(struct kefir_parser_token_cursor *);
kefir_result_t kefir_parser_token_cursor_next(struct kefir_parser_token_cursor *);
kefir_result_t kefir_parser_token_cursor_save(struct kefir_parser_token_cursor *, kefir_size_t *);
kefir_result_t kefir_parser_token_cursor_restore(struct kefir_parser_token_cursor *, kefir_size_t);

#endif
