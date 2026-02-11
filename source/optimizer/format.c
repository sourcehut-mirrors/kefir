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

#include "kefir/optimizer/format.h"
#include "kefir/optimizer/schedule.h"
#include "kefir/optimizer/topological_schedule.h"
#include "kefir/optimizer/module_liveness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t id_format(struct kefir_json_output *json, kefir_id_t id) {
    if (id == KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_json_output_null(json));
    } else {
        REQUIRE_OK(kefir_json_output_uinteger(json, id));
    }
    return KEFIR_OK;
}

static kefir_result_t format_condition_variamt(struct kefir_json_output *json,
                                               kefir_opt_branch_condition_variant_t variant) {
    switch (variant) {
        case KEFIR_OPT_BRANCH_CONDITION_8BIT:
            REQUIRE_OK(kefir_json_output_string(json, "8bit"));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT:
            REQUIRE_OK(kefir_json_output_string(json, "inverted_8bit"));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_16BIT:
            REQUIRE_OK(kefir_json_output_string(json, "16bit"));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT:
            REQUIRE_OK(kefir_json_output_string(json, "inverted_16bit"));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_32BIT:
            REQUIRE_OK(kefir_json_output_string(json, "32bit"));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT:
            REQUIRE_OK(kefir_json_output_string(json, "inverted_32bit"));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_64BIT:
            REQUIRE_OK(kefir_json_output_string(json, "64bit"));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT:
            REQUIRE_OK(kefir_json_output_string(json, "inverted_64bit"));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t format_operation_branch(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                              const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
            REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
            REQUIRE_OK(kefir_json_output_object_key(json, "alternative_block"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.alternative_block));
            REQUIRE_OK(kefir_json_output_object_key(json, "condition"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.condition_ref));
            REQUIRE_OK(kefir_json_output_object_key(json, "condition_variant"));
            REQUIRE_OK(format_condition_variamt(json, oper->parameters.branch.condition_variant));
            break;

        case KEFIR_OPT_OPCODE_IJUMP:
            REQUIRE_OK(kefir_json_output_object_key(json, "target_ref"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "gate_block"));
            REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer operation code");
    }
    return KEFIR_OK;
}

static kefir_result_t format_comparison(struct kefir_json_output *json, kefir_opt_comparison_operation_t comparison) {
    switch (comparison) {
        case KEFIR_OPT_COMPARISON_INT8_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int8_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int8_not_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "int8_greater"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int8_greater_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER:
            REQUIRE_OK(kefir_json_output_string(json, "int8_lesser"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int8_lesser_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE:
            REQUIRE_OK(kefir_json_output_string(json, "int8_above"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int8_above_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW:
            REQUIRE_OK(kefir_json_output_string(json, "int8_below"));
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int8_below_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int16_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int16_not_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "int16_greater"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int16_greater_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_LESSER:
            REQUIRE_OK(kefir_json_output_string(json, "int16_lesser"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int16_lesser_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_ABOVE:
            REQUIRE_OK(kefir_json_output_string(json, "int16_above"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int16_above_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_BELOW:
            REQUIRE_OK(kefir_json_output_string(json, "int16_below"));
            break;

        case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int16_below_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int32_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int32_not_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "int32_greater"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int32_greater_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_LESSER:
            REQUIRE_OK(kefir_json_output_string(json, "int32_lesser"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int32_lesser_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_ABOVE:
            REQUIRE_OK(kefir_json_output_string(json, "int32_above"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int32_above_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_BELOW:
            REQUIRE_OK(kefir_json_output_string(json, "int32_below"));
            break;

        case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int32_below_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int64_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int64_not_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "int64_greater"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int64_greater_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_LESSER:
            REQUIRE_OK(kefir_json_output_string(json, "int64_lesser"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int64_lesser_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_ABOVE:
            REQUIRE_OK(kefir_json_output_string(json, "int64_above"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int64_above_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_BELOW:
            REQUIRE_OK(kefir_json_output_string(json, "int64_below"));
            break;

        case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_json_output_string(json, "int64_below_or_equals"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float32_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float32_not_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "float32_greater"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float32_greater_or_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER:
            REQUIRE_OK(kefir_json_output_string(json, "float32_lesser"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float32_lesser_or_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "float32_not_greater"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float32_not_greater_or_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER:
            REQUIRE_OK(kefir_json_output_string(json, "float32_not_lesser"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float32_not_lesser_or_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float64_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float64_not_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "float64_greater"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float64_greater_or_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER:
            REQUIRE_OK(kefir_json_output_string(json, "float64_lesser"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float64_lesser_or_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "float64_not_greater"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float64_not_greater_or_equal"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER:
            REQUIRE_OK(kefir_json_output_string(json, "float64_not_lesser"));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "float64_not_lesser_or_equal"));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected comparison operator");
    }

    return KEFIR_OK;
}

static kefir_result_t format_operation_branch_compare(struct kefir_json_output *json,
                                                      const struct kefir_opt_module *module,
                                                      const struct kefir_opt_code_container *code,
                                                      const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "comparison"));
    REQUIRE_OK(format_comparison(json, oper->parameters.branch.comparison.operation));
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.target_block));
    REQUIRE_OK(kefir_json_output_object_key(json, "alternative_block"));
    REQUIRE_OK(id_format(json, oper->parameters.branch.alternative_block));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref1(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                            const struct kefir_opt_code_container *code,
                                            const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    return KEFIR_OK;
}

static kefir_result_t format_operation_bitint_ref1(struct kefir_json_output *json,
                                                   const struct kefir_opt_module *module,
                                                   const struct kefir_opt_code_container *code,
                                                   const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "bitwidth"));
    REQUIRE_OK(id_format(json, oper->parameters.bitwidth));
    return KEFIR_OK;
}

static kefir_result_t format_operation_bitint_ref2(struct kefir_json_output *json,
                                                   const struct kefir_opt_module *module,
                                                   const struct kefir_opt_code_container *code,
                                                   const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "bitwidth"));
    REQUIRE_OK(id_format(json, oper->parameters.bitwidth));
    return KEFIR_OK;
}
static kefir_result_t format_operation_bitint2_ref1(struct kefir_json_output *json,
                                                    const struct kefir_opt_module *module,
                                                    const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "bitwidth"));
    REQUIRE_OK(id_format(json, oper->parameters.bitwidth));
    REQUIRE_OK(kefir_json_output_object_key(json, "src_bitwidth"));
    REQUIRE_OK(id_format(json, oper->parameters.src_bitwidth));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref2(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                            const struct kefir_opt_code_container *code,
                                            const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref3_cond(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                 const struct kefir_opt_code_container *code,
                                                 const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
    REQUIRE_OK(format_condition_variamt(json, oper->parameters.condition_variant));
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref4_compare(struct kefir_json_output *json,
                                                    const struct kefir_opt_module *module,
                                                    const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "comparison"));
    REQUIRE_OK(format_comparison(json, oper->parameters.comparison));
    REQUIRE_OK(kefir_json_output_object_key(json, "comparison_args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[3]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_load_mem(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "location"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "flags"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "load_extension"));
    switch (oper->parameters.memory_access.flags.load_extension) {
        case KEFIR_OPT_MEMORY_LOAD_NOEXTEND:
            REQUIRE_OK(kefir_json_output_string(json, "noextend"));
            break;

        case KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND:
            REQUIRE_OK(kefir_json_output_string(json, "zero_extend"));
            break;

        case KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND:
            REQUIRE_OK(kefir_json_output_string(json, "sign_extend"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.memory_access.flags.volatile_access));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_bitint_load(struct kefir_json_output *json,
                                                   const struct kefir_opt_module *module,
                                                   const struct kefir_opt_code_container *code,
                                                   const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "location"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "bitwidth"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitwidth));
    REQUIRE_OK(kefir_json_output_object_key(json, "flags"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "load_extension"));
    switch (oper->parameters.bitint_memflags.load_extension) {
        case KEFIR_OPT_MEMORY_LOAD_NOEXTEND:
            REQUIRE_OK(kefir_json_output_string(json, "noextend"));
            break;

        case KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND:
            REQUIRE_OK(kefir_json_output_string(json, "zero_extend"));
            break;

        case KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND:
            REQUIRE_OK(kefir_json_output_string(json, "sign_extend"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.bitint_memflags.volatile_access));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_bitint_store(struct kefir_json_output *json,
                                                    const struct kefir_opt_module *module,
                                                    const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "location"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "bitwidth"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitwidth));
    REQUIRE_OK(kefir_json_output_object_key(json, "flags"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.bitint_memflags.volatile_access));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_bitint_atomic(struct kefir_json_output *json,
                                                     const struct kefir_opt_module *module,
                                                     const struct kefir_opt_code_container *code,
                                                     const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "location"));
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_LOAD:
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            break;

        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_STORE:
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            break;

        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_COMPARE_EXCHANGE:
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "compare_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            REQUIRE_OK(kefir_json_output_object_key(json, "new_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer instruction opcode");
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "bitwidth"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitwidth));
    REQUIRE_OK(kefir_json_output_object_key(json, "flags"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    if (oper->opcode == KEFIR_OPT_OPCODE_BITINT_ATOMIC_LOAD) {
        REQUIRE_OK(kefir_json_output_object_key(json, "load_extension"));
        switch (oper->parameters.bitint_memflags.load_extension) {
            case KEFIR_OPT_MEMORY_LOAD_NOEXTEND:
                REQUIRE_OK(kefir_json_output_string(json, "noextend"));
                break;

            case KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND:
                REQUIRE_OK(kefir_json_output_string(json, "zero_extend"));
                break;

            case KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND:
                REQUIRE_OK(kefir_json_output_string(json, "sign_extend"));
                break;
        }
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.bitint_memflags.volatile_access));
    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "atomic_model"));
    switch (oper->parameters.bitint_atomic_memorder) {
        case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:
            REQUIRE_OK(kefir_json_output_string(json, "seq_cst"));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic model");
    }
    return KEFIR_OK;
}

static kefir_result_t format_operation_bitint_bitfield(struct kefir_json_output *json,
                                                       const struct kefir_opt_module *module,
                                                       const struct kefir_opt_code_container *code,
                                                       const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    if (oper->opcode == KEFIR_OPT_OPCODE_BITINT_INSERT) {
        REQUIRE_OK(kefir_json_output_object_key(json, "args"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
        REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
        REQUIRE_OK(kefir_json_output_array_end(json));
    } else {
        REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
        REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "bitwidth"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitwidth));
    REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitint_bitfield.offset));
    REQUIRE_OK(kefir_json_output_object_key(json, "length"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitint_bitfield.length));
    return KEFIR_OK;
}

static kefir_result_t format_operation_store_mem(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                 const struct kefir_opt_code_container *code,
                                                 const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "location"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "flags"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.memory_access.flags.volatile_access));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_tmpobj(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                              const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "size"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.tmp_object.size));
    REQUIRE_OK(kefir_json_output_object_key(json, "alignment"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.tmp_object.alignment));
    return KEFIR_OK;
}

static kefir_result_t format_operation_bitfield(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "base"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_BITFIELD_BASE_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_BITFIELD_VALUE_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitfield.offset));
    REQUIRE_OK(kefir_json_output_object_key(json, "length"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.bitfield.length));
    return KEFIR_OK;
}

static kefir_result_t format_operation_typed_ref2(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                  const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "target"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "source"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.type.type_index));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_compare_ref2(struct kefir_json_output *json,
                                                    const struct kefir_opt_module *module,
                                                    const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "comparison"));
    REQUIRE_OK(format_comparison(json, oper->parameters.comparison));
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref_offset(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                  const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "ref"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
    REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.offset));
    return KEFIR_OK;
}

static kefir_result_t format_operation_ref_index2(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                  const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "ref"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "indices"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.index_pair[0]));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.index_pair[1]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_overflow_arith(struct kefir_json_output *json,
                                                      const struct kefir_opt_module *module,
                                                      const struct kefir_opt_code_container *code,
                                                      const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "args"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
    REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.type.type_index));
    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "signedness"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(kefir_json_output_boolean(json, (oper->parameters.overflow_arith.signedness & 1) == 1));
    REQUIRE_OK(kefir_json_output_boolean(json, ((oper->parameters.overflow_arith.signedness >> 1) & 1) == 1));
    REQUIRE_OK(kefir_json_output_boolean(json, ((oper->parameters.overflow_arith.signedness >> 2) & 1) == 1));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_stack_alloc(struct kefir_json_output *json,
                                                   const struct kefir_opt_module *module,
                                                   const struct kefir_opt_code_container *code,
                                                   const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "size_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_STACK_ALLOCATION_SIZE_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "alignment_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[KEFIR_OPT_STACK_ALLOCATION_ALIGNMENT_REF]));
    REQUIRE_OK(kefir_json_output_object_key(json, "within_scope"));
    REQUIRE_OK(kefir_json_output_boolean(json, oper->parameters.stack_allocation.within_scope));
    return KEFIR_OK;
}

