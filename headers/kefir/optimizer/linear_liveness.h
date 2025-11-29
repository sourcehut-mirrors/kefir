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

#ifndef KEFIR_OPTIMIZER_LINEAR_LIVENESS_H_
#define KEFIR_OPTIMIZER_LINEAR_LIVENESS_H_

#include "kefir/core/hashtable.h"
#include "kefir/optimizer/schedule.h"
#include "kefir/optimizer/structure.h"

typedef struct kefir_opt_code_instruction_linear_liveness {
    struct kefir_hashtable per_block;
} kefir_opt_code_instruction_linear_liveness_t;

typedef struct kefir_opt_code_linear_liveness {
    struct kefir_hashtable instructions;
} kefir_opt_code_linear_liveness_t;

kefir_result_t kefir_opt_code_linear_liveness_init(struct kefir_opt_code_linear_liveness *);
kefir_result_t kefir_opt_code_linear_liveness_free(struct kefir_mem *, struct kefir_opt_code_linear_liveness *);
kefir_result_t kefir_opt_code_linear_liveness_build(struct kefir_mem *, struct kefir_opt_code_linear_liveness *, const struct kefir_opt_code_container *, const struct kefir_opt_code_structure *, const struct kefir_opt_code_schedule *);

typedef struct kefir_opt_code_instruction_linear_liveness_iterator {
    struct kefir_hashtable_iterator iter;
} kefir_opt_code_instruction_linear_liveness_iterator_t;

kefir_result_t kefir_opt_code_instruction_linear_liveness_iter(const struct kefir_opt_code_linear_liveness *, kefir_opt_instruction_ref_t, struct kefir_opt_code_instruction_linear_liveness_iterator *, kefir_opt_block_id_t *, kefir_uint32_t *, kefir_uint32_t *);
kefir_result_t kefir_opt_code_instruction_linear_liveness_next(struct kefir_opt_code_instruction_linear_liveness_iterator *, kefir_opt_block_id_t *, kefir_uint32_t *, kefir_uint32_t *);

#endif
