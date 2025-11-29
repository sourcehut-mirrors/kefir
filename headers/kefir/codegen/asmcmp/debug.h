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

#ifndef KEFIR_CODEGEN_ASMCMP_DEBUG_H_
#define KEFIR_CODEGEN_ASMCMP_DEBUG_H_

#include "kefir/codegen/asmcmp/type_defs.h"
#include "kefir/core/string_pool.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/hashtable.h"
#include "kefir/core/source_location.h"

typedef kefir_uint64_t kefir_asmcmp_debug_info_code_reference_t;
typedef kefir_uint64_t kefir_asmcmp_debug_info_value_reference_t;

typedef struct kefir_asmcmp_debug_info_source_map {
    struct kefir_hashtree map;
} kefir_asmcmp_debug_info_source_map_t;

typedef struct kefir_asmcmp_debug_info_code_fragment {
    kefir_asmcmp_label_index_t begin_label;
    kefir_asmcmp_label_index_t end_label;
} kefir_asmcmp_debug_info_code_fragment_t;

typedef struct kefir_asmcmp_debug_info_code_map_entry {
    struct kefir_asmcmp_debug_info_code_fragment *fragments;
    kefir_size_t fragments_length;
    kefir_size_t fragments_capacity;
} kefir_asmcmp_debug_info_code_map_entry_t;

typedef struct kefir_asmcmp_debug_info_code_map {
    struct kefir_hashtable fragments;
} kefir_asmcmp_debug_info_code_map_t;

typedef struct kefir_asmcmp_debug_info_value_fragment {
    kefir_asmcmp_virtual_register_index_t vreg_idx;
    kefir_asmcmp_label_index_t begin_label;
    kefir_asmcmp_label_index_t end_label;
} kefir_asmcmp_debug_info_value_fragment_t;

typedef struct kefir_asmcmp_debug_info_value_map_entry {
    struct kefir_asmcmp_debug_info_value_fragment *fragments;
    kefir_size_t fragments_length;
    kefir_size_t fragments_capacity;
} kefir_asmcmp_debug_info_value_map_entry_t;

typedef struct kefir_asmcmp_debug_info_value_map {
    struct kefir_hashtable fragments;
} kefir_asmcmp_debug_info_value_map_t;

typedef struct kefir_asmcmp_debug_info {
    struct kefir_asmcmp_debug_info_source_map source_map;
    struct kefir_asmcmp_debug_info_code_map code_map;
    struct kefir_asmcmp_debug_info_value_map value_map;
} kefir_asmcmp_debug_info_t;

kefir_result_t kefir_asmcmp_debug_info_source_map_init(struct kefir_asmcmp_debug_info_source_map *);
kefir_result_t kefir_asmcmp_debug_info_source_map_free(struct kefir_mem *, struct kefir_asmcmp_debug_info_source_map *);

kefir_result_t kefir_asmcmp_debug_info_code_map_init(struct kefir_asmcmp_debug_info_code_map *);
kefir_result_t kefir_asmcmp_debug_info_code_map_free(struct kefir_mem *, struct kefir_asmcmp_debug_info_code_map *);

kefir_result_t kefir_asmcmp_debug_info_value_map_init(struct kefir_asmcmp_debug_info_value_map *);
kefir_result_t kefir_asmcmp_debug_info_value_map_free(struct kefir_mem *, struct kefir_asmcmp_debug_info_value_map *);

kefir_result_t kefir_asmcmp_debug_info_init(struct kefir_asmcmp_debug_info *);
kefir_result_t kefir_asmcmp_debug_info_free(struct kefir_mem *, struct kefir_asmcmp_debug_info *);

kefir_result_t kefir_asmcmp_debug_info_source_map_add_location(struct kefir_mem *, struct kefir_asmcmp_debug_info_source_map *,
                                                    struct kefir_string_pool *, kefir_asmcmp_instruction_index_t,
                                                    kefir_asmcmp_instruction_index_t,
                                                    const struct kefir_source_location *);
kefir_result_t kefir_asmcmp_debug_info_source_map_at(const struct kefir_asmcmp_debug_info_source_map *, kefir_asmcmp_instruction_index_t,
                                          const struct kefir_source_location **);

kefir_result_t kefir_asmcmp_code_map_add_fragment(struct kefir_mem *, struct kefir_asmcmp_debug_info_code_map *, kefir_asmcmp_debug_info_code_reference_t, kefir_asmcmp_label_index_t, kefir_asmcmp_label_index_t);
kefir_result_t kefir_asmcmp_value_map_add_fragment(struct kefir_mem *, struct kefir_asmcmp_debug_info_value_map *, kefir_asmcmp_debug_info_value_reference_t, kefir_asmcmp_virtual_register_index_t, kefir_asmcmp_label_index_t, kefir_asmcmp_label_index_t);

#endif
