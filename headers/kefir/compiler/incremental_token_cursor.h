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

#ifndef KEFIR_COMPILER_INCREMENTAL_TOKEN_CURSOR_H_
#define KEFIR_COMPILER_INCREMENTAL_TOKEN_CURSOR_H_

#include "kefir/lexer/lexem.h"
#include "kefir/lexer/buffer.h"
#include "kefir/preprocessor/preprocessor.h"

typedef struct kefir_token_incremental_cursor_handle {
    struct kefir_token_cursor_handle handle;
    kefir_bool_t preprocessor_mode;

    struct kefir_token_buffer buffer;
    struct kefir_token_buffer pp_buffer;
    kefir_size_t cursor_offset;

    struct kefir_mem *mem;
    struct kefir_preprocessor_state preprocessor_state;
} kefir_token_incremental_cursor_handle_t;

kefir_result_t kefir_token_incremental_cursor_handle_init(struct kefir_mem *, struct kefir_preprocessor *,
                                                          struct kefir_token_allocator *,
                                                          struct kefir_token_incremental_cursor_handle *);
kefir_result_t kefir_token_incremental_cursor_handle_free(struct kefir_token_incremental_cursor_handle *);

kefir_result_t kefir_token_incremental_cursor_handle_flush_pp_tokens(struct kefir_mem *,
                                                                     struct kefir_token_incremental_cursor_handle *);

#endif