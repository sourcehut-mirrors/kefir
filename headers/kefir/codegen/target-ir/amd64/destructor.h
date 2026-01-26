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

#ifndef KEFIR_CODEGEN_TARGET_IR_AMD64_DESTRUCTOR_H_
#define KEFIR_CODEGEN_TARGET_IR_AMD64_DESTRUCTOR_H_

#include "kefir/codegen/target-ir/constructor.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/schedule.h"
#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/codegen/target-ir/destructor_ops.h"
#include "kefir/codegen/amd64/asmcmp.h"

kefir_result_t kefir_codegen_target_ir_amd64_destruct(
    struct kefir_mem *, const struct kefir_codegen_target_ir_code *, struct kefir_asmcmp_amd64 *,
    const struct kefir_codegen_target_ir_stack_frame *, const struct kefir_codegen_target_ir_control_flow *,
    const struct kefir_codegen_target_ir_liveness *, const struct kefir_codegen_target_ir_interference *,
    const struct kefir_codegen_target_ir_regalloc *, const struct kefir_codegen_target_ir_code_constructor_metadata *,
    const struct kefir_codegen_target_ir_destructor_ops *);

#endif
