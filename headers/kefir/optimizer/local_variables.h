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

#ifndef KEFIR_OPTIMIZER_LOCAL_VARIABLES_H_
#define KEFIR_OPTIMIZER_LOCAL_VARIABLES_H_

#include "kefir/optimizer/liveness.h"
#include "kefir/core/hashset.h"

typedef struct kefir_opt_code_variable_local_conflicts {
    struct kefir_hashset local_conflicts;
} kefir_opt_code_variable_local_conflicts_t;

typedef struct kefir_opt_code_variable_conflicts {
    struct kefir_hashset globally_alive;
    struct kefir_hashtree locally_alive;
} kefir_opt_code_variable_conflicts_t;

kefir_result_t kefir_opt_code_variable_conflicts_init(struct kefir_opt_code_variable_conflicts *);
kefir_result_t kefir_opt_code_variable_conflicts_free(struct kefir_mem *, struct kefir_opt_code_variable_conflicts *);

kefir_result_t kefir_opt_code_variable_conflicts_build(struct kefir_mem *, struct kefir_opt_code_variable_conflicts *, const struct kefir_opt_code_liveness *);

#endif
