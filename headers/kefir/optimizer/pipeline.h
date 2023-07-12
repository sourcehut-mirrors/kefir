/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#ifndef KEFIR_OPTIMIZER_PIPELINE_H_
#define KEFIR_OPTIMIZER_PIPELINE_H_

#include "kefir/optimizer/module.h"
#include "kefir/core/list.h"

typedef struct kefir_optimizer_pass {
    const char *name;
    kefir_result_t (*apply)(struct kefir_mem *, const struct kefir_opt_module *, struct kefir_opt_function *,
                            const struct kefir_optimizer_pass *);
    void *payload;
} kefir_optimizer_pass_t;

typedef struct kefir_optimizer_pipeline {
    struct kefir_list pipeline;
} kefir_optimizer_pipeline_t;

kefir_result_t kefir_optimizer_pass_resolve(const char *, const struct kefir_optimizer_pass **);

kefir_result_t kefir_optimizer_pipeline_init(struct kefir_optimizer_pipeline *);
kefir_result_t kefir_optimizer_pipeline_free(struct kefir_mem *, struct kefir_optimizer_pipeline *);

kefir_result_t kefir_optimizer_pipeline_add(struct kefir_mem *, struct kefir_optimizer_pipeline *,
                                            const struct kefir_optimizer_pass *);

kefir_result_t kefir_optimizer_pipeline_apply(struct kefir_mem *, const struct kefir_opt_module *,
                                              const struct kefir_optimizer_pipeline *);
kefir_result_t kefir_optimizer_pipeline_apply_function(struct kefir_mem *, const struct kefir_opt_module *, kefir_id_t,
                                                       const struct kefir_optimizer_pipeline *);

#ifdef KEFIR_OPTIMIZER_PIPELINE_INTERNAL
#define DECLARE_PASS(_id) extern const struct kefir_optimizer_pass KefirOptimizerPass##_id
DECLARE_PASS(Noop);
DECLARE_PASS(CompareBranchFuse);
#undef DECLARE_PASS
#endif

#endif
