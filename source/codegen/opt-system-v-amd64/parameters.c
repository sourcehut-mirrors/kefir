/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "kefir/codegen/opt-system-v-amd64/parameters.h"
#include "kefir/ir/builtins.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Provided function parameter type is not yet implemented");
}

struct parameter_traversal_param {
    struct kefir_mem *mem;
    kefir_size_t argument_index;
    const struct kefir_abi_amd64_function_decl *target_func_decl;
    struct kefir_hashtree *parameters;
};

static kefir_result_t insert_argument(struct kefir_mem *mem, struct kefir_hashtree *parameters, kefir_size_t index,
                                      const struct kefir_codegen_opt_amd64_sysv_function_parameter_location *location) {
    struct kefir_codegen_opt_amd64_sysv_function_parameter_location *argument =
        KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_opt_amd64_sysv_function_parameter_location));
    REQUIRE(argument != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer codegen parameter location"));

    memcpy(argument, location, sizeof(struct kefir_codegen_opt_amd64_sysv_function_parameter_location));
    kefir_result_t res =
        kefir_hashtree_insert(mem, parameters, (kefir_hashtree_key_t) index, (kefir_hashtree_value_t) argument);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, argument);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t traverse_integer_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                                const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type entry"));
    ASSIGN_DECL_CAST(struct parameter_traversal_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen argument traversal parameter"));

    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(param->target_func_decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));

    struct kefir_codegen_opt_amd64_sysv_function_parameter_location argument = {0};
    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER) {
        argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_DIRECT;
        argument.direct = parameter.direct_reg;
    } else {
        REQUIRE(parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 integer function parameter location"));
        argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_INDIRECT;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
            &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &argument.indirect.base,
            &argument.indirect.offset));
        argument.indirect.aggregate = false;
    }
    argument.parameter = parameter;
    REQUIRE_OK(insert_argument(param->mem, param->parameters, param->argument_index, &argument));
    param->argument_index++;
    return KEFIR_OK;
}

static kefir_result_t traverse_sse_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type entry"));
    ASSIGN_DECL_CAST(struct parameter_traversal_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen argument traversal parameter"));

    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(param->target_func_decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));

    struct kefir_codegen_opt_amd64_sysv_function_parameter_location argument = {0};
    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER) {
        argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_FLOATING_POINT_DIRECT;
        argument.direct = parameter.direct_reg;
    } else {
        REQUIRE(parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 sse function parameter location"));
        argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_INDIRECT;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
            &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &argument.indirect.base,
            &argument.indirect.offset));
        argument.indirect.aggregate = false;
    }
    argument.parameter = parameter;
    REQUIRE_OK(insert_argument(param->mem, param->parameters, param->argument_index, &argument));
    param->argument_index++;
    return KEFIR_OK;
}

static kefir_result_t traverse_aggregate_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                                  const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type entry"));
    ASSIGN_DECL_CAST(struct parameter_traversal_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen argument traversal parameter"));

    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(param->target_func_decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));

    struct kefir_codegen_opt_amd64_sysv_function_parameter_location argument = {0};
    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY) {
        argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_INDIRECT;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
            &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &argument.indirect.base,
            &argument.indirect.offset));
        argument.indirect.aggregate = true;
    } else {
        REQUIRE(parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 aggregate function parameter location"));
        argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_REGISTER_AGGREGATE;
        const struct kefir_abi_amd64_type_layout *parameters_layout;
        const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
        REQUIRE_OK(kefir_abi_amd64_function_decl_parameters_layout(param->target_func_decl, &parameters_layout));
        REQUIRE_OK(kefir_abi_amd64_type_layout_at(parameters_layout, param->argument_index, &layout));
        argument.register_aggregate_props.size = layout->size;
        argument.register_aggregate_props.alignment = layout->alignment;
    }
    argument.parameter = parameter;
    REQUIRE_OK(insert_argument(param->mem, param->parameters, param->argument_index, &argument));
    param->argument_index++;
    return KEFIR_OK;
}

