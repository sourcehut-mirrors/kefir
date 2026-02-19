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

#ifndef KEFIR_OPTIMIZER_MEM2REG_UTIL_H_
#define KEFIR_OPTIMIZER_MEM2REG_UTIL_H_

#include "kefir/optimizer/code.h"
#include "kefir/optimizer/debug.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/ir/module.h"

kefir_result_t kefir_opt_code_util_extend_load_value(struct kefir_mem *, struct kefir_opt_code_container *,
                                                     const struct kefir_ir_module *,
                                                     const struct kefir_opt_instruction *, kefir_opt_instruction_ref_t,
                                                     kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_util_mem2reg_apply(struct kefir_mem *, const struct kefir_ir_module *,
                                                 struct kefir_opt_code_container *, struct kefir_opt_code_debug_info *,
                                                 const struct kefir_opt_code_control_flow *,
                                                 const struct kefir_hashset *);

#endif