static kefir_result_t format_operation_call_ref(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_operation *oper) {
    UNUSED(module);
    const struct kefir_opt_call_node *call;
    REQUIRE_OK(kefir_opt_code_container_call(code, oper->parameters.function_call.call_ref, &call));
    REQUIRE_OK(kefir_json_output_object_key(json, "function_declaration_id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, call->function_declaration_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "arguments"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    for (kefir_size_t i = 0; i < call->argument_count; i++) {
        REQUIRE_OK(id_format(json, call->arguments[i]));
    }

    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "return_space"));
    REQUIRE_OK(id_format(json, call->return_space));
    REQUIRE_OK(kefir_json_output_object_key(json, "indirect_ref"));
    REQUIRE_OK(id_format(json, oper->parameters.function_call.indirect_ref));
    return KEFIR_OK;
}

static kefir_result_t format_operation_index(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                             const struct kefir_opt_code_container *code,
                                             const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.index));
    return KEFIR_OK;
}

static kefir_result_t format_operation_variable(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_GET_GLOBAL:
        case KEFIR_OPT_OPCODE_GET_THREAD_LOCAL:
            REQUIRE_OK(kefir_json_output_object_key(json, "global_ref"));
            REQUIRE_OK(id_format(json, oper->parameters.variable.global_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer instruction opcode");
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
    REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.variable.offset));
    return KEFIR_OK;
}

