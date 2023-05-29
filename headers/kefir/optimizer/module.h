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

#ifndef KEFIR_OPTIMIZER_MODULE_H_
#define KEFIR_OPTIMIZER_MODULE_H_

#include "kefir/optimizer/type.h"
#include "kefir/optimizer/function.h"
#include "kefir/ir/module.h"

typedef struct kefir_opt_module {
    struct kefir_ir_module *ir_module;

    struct kefir_hashtree type_descriptors;
    struct kefir_hashtree functions;
} kefir_opt_module_t;

kefir_result_t kefir_opt_module_init(struct kefir_mem *, struct kefir_ir_module *, struct kefir_opt_module *);
kefir_result_t kefir_opt_module_construct(struct kefir_mem *, const struct kefir_ir_target_platform *,
                                          struct kefir_opt_module *);
kefir_result_t kefir_opt_module_free(struct kefir_mem *, struct kefir_opt_module *);

kefir_result_t kefir_opt_module_get_type(const struct kefir_opt_module *, kefir_id_t,
                                         const struct kefir_opt_type_descriptor **);
kefir_result_t kefir_opt_module_get_function(const struct kefir_opt_module *, kefir_id_t,
                                             const struct kefir_opt_function **);

#endif
