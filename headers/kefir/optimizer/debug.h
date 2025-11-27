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

#ifndef KEFIR_OPTIMIZER_DEBUG_H_
#define KEFIR_OPTIMIZER_DEBUG_H_

#include "kefir/core/hashtree.h"
#include "kefir/core/source_location.h"
#include "kefir/optimizer/code.h"
#include "kefir/core/hashset.h"

#define KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE ((kefir_size_t) - 1ll)

typedef kefir_uint64_t kefir_opt_code_debug_info_local_variable_ref_t;

typedef struct kefir_opt_code_debug_info_allocation_placement {
    kefir_opt_instruction_ref_t allocation_ref;
    struct kefir_hashset placement;
} kefir_opt_code_debug_info_allocation_placement_t;

typedef struct kefir_opt_code_debug_info_local_variable {
    kefir_opt_code_debug_info_local_variable_ref_t variable_ref;
    struct kefir_hashset allocations;
} kefir_opt_code_debug_info_local_variable_t;

typedef struct kefir_opt_code_debug_info {
    kefir_bool_t record_debug_info;
    struct kefir_opt_code_event_listener listener;

    kefir_size_t instruction_location_cursor;
    struct kefir_hashtree instruction_locations;

    struct kefir_hashset active_refs;
    struct kefir_hashtable local_variables;
    struct kefir_hashtable allocations;
} kefir_opt_code_debug_info_t;

kefir_result_t kefir_opt_code_debug_info_init(struct kefir_opt_code_debug_info *);
kefir_result_t kefir_opt_code_debug_info_free(struct kefir_mem *, struct kefir_opt_code_debug_info *);

kefir_result_t kefir_opt_code_debug_info_set_instruction_location_cursor(struct kefir_opt_code_debug_info *,
                                                                         kefir_size_t);
kefir_result_t kefir_opt_code_debug_info_set_instruction_location_cursor_of(struct kefir_opt_code_debug_info *,
                                                                            kefir_opt_instruction_ref_t);

kefir_result_t kefir_opt_code_debug_info_instruction_location(const struct kefir_opt_code_debug_info *,
                                                              kefir_opt_instruction_ref_t, kefir_size_t *);

kefir_result_t kefir_opt_code_debug_info_add_local_variable_allocation(struct kefir_mem *, struct kefir_opt_code_debug_info *, kefir_opt_code_debug_info_local_variable_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_debug_info_add_allocation_placement(struct kefir_mem *, struct kefir_opt_code_debug_info *, kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_opt_code_debug_info_local_variable(const struct kefir_opt_code_debug_info *, kefir_opt_code_debug_info_local_variable_ref_t, const struct kefir_opt_code_debug_info_local_variable **);
kefir_result_t kefir_opt_code_debug_info_allocation_placement(const struct kefir_opt_code_debug_info *, kefir_opt_instruction_ref_t, const struct kefir_opt_code_debug_info_allocation_placement **);

typedef struct kefir_opt_code_debug_info_local_variable_iterator {
    struct kefir_hashtable_iterator iter;
} kefir_opt_code_debug_info_local_variable_iterator_t;

kefir_result_t kefir_opt_code_debug_info_local_variable_iter(const struct kefir_opt_code_debug_info *,
                                                             struct kefir_opt_code_debug_info_local_variable_iterator *,
                                                             const struct kefir_opt_code_debug_info_local_variable **);
kefir_result_t kefir_opt_code_debug_info_local_variable_next(struct kefir_opt_code_debug_info_local_variable_iterator *,
                                                             const struct kefir_opt_code_debug_info_local_variable **);

typedef struct kefir_opt_code_debug_info_allocation_placement_iterator {
    struct kefir_hashtable_iterator iter;
} kefir_opt_code_debug_info_allocation_placement_iterator_t;

kefir_result_t kefir_opt_code_debug_info_allocation_placement_iter(const struct kefir_opt_code_debug_info *,
                                                             struct kefir_opt_code_debug_info_allocation_placement_iterator *,
                                                             const struct kefir_opt_code_debug_info_allocation_placement **);
kefir_result_t kefir_opt_code_debug_info_allocation_placement_next(struct kefir_opt_code_debug_info_allocation_placement_iterator *,
                                                             const struct kefir_opt_code_debug_info_allocation_placement **);

#endif
