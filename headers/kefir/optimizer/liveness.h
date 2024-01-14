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

#ifndef KEFIR_OPTIMIZER_LIVENESS_H_
#define KEFIR_OPTIMIZER_LIVENESS_H_

#include "kefir/core/hashtree.h"
#include "kefir/core/hashtreeset.h"
#include "kefir/optimizer/code.h"

typedef struct kefir_opt_code_analysis kefir_opt_code_analysis_t;  // Forward declaration

typedef struct kefir_opt_instruction_liveness_interval {
    kefir_opt_instruction_ref_t instr_ref;
    kefir_opt_instruction_ref_t alias_ref;
    struct {
        kefir_size_t begin;
        kefir_size_t end;
    } range;
} kefir_opt_instruction_liveness_interval_t;

typedef struct kefir_opt_code_liveness_intervals {
    const struct kefir_opt_code_analysis *analysis;
    struct kefir_opt_instruction_liveness_interval *intervals;
} kefir_opt_code_liveness_intervals_t;

kefir_result_t kefir_opt_code_liveness_intervals_build(struct kefir_mem *, const struct kefir_opt_code_analysis *,
                                                       struct kefir_opt_code_liveness_intervals *);
kefir_result_t kefir_opt_code_liveness_intervals_free(struct kefir_mem *, struct kefir_opt_code_liveness_intervals *);

#endif
