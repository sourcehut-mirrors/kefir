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

#ifndef KEFIR_IR_DEBUG_H_
#define KEFIR_IR_DEBUG_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/string_pool.h"
#include "kefir/core/source_location.h"
#include "kefir/core/interval_tree.h"

typedef struct kefir_ir_source_location {
    struct kefir_source_location location;
    kefir_size_t begin;
    kefir_size_t end;

    struct kefir_ir_source_location *next;
} kefir_ir_source_location_t;

typedef struct kefir_ir_source_map {
    struct kefir_interval_tree locations;
} kefir_ir_source_map_t;

typedef struct kefir_ir_source_map_iterator {
    struct kefir_interval_tree_iterator iter;
    struct kefir_ir_source_location *source_location;
} kefir_ir_source_map_iterator_t;

typedef struct kefir_ir_function_debug_info {
    struct kefir_ir_source_map source_map;
} kefir_ir_function_debug_info_t;

kefir_result_t kefir_ir_source_map_init(struct kefir_ir_source_map *);
kefir_result_t kefir_ir_source_map_free(struct kefir_mem *, struct kefir_ir_source_map *);

kefir_result_t kefir_ir_source_map_insert(struct kefir_mem *, struct kefir_ir_source_map *, struct kefir_string_pool *,
                                          const struct kefir_source_location *, kefir_size_t, kefir_size_t);
kefir_result_t kefir_ir_source_map_find(const struct kefir_ir_source_map *, kefir_size_t,
                                      const struct kefir_ir_source_location **);

kefir_result_t kefir_ir_source_map_iter(const struct kefir_ir_source_map *, struct kefir_ir_source_map_iterator *,
                                        const struct kefir_ir_source_location **);
kefir_result_t kefir_ir_source_map_next(struct kefir_ir_source_map_iterator *,
                                        const struct kefir_ir_source_location **);

kefir_result_t kefir_ir_function_debug_info_init(struct kefir_ir_function_debug_info *);
kefir_result_t kefir_ir_function_debug_info_free(struct kefir_mem *, struct kefir_ir_function_debug_info *);

#endif
