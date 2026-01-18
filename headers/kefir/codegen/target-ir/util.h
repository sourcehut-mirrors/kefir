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

#ifndef KEFIR_CODEGEN_TARGET_IR_UTIL_H_
#define KEFIR_CODEGEN_TARGET_IR_UTIL_H_

#include "kefir/codegen/target-ir/code.h"

kefir_int64_t kefir_codegen_target_ir_sign_extend(kefir_int64_t, kefir_codegen_target_ir_operand_variant_t);
kefir_result_t kefir_codegen_target_ir_add_produced_resource_aspects(struct kefir_mem *, struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t);
kefir_result_t kefir_codegen_target_ir_code_get_single_user(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_value_ref_t, kefir_codegen_target_ir_instruction_ref_t *);

#endif
