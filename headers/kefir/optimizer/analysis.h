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

#ifndef KEFIR_OPTIMIZER_ANALYSIS_H_
#define KEFIR_OPTIMIZER_ANALYSIS_H_

#include "kefir/optimizer/module.h"
#include "kefir/core/hashtree.h"

#define KEFIR_OPT_CODE_ANALYSIS_LINEAR_INDEX_UNDEFINED (~(kefir_size_t) 0ull)

typedef struct kefir_opt_code_analysis_block_properties {
    kefir_opt_block_id_t block_id;
    kefir_bool_t reachable;
    kefir_size_t linear_position;
    struct kefir_list predecessors;
    struct kefir_list successors;
} kefir_opt_code_analysis_block_properties_t;

typedef struct kefir_opt_code_analysis {
    const struct kefir_opt_code_container *code;

    struct kefir_opt_code_analysis_block_properties *blocks;

    struct kefir_list indirect_jump_target_blocks;

    kefir_size_t block_linearization_length;
    struct kefir_opt_code_analysis_block_properties **block_linearization;
} kefir_opt_code_analysis_t;

kefir_result_t kefir_opt_code_analyze(struct kefir_mem *, const struct kefir_opt_code_container *,
                                      struct kefir_opt_code_analysis *);
kefir_result_t kefir_opt_code_analysis_free(struct kefir_mem *, struct kefir_opt_code_analysis *);

kefir_result_t kefir_opt_code_analysis_block_properties(const struct kefir_opt_code_analysis *, kefir_opt_block_id_t,
                                                        const struct kefir_opt_code_analysis_block_properties **);

typedef struct kefir_opt_code_analyze_block_scheduler {
    kefir_result_t (*schedule)(struct kefir_mem *, const struct kefir_opt_code_analyze_block_scheduler *,
                               const struct kefir_opt_code_container *,
                               kefir_result_t (*)(kefir_opt_block_id_t, void *), void *);
    void *payload;
} kefir_opt_code_analyze_block_scheduler_t;

extern const struct kefir_opt_code_analyze_block_scheduler kefir_opt_code_analyze_bfs_block_scheduler;

#ifdef KEFIR_OPTIMIZER_ANALYSIS_INTERNAL
kefir_result_t kefir_opt_code_analyze_reachability(struct kefir_mem *, struct kefir_opt_code_analysis *);
kefir_result_t kefir_opt_code_analyze_linearize(struct kefir_mem *, struct kefir_opt_code_analysis *,
                                                const struct kefir_opt_code_analyze_block_scheduler *);
#endif

typedef struct kefir_opt_module_analysis {
    struct kefir_opt_module *module;
    struct kefir_hashtree functions;
} kefir_opt_module_analysis_t;

kefir_result_t kefir_opt_module_analyze(struct kefir_mem *, struct kefir_opt_module *,
                                        struct kefir_opt_module_analysis *);
kefir_result_t kefir_opt_module_analysis_free(struct kefir_mem *, struct kefir_opt_module_analysis *);

kefir_result_t kefir_opt_module_analysis_get_function(const struct kefir_opt_module_analysis *, kefir_id_t,
                                                      const struct kefir_opt_code_analysis **);

#endif
