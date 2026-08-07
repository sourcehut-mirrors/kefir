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

#include "kefir/codegen/amd64/scheduler.h"
#include "kefir/codegen/amd64/util.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_function_schedule_instruction(
    kefir_opt_instruction_ref_t instr_ref,
    kefir_opt_code_topological_scheduler_instruction_dependency_callback_t dependency_callback,
    void *dependency_callback_payload, kefir_bool_t *schedule_instruction, void *payload) {
    REQUIRE(
        dependency_callback != NULL,
        KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction dependency scheduler callback"));
    REQUIRE(schedule_instruction != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction scheduler flag"));
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64_function_schedule_instruction_parameters *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer scheduler parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->func->generic.function->code, instr_ref, &instr));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
        return KEFIR_OK;
    }
    if ((instr->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE &&
         (instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_GREATER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER ||
          instr->operation.parameters.branch.comparison.operation ==
              KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_LESSER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_GREATER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER ||
          instr->operation.parameters.branch.comparison.operation ==
              KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_LESSER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER ||
          instr->operation.parameters.branch.comparison.operation ==
              KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL)) ||
        (instr->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE &&
         (instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_GREATER ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_GREATER ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL))) {
        const struct kefir_opt_instruction *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&param->func->generic.function->code, instr->operation.parameters.refs[1],
                                                  &arg2_instr));
        if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_CONST ||
            arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_CONST) {
            REQUIRE_OK(dependency_callback(instr->operation.parameters.refs[0], dependency_callback_payload));
            *schedule_instruction = true;
            return KEFIR_OK;
        }
    }
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE &&
        (instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_LESSER ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_LESSER ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL)) {
        const struct kefir_opt_instruction *arg1_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&param->func->generic.function->code, instr->operation.parameters.refs[0],
                                                  &arg1_instr));
        if (arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_CONST ||
            arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_CONST) {
            REQUIRE_OK(dependency_callback(instr->operation.parameters.refs[1], dependency_callback_payload));
            *schedule_instruction = true;
            return KEFIR_OK;
        }
    }
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
        const struct kefir_opt_call_node *call_node;
        REQUIRE_OK(kefir_opt_code_container_call(&param->func->generic.function->code,
                                                 instr->operation.parameters.function_call.call_ref, &call_node));

        kefir_bool_t tail_call_possible;
        REQUIRE_OK(
            kefir_codegen_amd64_tail_call_possible(param->mem, param->func, call_node->node_id, &tail_call_possible));

        if (tail_call_possible) {
            kefir_bool_t passthrough_aggregate_return = false;
            REQUIRE_OK(kefir_codegen_amd64_tail_call_return_aggregate_passthrough(param->func, call_node->node_id,
                                                                                  &passthrough_aggregate_return));
            if (passthrough_aggregate_return && param->func->stack_frame.return_space_vreg != KEFIR_ASMCMP_INDEX_NONE) {
                REQUIRE_OK(kefir_codegen_local_variable_allocator_mark_return_space(&param->func->generic.variable_allocator,
                                                                                    call_node->return_space));
            }
        }

        if (call_node->return_space != KEFIR_ID_NONE && !tail_call_possible) {
            REQUIRE_OK(dependency_callback(call_node->return_space, dependency_callback_payload));
        }

        if (instr->operation.parameters.function_call.indirect_ref != KEFIR_ID_NONE) {
            REQUIRE_OK(dependency_callback(instr->operation.parameters.function_call.indirect_ref,
                                           dependency_callback_payload));
        }
        for (kefir_size_t i = 0; i < call_node->argument_count; i++) {
            if (call_node->arguments[i] != KEFIR_ID_NONE) {
                REQUIRE_OK(dependency_callback(call_node->arguments[i], dependency_callback_payload));
            }
        }

        *schedule_instruction = true;
        return KEFIR_OK;
    }
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_RETURN && param->func->generic.function->ir_func->declaration->no_return) {
        *schedule_instruction = true;
        return KEFIR_OK;
    }

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_STORE ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_STORE ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_STORE ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_STORE ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_STORE ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_STORE ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE) {
        const struct kefir_opt_instruction *location_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(
            &param->func->generic.function->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
            &location_instr));
        if (location_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ADD) {
            const struct kefir_opt_instruction *location_arg1_instr, *location_arg2_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(
                &param->func->generic.function->code, location_instr->operation.parameters.refs[0], &location_arg1_instr));
            REQUIRE_OK(kefir_opt_code_container_instr(
                &param->func->generic.function->code, location_instr->operation.parameters.refs[1], &location_arg2_instr));

            if (location_arg2_instr->block_id == instr->block_id &&
                ((location_arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                  location_arg1_instr->operation.parameters.imm.integer >= KEFIR_INT32_MIN &&
                  location_arg1_instr->operation.parameters.imm.integer <= KEFIR_INT32_MAX) ||
                 (location_arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                  location_arg1_instr->operation.parameters.imm.uinteger <= KEFIR_INT32_MAX))) {
                REQUIRE_OK(dependency_callback(location_arg2_instr->id, dependency_callback_payload));
                REQUIRE_OK(dependency_callback(instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF],
                                               dependency_callback_payload));
                *schedule_instruction = true;
                return KEFIR_OK;
            } else if (location_arg1_instr->block_id == instr->block_id &&
                       ((location_arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                         location_arg2_instr->operation.parameters.imm.integer >= KEFIR_INT32_MIN &&
                         location_arg2_instr->operation.parameters.imm.integer <= KEFIR_INT32_MAX) ||
                        (location_arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                         location_arg2_instr->operation.parameters.imm.uinteger <= KEFIR_INT32_MAX))) {
                REQUIRE_OK(dependency_callback(location_arg1_instr->id, dependency_callback_payload));
                REQUIRE_OK(dependency_callback(instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF],
                                               dependency_callback_payload));
                *schedule_instruction = true;
                return KEFIR_OK;
            }
        }
    }

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_LOAD ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_LOAD ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_LOAD ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD) {
        const struct kefir_opt_instruction *location_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(
            &param->func->generic.function->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
            &location_instr));
        if (location_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ADD) {
            const struct kefir_opt_instruction *location_arg1_instr, *location_arg2_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(
                &param->func->generic.function->code, location_instr->operation.parameters.refs[0], &location_arg1_instr));
            REQUIRE_OK(kefir_opt_code_container_instr(
                &param->func->generic.function->code, location_instr->operation.parameters.refs[1], &location_arg2_instr));

            if (location_arg2_instr->block_id == instr->block_id &&
                ((location_arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                  location_arg1_instr->operation.parameters.imm.integer >= KEFIR_INT32_MIN &&
                  location_arg1_instr->operation.parameters.imm.integer <= KEFIR_INT32_MAX) ||
                 (location_arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                  location_arg1_instr->operation.parameters.imm.uinteger <= KEFIR_INT32_MAX))) {
                REQUIRE_OK(dependency_callback(location_arg2_instr->id, dependency_callback_payload));
                *schedule_instruction = true;
                return KEFIR_OK;
            } else if (location_arg1_instr->block_id == instr->block_id &&
                       ((location_arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                         location_arg2_instr->operation.parameters.imm.integer >= KEFIR_INT32_MIN &&
                         location_arg2_instr->operation.parameters.imm.integer <= KEFIR_INT32_MAX) ||
                        (location_arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                         location_arg2_instr->operation.parameters.imm.uinteger <= KEFIR_INT32_MAX))) {
                REQUIRE_OK(dependency_callback(location_arg1_instr->id, dependency_callback_payload));
                *schedule_instruction = true;
                return KEFIR_OK;
            }
        }
    }

    REQUIRE_OK(kefir_opt_instruction_extract_inputs(&param->func->generic.function->code, instr, true, dependency_callback,
                                                    dependency_callback_payload));
    *schedule_instruction = true;
    return KEFIR_OK;
}
