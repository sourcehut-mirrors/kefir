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

#ifndef KEFIR_OPTIMIZER_MODULE_LIVENESS_H_
#define KEFIR_OPTIMIZER_MODULE_LIVENESS_H_

#include "kefir/optimizer/module.h"

typedef struct kefir_opt_module_liveness {
    struct kefir_hashtreeset symbols;
    struct kefir_hashtreeset string_literals;
} kefir_opt_module_liveness_t;

kefir_result_t kefir_opt_module_liveness_init(struct kefir_opt_module_liveness *);
kefir_result_t kefir_opt_module_liveness_free(struct kefir_mem *, struct kefir_opt_module_liveness *);

kefir_result_t kefir_opt_module_liveness_trace(struct kefir_mem *, struct kefir_opt_module_liveness *,
                                               const struct kefir_opt_module *);

kefir_bool_t kefir_opt_module_is_symbol_alive(const struct kefir_opt_module_liveness *, const char *);
kefir_bool_t kefir_opt_module_is_string_literal_alive(const struct kefir_opt_module_liveness *, kefir_id_t);

#endif
