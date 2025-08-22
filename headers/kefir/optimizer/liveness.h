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

#ifndef KEFIR_OPTIMIZER_LIVENESS_H_
#define KEFIR_OPTIMIZER_LIVENESS_H_

#include "kefir/optimizer/structure.h"
#include "kefir/core/hashset.h"

typedef struct kefir_opt_code_liveness_block {
    struct kefir_hashset alive_instr;
} kefir_opt_code_liveness_block_t;

typedef struct kefir_opt_code_liveness {
    const struct kefir_opt_code_container *code;
    struct kefir_opt_code_liveness_block *blocks;
} kefir_opt_code_liveness_t;

kefir_result_t kefir_opt_code_liveness_init(struct kefir_opt_code_liveness *);
kefir_result_t kefir_opt_code_liveness_free(struct kefir_mem *, struct kefir_opt_code_liveness *);

kefir_result_t kefir_opt_code_liveness_build(struct kefir_mem *, struct kefir_opt_code_liveness *,
                                             struct kefir_opt_code_structure *);

kefir_result_t kefir_opt_code_liveness_instruction_is_alive(const struct kefir_opt_code_liveness *,
                                                            kefir_opt_instruction_ref_t, kefir_bool_t *);

#endif
