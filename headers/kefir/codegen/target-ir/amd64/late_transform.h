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

#ifndef KEFIR_CODEGEN_TARGET_IR_AMD64_LATE_TRANSFORM_H_
#define KEFIR_CODEGEN_TARGET_IR_AMD64_LATE_TRANSFORM_H_

#include "kefir/codegen/target-ir/code.h"
#include "kefir/codegen/asmcmp/context.h"
#include "kefir/codegen/target-ir/regalloc.h"

kefir_result_t kefir_codegen_target_ir_amd64_transform_late_jump_propagation(
    struct kefir_mem *, struct kefir_codegen_target_ir_code *,
    const struct kefir_codegen_target_ir_regalloc *);
kefir_result_t kefir_codegen_target_ir_amd64_transform_late_peephole(
    struct kefir_mem *, const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t,
    const struct kefir_codegen_target_ir_regalloc *, const struct kefir_hashset *, struct kefir_asmcmp_instruction *);

#endif