static kefir_result_t format_operation_localvar(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                const struct kefir_opt_code_container *code,
                                                const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "scope"));
    REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "type_id"));
    REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.type.type_index));
    return KEFIR_OK;
}

static kefir_result_t format_operation_phi_ref(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                               const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_operation *oper) {
    UNUSED(module);
    const struct kefir_opt_phi_node *phi;
    REQUIRE_OK(kefir_opt_code_container_phi(code, oper->parameters.phi_ref, &phi));
    REQUIRE_OK(kefir_json_output_object_key(json, "links"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi->links, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, node->key);
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->value);
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "block_id"));
        REQUIRE_OK(kefir_json_output_uinteger(json, block_id));
        REQUIRE_OK(kefir_json_output_object_key(json, "instr_ref"));
        REQUIRE_OK(kefir_json_output_uinteger(json, instr_ref));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }

    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_inline_asm(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                  const struct kefir_opt_code_container *code,
                                                  const struct kefir_opt_operation *oper) {
    UNUSED(module);
    const struct kefir_opt_inline_assembly_node *inline_asm;
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, oper->parameters.inline_asm_ref, &inline_asm));

    REQUIRE_OK(kefir_json_output_object_key(json, "inline_asm_id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, inline_asm->inline_asm_id));
    REQUIRE_OK(kefir_json_output_object_key(json, "parameters"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    for (kefir_size_t i = 0; i < inline_asm->parameter_count; i++) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "load_store"));
        REQUIRE_OK(id_format(json, inline_asm->parameters[i].load_store_ref));
        REQUIRE_OK(kefir_json_output_object_key(json, "read"));
        REQUIRE_OK(id_format(json, inline_asm->parameters[i].read_ref));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }

    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "default_jump_target"));
    REQUIRE_OK(id_format(json, inline_asm->default_jump_target));
    REQUIRE_OK(kefir_json_output_object_key(json, "jump_targets"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_id_t, target_id, node->key);
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "id"));
        REQUIRE_OK(id_format(json, target_id));
        REQUIRE_OK(kefir_json_output_object_key(json, "target_block"));
        REQUIRE_OK(id_format(json, target_block));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_operation_atomic_op(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                 const struct kefir_opt_code_container *code,
                                                 const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD8:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD16:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD32:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD64:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT32:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT64:
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_DECIMAL32_ATOMIC_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL64_ATOMIC_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL128_ATOMIC_LOAD:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_STORE8:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE16:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE32:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE64:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT32:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT64:
        case KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_DECIMAL32_ATOMIC_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL64_ATOMIC_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL128_ATOMIC_STORE:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_FROM:
        case KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_TO:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "source"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            REQUIRE_OK(kefir_json_output_object_key(json, "type_id"));
            REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
            REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
            REQUIRE_OK(id_format(json, oper->parameters.type.type_index));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG64:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT32:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT64:
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_DECIMAL32_ATOMIC_CMPXCHG:
        case KEFIR_OPT_OPCODE_DECIMAL64_ATOMIC_CMPXCHG:
        case KEFIR_OPT_OPCODE_DECIMAL128_ATOMIC_CMPXCHG:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "compare_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            REQUIRE_OK(kefir_json_output_object_key(json, "new_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_MEMORY:
            REQUIRE_OK(kefir_json_output_object_key(json, "location"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[0]));
            REQUIRE_OK(kefir_json_output_object_key(json, "compare_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[1]));
            REQUIRE_OK(kefir_json_output_object_key(json, "new_value"));
            REQUIRE_OK(id_format(json, oper->parameters.refs[2]));
            REQUIRE_OK(kefir_json_output_object_key(json, "type_id"));
            REQUIRE_OK(id_format(json, oper->parameters.type.type_id));
            REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
            REQUIRE_OK(id_format(json, oper->parameters.type.type_index));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer operation code");
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "atomic_model"));
    switch (oper->parameters.atomic_op.model) {
        case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:
            REQUIRE_OK(kefir_json_output_string(json, "seq_cst"));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic model");
    }
    return KEFIR_OK;
}

