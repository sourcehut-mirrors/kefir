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

#ifndef KEFIR_OPTIMIZER_ITERATION_SPACE_H_
#define KEFIR_OPTIMIZER_ITERATION_SPACE_H_

#include "kefir/optimizer/loop_nest.h"

typedef enum kefir_opt_loop_iteration_space_type {
    KEFIR_OPT_LOOP_ITERATION_SPACE_STRIDED_RANGE,
    KEFIR_OPT_LOOP_ITERATION_SPACE_GENERAL_RANGE,
    KEFIR_OPT_LOOP_ITERATION_SPACE_GENERAL
} kefir_opt_loop_iteration_space_type_t;

typedef struct kefir_opt_loop_iteration_space {
    kefir_opt_loop_iteration_space_type_t type;
    union {
        struct {
            kefir_size_t comparison_width;
            kefir_bool_t signed_comparison;
            kefir_bool_t ascending;
            kefir_bool_t inclusive;
            kefir_opt_instruction_ref_t index_ref;
            kefir_opt_instruction_ref_t lower_bound_ref;
            kefir_opt_instruction_ref_t stride_ref;
            kefir_opt_instruction_ref_t upper_bound_ref;
        } range;

        struct {
            kefir_opt_instruction_ref_t init_ref;
            kefir_opt_instruction_ref_t condition_ref;
            kefir_opt_instruction_ref_t next_ref;
            kefir_opt_branch_condition_variant_t variant;
            kefir_bool_t invert;
        } general;
    };
} kefir_opt_loop_iteration_space_t;

kefir_result_t kefir_opt_loop_match_iteration_space(const struct kefir_opt_code_container *,
                                                    const struct kefir_opt_code_loop *,
                                                    struct kefir_opt_loop_iteration_space *);

kefir_result_t kefir_opt_loop_may_execute(const struct kefir_opt_code_container *,
                                          const struct kefir_opt_loop_iteration_space *, kefir_bool_t *);

#endif
