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

#ifndef KEFIR_CORE_MEMORY_ARENA_H_
#define KEFIR_CORE_MEMORY_ARENA_H_

#include "kefir/core/mem.h"

typedef struct kefir_memory_arena_chunk {
    kefir_size_t top;
    struct kefir_memory_arena_chunk *prev;
    _Alignas(kefir_max_align_t) char chunk[];
} kefir_memory_arena_chunk_t;

typedef struct kefir_memory_arena {
    struct kefir_mem *mem;
    struct kefir_memory_arena_chunk *chunk;
    struct kefir_memory_arena_chunk *special_chunk;
} kefir_memory_arena_t;

kefir_result_t kefir_memory_arena_init(struct kefir_mem *, struct kefir_memory_arena *);
kefir_result_t kefir_memory_arena_free(struct kefir_memory_arena *);

void *kefir_memory_arena_alloc(struct kefir_memory_arena *, kefir_size_t, kefir_size_t);

#endif
