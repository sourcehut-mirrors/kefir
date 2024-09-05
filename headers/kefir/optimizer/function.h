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

#ifndef KEFIR_OPTIMIZER_FUNCTION_H_
#define KEFIR_OPTIMIZER_FUNCTION_H_

#include "kefir/optimizer/code.h"
#include "kefir/optimizer/debug.h"
#include "kefir/optimizer/type.h"
#include "kefir/ir/function.h"

typedef struct kefir_opt_module kefir_opt_module_t;  // Forward declaration

typedef struct kefir_opt_function {
    const struct kefir_ir_function *ir_func;
    struct kefir_opt_code_container code;
    struct kefir_opt_code_debug_info debug_info;
    struct {
        const struct kefir_ir_type *type;
        kefir_id_t type_id;
    } locals;
} kefir_opt_function_t;

kefir_result_t kefir_opt_function_init(const struct kefir_opt_module *, const struct kefir_ir_function *,
                                       struct kefir_opt_function *);
kefir_result_t kefir_opt_function_free(struct kefir_mem *, struct kefir_opt_function *);

#endif
