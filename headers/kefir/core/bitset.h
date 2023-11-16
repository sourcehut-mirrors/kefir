/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#ifndef KEFIR_CORE_BITSET_H_
#define KEFIR_CORE_BITSET_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"

typedef struct kefir_bitset {
    kefir_uint64_t *content;
    kefir_size_t length;
    kefir_size_t capacity;
    kefir_bool_t static_content;
} kefir_bitset_t;

kefir_result_t kefir_bitset_init(struct kefir_bitset *);
kefir_result_t kefir_bitset_init_static(struct kefir_bitset *, kefir_uint64_t *, kefir_size_t, kefir_size_t);
kefir_result_t kefir_bitset_free(struct kefir_mem *, struct kefir_bitset *);

kefir_result_t kefir_bitset_get(const struct kefir_bitset *, kefir_size_t, kefir_bool_t *);
kefir_result_t kefir_bitset_set(const struct kefir_bitset *, kefir_size_t, kefir_bool_t);
kefir_result_t kefir_bitset_find(const struct kefir_bitset *, kefir_bool_t, kefir_size_t, kefir_size_t *);
kefir_result_t kefir_bitset_set_consecutive(const struct kefir_bitset *, kefir_size_t, kefir_size_t, kefir_bool_t);
kefir_result_t kefir_bitset_find_consecutive(const struct kefir_bitset *, kefir_bool_t, kefir_size_t, kefir_size_t,
                                             kefir_size_t *);
kefir_result_t kefir_bitset_clear(const struct kefir_bitset *);

kefir_result_t kefir_bitset_length(const struct kefir_bitset *, kefir_size_t *);
kefir_result_t kefir_bitset_resize(struct kefir_mem *, struct kefir_bitset *, kefir_size_t);

#define KEFIR_BITSET_STATIC_CONTENT_CAPACITY(_bits) \
    (((_bits) + (sizeof(kefir_uint64_t) * CHAR_BIT - 1)) / (sizeof(kefir_uint64_t) * CHAR_BIT))

#endif