static kefir_result_t traverse_builtin_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                                const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type entry"));
    ASSIGN_DECL_CAST(struct parameter_traversal_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen argument traversal parameter"));

    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(param->target_func_decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));

    struct kefir_codegen_opt_amd64_sysv_function_parameter_location argument = {0};
    switch (typeentry->param) {
        case KEFIR_IR_TYPE_BUILTIN_VARARG:
            if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER) {
                argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_DIRECT;
                argument.direct = parameter.direct_reg;
            } else {
                REQUIRE(parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location"));
                argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_INDIRECT;
                REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
                    &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &argument.indirect.base,
                    &argument.indirect.offset));
                argument.indirect.aggregate = false;
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR builtin type");
    }
    argument.parameter = parameter;
    REQUIRE_OK(insert_argument(param->mem, param->parameters, param->argument_index, &argument));
    param->argument_index++;
    return KEFIR_OK;
}

static kefir_result_t traverse_long_double_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                                    const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type entry"));
    ASSIGN_DECL_CAST(struct parameter_traversal_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen argument traversal parameter"));

    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(param->target_func_decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));

    struct kefir_codegen_opt_amd64_sysv_function_parameter_location argument = {0};
    argument.type = KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_INDIRECT;
    REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(&parameter,
                                                                  KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE,
                                                                  &argument.indirect.base, &argument.indirect.offset));
    argument.indirect.aggregate = true;
    argument.parameter = parameter;
    REQUIRE_OK(insert_argument(param->mem, param->parameters, param->argument_index, &argument));
    param->argument_index++;
    return KEFIR_OK;
}

static kefir_result_t parameters_init(struct kefir_mem *mem,
                                      struct kefir_codegen_opt_amd64_sysv_function_parameters *parameters) {
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, traverse_integer_argument);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, traverse_sse_argument);
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = traverse_long_double_argument;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = traverse_aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_UNION] = traverse_aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = traverse_aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = traverse_builtin_argument;
    struct parameter_traversal_param param = {.mem = mem,
                                              .target_func_decl = parameters->function_decl,
                                              .argument_index = 0,
                                              .parameters = &parameters->parameters};

    const struct kefir_ir_function_decl *ir_func_decl;
    REQUIRE_OK(kefir_abi_amd64_function_decl_ir(parameters->function_decl, &ir_func_decl));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(ir_func_decl->params, &visitor, (void *) &param, 0,
                                                kefir_ir_type_children(ir_func_decl->params)));
    return KEFIR_OK;
}

static kefir_result_t free_parameter(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                     kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_opt_amd64_sysv_function_parameter_location *, parameter, value);
    REQUIRE(parameter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen function parameter"));

    memset(parameter, 0, sizeof(struct kefir_codegen_opt_amd64_sysv_function_parameter_location));
    KEFIR_FREE(mem, parameter);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_function_parameters_init(
    struct kefir_mem *mem, const struct kefir_abi_amd64_function_decl *func_decl,
    struct kefir_codegen_opt_amd64_sysv_function_parameters *parameters) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func_decl != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V function declaration"));
    REQUIRE(parameters != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                                "Expected valid pointer to optimizer codegen function parameters"));

    REQUIRE_OK(kefir_hashtree_init(&parameters->parameters, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&parameters->parameters, free_parameter, NULL));
    parameters->function_decl = func_decl;

    kefir_result_t res = parameters_init(mem, parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &parameters->parameters);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_function_parameters_free(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64_sysv_function_parameters *parameters) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parameters != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen function parameters"));

    REQUIRE_OK(kefir_hashtree_free(mem, &parameters->parameters));
    memset(parameters, 0, sizeof(struct kefir_codegen_opt_amd64_sysv_function_parameters));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_amd64_sysv_function_parameter_location_of(
    const struct kefir_codegen_opt_amd64_sysv_function_parameters *parameters, kefir_size_t index,
    const struct kefir_codegen_opt_amd64_sysv_function_parameter_location **parameter_ptr) {
    REQUIRE(parameters != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen function parameters"));
    REQUIRE(parameter_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen function parameter"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&parameters->parameters, (kefir_hashtree_key_t) index, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer codegen function parameter");
    }
    REQUIRE_OK(res);

    *parameter_ptr = (const struct kefir_codegen_opt_amd64_sysv_function_parameter_location *) node->value;
    return KEFIR_OK;
}
