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

#ifndef KEFIR_CODEGEN_AMD64_TARGET_IR_H_
#define KEFIR_CODEGEN_AMD64_TARGET_IR_H_

#include "kefir/codegen/target-ir/code.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/codegen/target-ir/interference.h"
#include "kefir/codegen/target-ir/coalesce.h"
#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/codegen/target-ir/amd64/regalloc.h"
#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/translator.h"
#include "kefir/codegen/target-ir/constructor.h"

typedef struct kefir_codegen_amd64_function_target_ir {
    struct kefir_codegen_target_ir_code code;
    struct kefir_codegen_target_ir_control_flow control_flow;
    struct kefir_codegen_target_ir_liveness liveness;
    struct kefir_codegen_target_ir_interference interference;
    struct kefir_codegen_target_ir_coalesce coalesce;
    struct kefir_codegen_target_ir_regalloc regalloc;
    struct kefir_codegen_target_ir_amd64_regalloc_class regalloc_class;
    struct kefir_codegen_target_ir_code_constructor_metadata target_ir_metadata;
} kefir_codegen_amd64_function_target_ir_t;

kefir_result_t kefir_codegen_amd64_function_target_ir_init(struct kefir_mem *, struct kefir_codegen_amd64_function_target_ir *, kefir_abi_amd64_variant_t);
kefir_result_t kefir_codegen_amd64_function_target_ir_free(struct kefir_mem *, struct kefir_codegen_amd64_function_target_ir *);

kefir_result_t kefir_codegen_amd64_function_target_ir_apply(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                          struct kefir_codegen_amd64_function_translator_state *,
                                          struct kefir_codegen_amd64_function_target_ir *,
                                          struct kefir_codegen_target_ir_code_constructor_metadata *,
                                          struct kefir_asmcmp_amd64 *,
                                            struct kefir_codegen_amd64_stack_frame *,
                                            const struct kefir_opt_code_debug_info *);

#endif
