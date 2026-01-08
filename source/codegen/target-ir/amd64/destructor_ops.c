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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/target-ir/amd64/destructor_ops.h"
#include "kefir/codegen/target-ir/amd64/code.h"
#include "kefir/codegen/target-ir/amd64/topological_scheduler.h"
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t bind_native_id(struct kefir_mem *mem, kefir_asmcmp_label_index_t label, kefir_codegen_target_ir_native_id_t native_id, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));
    
    kefir_result_t res = kefir_hashtree_insert(mem, &ops->constants, (kefir_hashtree_key_t) label, (kefir_hashtree_value_t) native_id);
    if (res == KEFIR_ALREADY_EXISTS) {
        REQUIRE_OK(res);
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

static kefir_result_t new_inline_asm(struct kefir_mem *mem, kefir_asmcmp_instruction_index_t insert_after_idx, kefir_asmcmp_inline_assembly_index_t inline_asm_idx, kefir_asmcmp_instruction_index_t *instr_idx, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));
    
    REQUIRE_OK(kefir_asmcmp_amd64_inline_assembly(mem, ops->code, insert_after_idx, inline_asm_idx, instr_idx));
    return KEFIR_OK;
}

static kefir_result_t materialize_attribute(struct kefir_mem *mem, kefir_asmcmp_instruction_index_t insert_after_idx, kefir_codegen_target_ir_native_id_t attribute, kefir_asmcmp_instruction_index_t *instr_idx, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));
    
    switch (attribute) {
        case KEFIR_TARGET_IR_AMD64_OPCODE(rexW):
            REQUIRE_OK(kefir_asmcmp_amd64_rexW(mem, ops->code, insert_after_idx, instr_idx));
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(data16):
            REQUIRE_OK(kefir_asmcmp_amd64_data16(mem, ops->code, insert_after_idx, instr_idx));
            break;

        case KEFIR_TARGET_IR_AMD64_OPCODE(lock):
            REQUIRE_OK(kefir_asmcmp_amd64_lock(mem, ops->code, insert_after_idx, instr_idx));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected amd64 target IR attribute");
    }
    return KEFIR_OK;
}

static kefir_result_t new_code_fragment(struct kefir_mem *mem, kefir_codegen_target_ir_metadata_code_ref_t code_ref, kefir_asmcmp_label_index_t begin_label, kefir_asmcmp_label_index_t end_label, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));

    REQUIRE_OK(kefir_asmcmp_code_map_add_fragment(mem, &ops->debug_code_map, (kefir_asmcmp_debug_info_code_reference_t) code_ref, begin_label, end_label));
    return KEFIR_OK;
}

static kefir_result_t new_value_fragment(struct kefir_mem *mem, kefir_codegen_target_ir_metadata_value_ref_t value_ref, kefir_asmcmp_debug_info_value_location_reference_t location_ref, kefir_asmcmp_label_index_t begin_label, kefir_asmcmp_label_index_t end_label, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_destructor_amd64_ops *, ops,
        payload);
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));

    if (kefir_opt_code_debug_info_is_active_ref(&ops->function->function->debug_info, value_ref)) {
        REQUIRE_OK(kefir_asmcmp_value_map_add_fragment(mem, &ops->debug_value_map, value_ref, location_ref, begin_label, end_label));
    }
    return KEFIR_OK;
}

static kefir_result_t schedule_code(struct kefir_mem *mem,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    struct kefir_codegen_target_ir_code_schedule *schedule,
    void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule"));
    UNUSED(payload);

    struct kefir_codegen_target_ir_code_scheduler scheduler;
    REQUIRE_OK(kefir_codegen_target_ir_amd64_topological_scheduler_init(control_flow, &scheduler));
    REQUIRE_OK(kefir_codegen_target_ir_code_schedule_build(mem, schedule, &scheduler));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_destructor_amd64_ops_init(const struct kefir_codegen_amd64_function *function, struct kefir_asmcmp_amd64 *code, struct kefir_codegen_target_ir_destructor_amd64_ops *ops) {
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen function"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp code"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid pointer to target IR destructor amd64 ops"));

    ops->function = function;
    ops->code = code;
    ops->ops.unreachable_opcode = KEFIR_ASMCMP_AMD64_OPCODE(ud2);
    ops->ops.noop_opcode = KEFIR_ASMCMP_AMD64_OPCODE(noop);
    ops->ops.bind_native_id = bind_native_id;
    ops->ops.new_inline_asm = new_inline_asm;
    ops->ops.materialize_attribute = materialize_attribute;
    ops->ops.new_code_fragment = new_code_fragment;
    ops->ops.new_value_fragment = new_value_fragment;
    ops->ops.schedule_code = schedule_code;
    ops->ops.payload = ops;

    REQUIRE_OK(kefir_hashtree_init(&ops->constants, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_asmcmp_debug_info_code_map_init(&ops->debug_code_map));
    REQUIRE_OK(kefir_asmcmp_debug_info_value_map_init(&ops->debug_value_map));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_destructor_amd64_ops_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_destructor_amd64_ops *ops) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid memory allocator"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected vaid target IR destructor amd64 ops"));

    REQUIRE_OK(kefir_asmcmp_debug_info_value_map_free(mem, &ops->debug_value_map));
    REQUIRE_OK(kefir_asmcmp_debug_info_code_map_free(mem, &ops->debug_code_map));
    REQUIRE_OK(kefir_hashtree_free(mem, &ops->constants));
    return KEFIR_OK;
}
