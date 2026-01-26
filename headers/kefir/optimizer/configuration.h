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

#ifndef KEFIR_OPTIMIZER_CONFIGURATION_H_
#define KEFIR_OPTIMIZER_CONFIGURATION_H_

#include "kefir/optimizer/pipeline.h"
#include "kefir/core/data_model.h"

typedef struct kefir_optimizer_configuration {
    struct kefir_optimizer_pipeline pipeline;

    kefir_bool_t debug_info;
    kefir_size_t max_inline_depth;
    kefir_size_t max_inlines_per_function;
    kefir_size_t imprecise_decimal_bitint_conv;
    kefir_data_model_decimal_encoding_t decimal_encoding;
    const struct kefir_optimizer_target_lowering *target_lowering;
} kefir_optimizer_configuration_t;

kefir_result_t kefir_optimizer_configuration_init(struct kefir_optimizer_configuration *);
kefir_result_t kefir_optimizer_configuration_free(struct kefir_mem *, struct kefir_optimizer_configuration *);

kefir_result_t kefir_optimizer_configuration_add_pipeline_pass(struct kefir_mem *,
                                                               struct kefir_optimizer_configuration *, const char *);
kefir_result_t kefir_optimizer_configuration_copy_from(struct kefir_mem *, struct kefir_optimizer_configuration *,
                                                       const struct kefir_optimizer_configuration *);

#endif
