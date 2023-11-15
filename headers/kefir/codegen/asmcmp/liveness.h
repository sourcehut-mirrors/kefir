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

#ifndef KEFIR_CODDEGEN_ASMCMP_LIVENESS_H_
#define KEFIR_CODDEGEN_ASMCMP_LIVENESS_H_

#include "kefir/codegen/asmcmp/type_defs.h"
#include "kefir/core/hashtree.h"

typedef struct kefir_asmcmp_virtual_register_liveness_map {
    struct kefir_hashtree lifetime_ranges;

    struct {
        kefir_asmcmp_linear_reference_index_t begin;
        kefir_asmcmp_linear_reference_index_t end;
    } global_reference_range;
} kefir_asmcmp_virtual_register_liveness_map_t;

typedef struct kefir_asmcmp_liveness_map {
    struct kefir_asmcmp_virtual_register_liveness_map *map;
    kefir_size_t length;
    kefir_size_t capacity;
} kefir_asmcmp_liveness_map_t;

kefir_result_t kefir_asmcmp_liveness_map_init(struct kefir_asmcmp_liveness_map *);
kefir_result_t kefir_asmcmp_liveness_map_free(struct kefir_mem *, struct kefir_asmcmp_liveness_map *);

kefir_result_t kefir_asmcmp_liveness_map_resize(struct kefir_mem *, struct kefir_asmcmp_liveness_map *, kefir_size_t);
kefir_result_t kefir_asmcmp_liveness_map_mark_activity(struct kefir_asmcmp_liveness_map *,
                                                       kefir_asmcmp_virtual_register_index_t,
                                                       kefir_asmcmp_linear_reference_index_t);
kefir_result_t kefir_asmcmp_liveness_map_add_lifetime_range(struct kefir_mem *, struct kefir_asmcmp_liveness_map *,
                                                            kefir_asmcmp_virtual_register_index_t,
                                                            kefir_asmcmp_linear_reference_index_t,
                                                            kefir_asmcmp_linear_reference_index_t);

kefir_result_t kefir_asmcmp_get_active_lifetime_range_for(const struct kefir_asmcmp_liveness_map *,
                                                          kefir_asmcmp_virtual_register_index_t,
                                                          kefir_asmcmp_linear_reference_index_t,
                                                          kefir_asmcmp_linear_reference_index_t *,
                                                          kefir_asmcmp_linear_reference_index_t *);

#endif
