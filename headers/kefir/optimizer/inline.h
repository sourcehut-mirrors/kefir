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

#ifndef KEFIR_OPTIMZIER_INLINE_H_
#define KEFIR_OPTIMZIER_INLINE_H_

#include "kefir/optimizer/code.h"
#include "kefir/optimizer/structure.h"
#include "kefir/optimizer/module.h"

typedef struct kefir_opt_try_inline_function_call_parameters {
    kefir_size_t max_inline_depth;
    kefir_size_t max_inlines_per_function;
} kefir_opt_try_inline_function_call_parameters_t;

kefir_result_t kefir_opt_try_inline_function_call(struct kefir_mem *, const struct kefir_opt_module *,
                                                  struct kefir_opt_function *, struct kefir_opt_code_structure *,
                                                  const struct kefir_opt_try_inline_function_call_parameters *,
                                                  kefir_opt_instruction_ref_t, kefir_bool_t *);

#endif
