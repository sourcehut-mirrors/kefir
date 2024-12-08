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

#ifndef KEFIR_LEXER_ALLOCATOR_H_
#define KEFIR_LEXER_ALLOCATOR_H_

#include "kefir/lexer/lexem.h"

typedef struct kefir_token_allocator_chunk {
    struct kefir_token_allocator_chunk *prev_chunk;
    struct kefir_token tokens[];
} kefir_token_allocator_chunk_t;

typedef struct kefir_token_allocator {
    kefir_size_t chunk_capacity;
    kefir_size_t last_token_index;
    struct kefir_token_allocator_chunk *last_chunk;
} kefir_token_allocator_t;

kefir_result_t kefir_token_allocator_init(struct kefir_token_allocator *);
kefir_result_t kefir_token_allocator_free(struct kefir_mem *, struct kefir_token_allocator *);

kefir_result_t kefir_token_allocator_allocate(struct kefir_mem *, struct kefir_token_allocator *, struct kefir_token *,
                                              const struct kefir_token **);

#endif
