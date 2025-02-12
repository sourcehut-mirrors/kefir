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

#ifndef KEFIR_OPTIMIZER_STRUCTURE_H_
#define KEFIR_OPTIMIZER_STRUCTURE_H_

#include "kefir/optimizer/code.h"
#include "kefir/core/list.h"
#include "kefir/core/bucketset.h"

typedef struct kefir_opt_code_structure_block {
    struct kefir_list predecessors;
    struct kefir_list successors;
    kefir_opt_block_id_t immediate_dominator;
} kefir_opt_code_structure_block_t;

typedef struct kefir_opt_code_structure {
    const struct kefir_opt_code_container *code;
    struct kefir_opt_code_structure_block *blocks;
    struct kefir_hashtreeset indirect_jump_target_blocks;
    struct kefir_bucketset sequenced_before;
} kefir_opt_code_structure_t;

kefir_result_t kefir_opt_code_structure_init(struct kefir_opt_code_structure *);
kefir_result_t kefir_opt_code_structure_free(struct kefir_mem *, struct kefir_opt_code_structure *);

kefir_result_t kefir_opt_code_structure_build(struct kefir_mem *, struct kefir_opt_code_structure *,
                                              const struct kefir_opt_code_container *);
kefir_result_t kefir_opt_code_structure_drop_sequencing_cache(struct kefir_mem *, struct kefir_opt_code_structure *);

kefir_result_t kefir_opt_code_structure_link_blocks(struct kefir_mem *, struct kefir_opt_code_structure *);
kefir_result_t kefir_opt_code_structure_find_dominators(struct kefir_mem *, struct kefir_opt_code_structure *);

typedef struct kefir_opt_code_container_tracer {
    kefir_result_t (*trace_instruction)(kefir_opt_instruction_ref_t, void *);
    void *payload;
} kefir_opt_code_container_tracer_t;
kefir_result_t kefir_opt_code_container_trace(struct kefir_mem *, const struct kefir_opt_code_container *,
                                              const struct kefir_opt_code_container_tracer *);

kefir_result_t kefir_opt_code_structure_is_dominator(const struct kefir_opt_code_structure *, kefir_opt_block_id_t,
                                                     kefir_opt_block_id_t, kefir_bool_t *);

kefir_result_t kefir_opt_code_structure_is_sequenced_before(struct kefir_mem *, struct kefir_opt_code_structure *,
                                                            kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t,
                                                            kefir_bool_t *);

#endif