static kefir_result_t format_operation_immediate(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                                 const struct kefir_opt_code_container *code,
                                                 const struct kefir_opt_operation *oper) {
    UNUSED(code);
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    switch (oper->opcode) {
        case KEFIR_OPT_OPCODE_INT_CONST:
            REQUIRE_OK(kefir_json_output_integer(json, oper->parameters.imm.integer));
            break;

        case KEFIR_OPT_OPCODE_UINT_CONST:
            REQUIRE_OK(kefir_json_output_uinteger(json, oper->parameters.imm.uinteger));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_CONST:
            REQUIRE_OK(kefir_json_output_float(json, oper->parameters.imm.float32));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_CONST:
            REQUIRE_OK(kefir_json_output_float(json, oper->parameters.imm.float64));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
            REQUIRE_OK(kefir_json_output_long_double(json, oper->parameters.imm.long_double));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_CONST: {
            char buf[128];
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            kefir_dfp_decimal32_format(buf, sizeof(buf), oper->parameters.imm.decimal32);
            REQUIRE_OK(kefir_json_output_string(json, buf));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL64_CONST: {
            char buf[128];
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            kefir_dfp_decimal64_format(buf, sizeof(buf), oper->parameters.imm.decimal64);
            REQUIRE_OK(kefir_json_output_string(json, buf));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL128_CONST: {
            char buf[128];
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            kefir_dfp_decimal128_format(buf, sizeof(buf), oper->parameters.imm.decimal128);
            REQUIRE_OK(kefir_json_output_string(json, buf));
        } break;

        case KEFIR_OPT_OPCODE_STRING_REF:
            REQUIRE_OK(id_format(json, oper->parameters.imm.string_ref));
            break;

        case KEFIR_OPT_OPCODE_BLOCK_LABEL:
            REQUIRE_OK(id_format(json, oper->parameters.imm.block_ref));
            break;

        case KEFIR_IR_OPCODE_BITINT_SIGNED_CONST:
        case KEFIR_IR_OPCODE_BITINT_UNSIGNED_CONST: {
            const struct kefir_bigint *bigint;
            REQUIRE_OK(
                kefir_ir_module_get_bigint(module->ir_module, (kefir_id_t) oper->parameters.imm.bitint_ref, &bigint));
            REQUIRE_OK(kefir_json_output_bigint(json, bigint));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer operation code");
    }
    return KEFIR_OK;
}

static kefir_result_t format_operation_none(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                            const struct kefir_opt_code_container *code,
                                            const struct kefir_opt_operation *oper) {
    UNUSED(module);
    UNUSED(code);
    UNUSED(json);
    UNUSED(oper);
    return KEFIR_OK;
}

static kefir_result_t instr_format(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                   const struct kefir_opt_code_container *code,
                                   const struct kefir_opt_instruction *instr,
                                   const struct kefir_opt_code_memssa *memssa,
                                   const struct kefir_opt_code_debug_info *debug_info) {
    UNUSED(memssa);
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));

    switch (instr->operation.opcode) {
#define OPCODE(_id, _name, _class)                                                    \
    case KEFIR_OPT_OPCODE_##_id:                                                      \
        REQUIRE_OK(kefir_json_output_string(json, (_name)));                          \
        REQUIRE_OK(format_operation_##_class(json, module, code, &instr->operation)); \
        break;

        KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE, )
#undef OPCODE
    }

    REQUIRE_OK(kefir_json_output_object_key(json, "control_next"));
    REQUIRE_OK(id_format(json, instr->control_flow.next));

    if (debug_info != NULL) {
        REQUIRE_OK(kefir_json_output_object_key(json, "ir_instruction"));
        kefir_opt_code_debug_info_code_ref_t instruction_location;
        REQUIRE_OK(kefir_opt_code_debug_info_instruction_code_reference(debug_info, instr->id, &instruction_location));
        if (instruction_location != KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE) {
            REQUIRE_OK(kefir_json_output_uinteger(json, instruction_location));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
    }

    if (memssa != NULL) {
        kefir_opt_code_memssa_node_ref_t node_ref;
        kefir_result_t res = kefir_opt_code_memssa_instruction_binding(memssa, instr->id, &node_ref);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            REQUIRE_OK(kefir_json_output_object_key(json, "memssa_node_ref"));
            REQUIRE_OK(id_format(json, node_ref));
        }
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t code_block_format(
    struct kefir_json_output *json, const struct kefir_opt_module *module, const struct kefir_opt_code_container *code,
    const struct kefir_opt_code_block *block, const struct kefir_opt_code_control_flow *control_flow,
    const struct kefir_opt_code_liveness *liveness, const struct kefir_opt_code_memssa *memssa,
    const struct kefir_opt_code_debug_info *debug_info, const struct kefir_opt_code_schedule *schedule) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, block->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "control_flow"));
    REQUIRE_OK(id_format(json, block->control_flow.head));

    kefir_result_t res;
    REQUIRE_OK(kefir_json_output_object_key(json, "code"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    kefir_opt_instruction_ref_t instr_ref;
    const struct kefir_opt_instruction *instr = NULL;
    if (schedule == NULL || block->id == code->gate_block) {
        for (res = kefir_opt_code_block_instr_head(code, block->id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {

            REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
            REQUIRE_OK(instr_format(json, module, code, instr, memssa, debug_info));
        }
        REQUIRE_OK(res);
    } else {
        struct kefir_opt_code_block_schedule_iterator iter;
        for (res = kefir_opt_code_block_schedule_iter(schedule, block->id, &iter);
             res == KEFIR_OK && iter.instr_ref != KEFIR_ID_NONE; res = kefir_opt_code_block_schedule_next(&iter)) {
            REQUIRE_OK(kefir_opt_code_container_instr(code, iter.instr_ref, &instr));
            REQUIRE_OK(instr_format(json, module, code, instr, memssa, debug_info));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "public_labels"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&block->public_labels, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, alternative_label, iter.entry);
        REQUIRE_OK(kefir_json_output_string(json, alternative_label));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "properties"));
    if (control_flow != NULL && liveness != NULL) {
        const struct kefir_opt_code_control_flow_block *control_flow_block = &control_flow->blocks[block->id];

        REQUIRE_OK(kefir_json_output_object_begin(json));

        REQUIRE_OK(kefir_json_output_object_key(json, "successors"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t entry;
        for (res = kefir_hashset_iter(&control_flow_block->successors, &iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, entry);
            REQUIRE_OK(kefir_json_output_uinteger(json, block_id));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_key(json, "predecessors"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (res = kefir_hashset_iter(&control_flow_block->predecessors, &iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, entry);
            REQUIRE_OK(kefir_json_output_uinteger(json, block_id));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_key(json, "immediate_dominator"));
        REQUIRE_OK(id_format(json, control_flow_block->immediate_dominator));

        REQUIRE_OK(kefir_json_output_object_key(json, "alive_instructions"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (res = kefir_hashset_iter(&liveness->blocks[block->id].alive_instr, &iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &entry)) {
            REQUIRE_OK(id_format(json, (kefir_id_t) entry));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_end(json));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_blocks(struct kefir_json_output *json, const struct kefir_opt_module *module,
                                    const struct kefir_opt_code_container *code,
                                    const struct kefir_opt_code_control_flow *control_flow,
                                    const struct kefir_opt_code_liveness *liveness,
                                    const struct kefir_opt_code_memssa *memssa,
                                    struct kefir_opt_code_schedule *schedule,
                                    const struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE_OK(kefir_json_output_object_key(json, "blocks"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    if (schedule == NULL) {
        struct kefir_opt_code_container_iterator iter;
        for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(code, &iter); block != NULL;
             block = kefir_opt_code_container_next(&iter)) {
            REQUIRE_OK(
                code_block_format(json, module, code, block, control_flow, liveness, memssa, debug_info, schedule));
        }
    } else {
        const kefir_size_t num_of_blocks = kefir_opt_code_schedule_num_of_blocks(schedule);
        for (kefir_size_t i = 0; i < num_of_blocks; i++) {
            kefir_opt_block_id_t block_id;
            REQUIRE_OK(kefir_opt_code_schedule_block_by_index(schedule, i, &block_id));

            const struct kefir_opt_code_block *block;
            REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));
            REQUIRE_OK(
                code_block_format(json, module, code, block, control_flow, liveness, memssa, debug_info, schedule));
        }

        if (code->gate_block != KEFIR_ID_NONE) {
            const struct kefir_opt_code_block *block;
            REQUIRE_OK(kefir_opt_code_container_block(code, code->gate_block, &block));
            REQUIRE_OK(
                code_block_format(json, module, code, block, control_flow, liveness, memssa, debug_info, schedule));
        }
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_memssa(struct kefir_mem *mem, struct kefir_json_output *json,
                                    const struct kefir_opt_code_memssa *memssa, struct kefir_list *queue,
                                    struct kefir_hashset *visited) {
    REQUIRE_OK(kefir_json_output_object_key(json, "memssa"));
    REQUIRE_OK(kefir_json_output_array_begin(json));

    REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) memssa->root_ref));
    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL; iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_opt_code_memssa_node_ref_t, node_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));
        if (kefir_hashset_has(visited, (kefir_hashset_key_t) node_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(mem, visited, (kefir_hashset_key_t) node_ref));

        kefir_result_t res;
        struct kefir_opt_code_memssa_use_iterator use_iter;
        kefir_opt_code_memssa_node_ref_t use_node_ref;
        for (res = kefir_opt_code_memssa_use_iter(memssa, &use_iter, node_ref, &use_node_ref); res == KEFIR_OK;
             res = kefir_opt_code_memssa_use_next(&use_iter, &use_node_ref)) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) use_node_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        const struct kefir_opt_code_memssa_node *node;
        REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "node_ref"));
        REQUIRE_OK(id_format(json, node_ref));
        switch (node->type) {
            case KEFIR_OPT_CODE_MEMSSA_ROOT_NODE:
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_string(json, "root"));
                break;

            case KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE:
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_string(json, "produce"));
                REQUIRE_OK(kefir_json_output_object_key(json, "predecessor_ref"));
                REQUIRE_OK(id_format(json, node->predecessor_ref));
                break;

            case KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE:
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_string(json, "produce"));
                REQUIRE_OK(kefir_json_output_object_key(json, "predecessor_ref"));
                REQUIRE_OK(id_format(json, node->predecessor_ref));
                break;

            case KEFIR_OPT_CODE_MEMSSA_JOIN_NODE: {
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_string(json, "join"));
                REQUIRE_OK(kefir_json_output_object_key(json, "nodes"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                for (kefir_size_t i = 0; i < node->join.input_length; i++) {
                    REQUIRE_OK(id_format(json, node->join.inputs[i]));
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
            } break;

            case KEFIR_OPT_CODE_MEMSSA_PHI_NODE: {
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_string(json, "phi"));
                REQUIRE_OK(kefir_json_output_object_key(json, "links"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                for (kefir_size_t i = 0; i < node->phi.link_count; i++) {
                    REQUIRE_OK(kefir_json_output_object_begin(json));
                    REQUIRE_OK(kefir_json_output_object_key(json, "block_ref"));
                    REQUIRE_OK(id_format(json, node->phi.links[i].block_ref));
                    REQUIRE_OK(kefir_json_output_object_key(json, "node_ref"));
                    REQUIRE_OK(id_format(json, node->phi.links[i].node_ref));
                    REQUIRE_OK(kefir_json_output_object_end(json));
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
            } break;
        }
        REQUIRE_OK(kefir_json_output_object_end(json));
    }

    REQUIRE_OK(kefir_json_output_array_end(json));

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_format(struct kefir_mem *mem, struct kefir_json_output *json,
                                     const struct kefir_opt_module *module, const struct kefir_opt_code_container *code,
                                     const struct kefir_opt_code_control_flow *control_flow,
                                     const struct kefir_opt_code_liveness *liveness,
                                     const struct kefir_opt_code_memssa *memssa,
                                     const struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_json_output_object_begin(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "entry_point"));
    REQUIRE_OK(id_format(json, code->entry_point));
    if (code->gate_block != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_json_output_object_key(json, "gate_block"));
        REQUIRE_OK(id_format(json, code->gate_block));
    }

    struct kefir_opt_code_topological_scheduler topological_scheduler;
    REQUIRE_OK(kefir_opt_code_topological_scheduler_init(
        &topological_scheduler, kefir_opt_code_topological_scheduler_default_schedule, (void *) code));
    struct kefir_opt_code_schedule schedule;
    kefir_result_t res = KEFIR_OK;
    if (mem != NULL && control_flow != NULL && liveness != NULL) {
        REQUIRE_OK(kefir_opt_code_schedule_init(&schedule));
        res =
            kefir_opt_code_schedule_run(mem, &schedule, code, control_flow, liveness, &topological_scheduler.scheduler);
    }
    REQUIRE_CHAIN(
        &res, format_blocks(json, module, code, control_flow, liveness, memssa,
                            mem != NULL && control_flow != NULL && liveness != NULL ? &schedule : NULL, debug_info));
    if (mem != NULL && control_flow != NULL && liveness != NULL) {
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_opt_code_schedule_free(mem, &schedule);
            return res;
        });
        REQUIRE_OK(kefir_opt_code_schedule_free(mem, &schedule));
    } else {
        REQUIRE_OK(res);
    }

    if (debug_info != NULL) {
        REQUIRE_OK(kefir_json_output_object_key(json, "debug"));
        REQUIRE_OK(kefir_json_output_object_begin(json));

        REQUIRE_OK(kefir_json_output_object_key(json, "local_variables"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        struct kefir_opt_code_debug_info_local_variable_iterator local_variable_iter;
        const struct kefir_opt_code_debug_info_local_variable *local_variable;
        kefir_result_t res;
        for (res = kefir_opt_code_debug_info_local_variable_iter(debug_info, &local_variable_iter, &local_variable);
             res == KEFIR_OK;
             res = kefir_opt_code_debug_info_local_variable_next(&local_variable_iter, &local_variable)) {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "variable_id"));
            REQUIRE_OK(kefir_json_output_uinteger(json, local_variable->variable_ref));
            REQUIRE_OK(kefir_json_output_object_key(json, "allocations"));
            REQUIRE_OK(kefir_json_output_array_begin(json));
            struct kefir_hashset_iterator ref_iter;
            kefir_hashset_key_t ref_key;
            for (res = kefir_hashset_iter(&local_variable->allocations, &ref_iter, &ref_key); res == KEFIR_OK;
                 res = kefir_hashset_next(&ref_iter, &ref_key)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, ref_key);
                REQUIRE_OK(id_format(json, instr_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            REQUIRE_OK(kefir_json_output_array_end(json));
            REQUIRE_OK(kefir_json_output_object_end(json));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_key(json, "allocation_placement"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        struct kefir_opt_code_debug_info_allocation_placement_iterator placement_iter;
        const struct kefir_opt_code_debug_info_allocation_placement *placement;
        for (res = kefir_opt_code_debug_info_allocation_placement_iter(debug_info, &placement_iter, &placement);
             res == KEFIR_OK; res = kefir_opt_code_debug_info_allocation_placement_next(&placement_iter, &placement)) {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "allocation_ref"));
            REQUIRE_OK(kefir_json_output_uinteger(json, placement->allocation_ref));
            REQUIRE_OK(kefir_json_output_object_key(json, "placement"));
            REQUIRE_OK(kefir_json_output_array_begin(json));
            struct kefir_hashset_iterator ref_iter;
            kefir_hashset_key_t ref_key;
            for (res = kefir_hashset_iter(&placement->placement, &ref_iter, &ref_key); res == KEFIR_OK;
                 res = kefir_hashset_next(&ref_iter, &ref_key)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, ref_key);
                REQUIRE_OK(id_format(json, instr_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            REQUIRE_OK(kefir_json_output_array_end(json));
            REQUIRE_OK(kefir_json_output_object_end(json));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_json_output_array_end(json));

        REQUIRE_OK(kefir_json_output_object_end(json));
    }

    if (memssa != NULL && memssa->root_ref != KEFIR_ID_NONE) {
        struct kefir_list queue;
        struct kefir_hashset visited;
        REQUIRE_OK(kefir_list_init(&queue));
        REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
        res = format_memssa(mem, json, memssa, &queue, &visited);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_list_free(mem, &queue);
            kefir_hashset_free(mem, &visited);
            return res;
        });
        res = kefir_list_free(mem, &queue);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashset_free(mem, &visited);
            return res;
        });
        REQUIRE_OK(kefir_hashset_free(mem, &visited));
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_function(struct kefir_mem *mem, struct kefir_json_output *json,
                                      const struct kefir_opt_module *module, const struct kefir_opt_function *function,
                                      const struct kefir_opt_code_control_flow *control_flow,
                                      const struct kefir_opt_code_liveness *liveness,
                                      const struct kefir_opt_code_memssa *memssa, kefir_bool_t debug_info) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "id"));
    REQUIRE_OK(kefir_json_output_uinteger(json, function->ir_func->declaration->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "name"));
    if (function->ir_func->declaration->name != NULL) {
        REQUIRE_OK(kefir_json_output_string(json, function->ir_func->declaration->name));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "code"));
    REQUIRE_OK(kefir_opt_code_format(mem, json, module, &function->code, control_flow, liveness, memssa,
                                     debug_info ? &function->debug_info : NULL));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_format(struct kefir_mem *mem, struct kefir_json_output *json,
                                       const struct kefir_opt_module *module, kefir_bool_t analyze,
                                       kefir_bool_t debug_info) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));

    struct kefir_opt_module_liveness module_liveness;
    REQUIRE_OK(kefir_opt_module_liveness_init(&module_liveness));
    kefir_result_t res = KEFIR_OK;
    if (analyze) {
        res = kefir_opt_module_liveness_trace(mem, &module_liveness, module);
    }

    REQUIRE_CHAIN(&res, kefir_json_output_object_begin(json));

    REQUIRE_CHAIN(&res, kefir_json_output_object_key(json, "functions"));
    REQUIRE_CHAIN(&res, kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->functions, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        struct kefir_opt_code_control_flow control_flow;
        struct kefir_opt_code_liveness liveness;
        struct kefir_opt_code_memssa memssa;
        REQUIRE_CHAIN(&res, kefir_opt_code_control_flow_init(&control_flow));
        REQUIRE_CHAIN(&res, kefir_opt_code_liveness_init(&liveness));
        REQUIRE_CHAIN(&res, kefir_opt_code_memssa_init(&memssa));

        kefir_result_t res = KEFIR_OK;
        ASSIGN_DECL_CAST(const struct kefir_opt_function *, function, node->value);
        if (analyze) {
            REQUIRE_CHAIN(&res, kefir_opt_code_control_flow_build(mem, &control_flow, &function->code));
            REQUIRE_CHAIN(&res, kefir_opt_code_liveness_build(mem, &liveness, &control_flow));
            REQUIRE_CHAIN(&res,
                          kefir_opt_code_memssa_construct(mem, &memssa, &function->code, &control_flow, &liveness));
        }

        if (!analyze || kefir_opt_module_is_symbol_alive(&module_liveness, function->ir_func->declaration->name)) {
            REQUIRE_CHAIN(&res, format_function(mem, json, module, function, analyze ? &control_flow : NULL,
                                                analyze ? &liveness : NULL, analyze ? &memssa : NULL, debug_info));
        }

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_opt_code_memssa_free(mem, &memssa);
            kefir_opt_code_liveness_free(mem, &liveness);
            kefir_opt_code_control_flow_free(mem, &control_flow);
            kefir_opt_module_liveness_free(mem, &module_liveness);
            return res;
        });
        res = kefir_opt_code_memssa_free(mem, &memssa);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_opt_code_liveness_free(mem, &liveness);
            kefir_opt_code_control_flow_free(mem, &control_flow);
            kefir_opt_module_liveness_free(mem, &module_liveness);
            return res;
        });
        res = kefir_opt_code_liveness_free(mem, &liveness);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_opt_code_control_flow_free(mem, &control_flow);
            kefir_opt_module_liveness_free(mem, &module_liveness);
            return res;
        });
        res = kefir_opt_code_control_flow_free(mem, &control_flow);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_opt_module_liveness_free(mem, &module_liveness);
            return res;
        });
    }
    REQUIRE_CHAIN(&res, kefir_json_output_array_end(json));

    REQUIRE_CHAIN(&res, kefir_json_output_object_end(json));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_module_liveness_free(mem, &module_liveness);
        return res;
    });
    REQUIRE_OK(kefir_opt_module_liveness_free(mem, &module_liveness));
    return KEFIR_OK;
}
