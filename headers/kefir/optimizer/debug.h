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

#ifndef KEFIR_OPTIMIZER_DEBUG_H_
#define KEFIR_OPTIMIZER_DEBUG_H_

#include "kefir/core/hashtree.h"
#include "kefir/core/source_location.h"
#include "kefir/optimizer/code.h"

typedef struct kefir_opt_code_debug_info {
    struct kefir_opt_code_event_listener listener;

    struct kefir_hashtree source_locations;
    const struct kefir_source_location *source_location_cursor;
    struct kefir_hashtree instruction_locations;

    struct kefir_hashtree local_variable_refs;
} kefir_opt_code_debug_info_t;

kefir_result_t kefir_opt_code_debug_info_init(struct kefir_opt_code_debug_info *);
kefir_result_t kefir_opt_code_debug_info_free(struct kefir_mem *, struct kefir_opt_code_debug_info *);

kefir_result_t kefir_opt_code_debug_info_set_source_location_cursor(struct kefir_mem *,
                                                                    struct kefir_opt_code_debug_info *,
                                                                    const struct kefir_source_location *);
kefir_result_t kefir_opt_code_debug_info_set_source_location_cursor_of(struct kefir_opt_code_debug_info *,
                                                                       kefir_opt_instruction_ref_t);

kefir_result_t kefir_opt_code_debug_info_source_location_of(const struct kefir_opt_code_debug_info *,
                                                            kefir_opt_instruction_ref_t,
                                                            const struct kefir_source_location **);

kefir_result_t kefir_opt_code_debug_info_add_local_variable_ref(struct kefir_mem *, struct kefir_opt_code_debug_info *,
                                                                kefir_opt_instruction_ref_t, kefir_size_t);

typedef struct kefir_opt_code_debug_info_local_variable_ref_iterator {
    struct kefir_hashtreeset_iterator iter;
} kefir_opt_code_debug_info_local_variable_ref_iterator_t;

kefir_result_t kefir_opt_code_debug_info_local_variable_ref_iter(
    const struct kefir_opt_code_debug_info *, struct kefir_opt_code_debug_info_local_variable_ref_iterator *,
    kefir_opt_instruction_ref_t, kefir_size_t *);
kefir_result_t kefir_opt_code_debug_info_local_variable_ref_next(
    struct kefir_opt_code_debug_info_local_variable_ref_iterator *, kefir_size_t *);

#endif
