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

#ifndef KEFIR_CORE_QUEUE_H_
#define KEFIR_CORE_QUEUE_H_

#include "kefir/core/list.h"

typedef kefir_uptr_t kefir_queue_entry_t;

typedef struct kefir_queue {
    struct kefir_list blocks;
    kefir_size_t head_index;
    kefir_size_t tail_index;
} kefir_queue_t;

kefir_result_t kefir_queue_init(struct kefir_queue *);
kefir_result_t kefir_queue_free(struct kefir_mem *, struct kefir_queue *);

kefir_bool_t kefir_queue_is_empty(const struct kefir_queue *);
kefir_result_t kefir_queue_push(struct kefir_mem *, struct kefir_queue *, kefir_queue_entry_t);
kefir_result_t kefir_queue_pop_first(struct kefir_mem *, struct kefir_queue *, kefir_queue_entry_t *);

#endif
