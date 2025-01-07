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

#ifndef KEFIR_CORE_STRING_POOL_H_
#define KEFIR_CORE_STRING_POOL_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/core/hashtree.h"

typedef struct kefir_string_pool {
    struct kefir_hashtree strings;
    struct kefir_hashtree named_strings;
    kefir_size_t next_id;
} kefir_string_pool_t;

kefir_result_t kefir_string_pool_init(struct kefir_string_pool *);
kefir_result_t kefir_string_pool_free(struct kefir_mem *, struct kefir_string_pool *);
const char *kefir_string_pool_insert(struct kefir_mem *, struct kefir_string_pool *, const char *, kefir_id_t *);
const char *kefir_string_pool_get(const struct kefir_string_pool *, kefir_id_t);

#endif
