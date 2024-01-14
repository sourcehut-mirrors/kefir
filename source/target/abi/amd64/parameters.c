/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/target/abi/amd64/system-v/parameters.h"
#include "kefir/target/abi/amd64/return.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/mem.h"
#include "kefir/ir/builtins.h"
#include <string.h>

kefir_size_t kefir_abi_amd64_num_of_general_purpose_parameter_registers(kefir_abi_amd64_variant_t variant) {
    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            return KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS_LENGTH;

        default:
            return 0;
    }
}

kefir_result_t kefir_abi_amd64_general_purpose_parameter_register(kefir_abi_amd64_variant_t variant, kefir_size_t index,
                                                                  kefir_asm_amd64_xasmgen_register_t *reg) {
    REQUIRE(reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(index < kefir_abi_amd64_num_of_general_purpose_parameter_registers(variant),
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided parameter register index is out of abi bounds"));
            *reg = KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[index];
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_size_t kefir_abi_amd64_num_of_sse_parameter_registers(kefir_abi_amd64_variant_t variant) {
    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            return KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS_LENGTH;

        default:
            return 0;
    }
}

kefir_result_t kefir_abi_amd64_sse_parameter_register(kefir_abi_amd64_variant_t variant, kefir_size_t index,
                                                      kefir_asm_amd64_xasmgen_register_t *reg) {
    REQUIRE(reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(index < kefir_abi_amd64_num_of_sse_parameter_registers(variant),
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided parameter register index is out of abi bounds"));
            *reg = KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS[index];
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

struct amd64_abi_payload {
    kefir_bool_t preliminary_classification;
    kefir_bool_t return_parameters;
    kefir_abi_amd64_variant_t variant;
    const struct kefir_ir_type *parameter_type;
    const struct kefir_abi_amd64_type_layout *parameter_layout;
    union {
        struct {
            struct kefir_abi_sysv_amd64_parameter_allocation *allocation;
            kefir_size_t allocation_length;
            struct kefir_abi_sysv_amd64_parameter_location location;
        } sysv;
    };
};

kefir_result_t kefir_abi_amd64_function_parameters_classify(
    struct kefir_mem *mem, kefir_abi_amd64_variant_t variant, const struct kefir_ir_type *parameter_type,
    const struct kefir_abi_amd64_type_layout *parameter_layout,
    struct kefir_abi_amd64_function_parameters *parameter_allocation) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parameter_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parameter IR type"));
    REQUIRE(parameter_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parameter type layout"));
    REQUIRE(parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 function parameter allocation"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V: {
            struct amd64_abi_payload *payload = KEFIR_MALLOC(mem, sizeof(struct amd64_abi_payload));
            REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE,
                                                     "Failed to allocate amd64 abi function parameter allocation"));
            payload->preliminary_classification = true;
            payload->variant = variant;
            payload->parameter_type = parameter_type;
            payload->parameter_layout = parameter_layout;
            payload->sysv.allocation_length = kefir_ir_type_slots(parameter_type);
            payload->sysv.location = (struct kefir_abi_sysv_amd64_parameter_location){0};
            payload->sysv.allocation = KEFIR_MALLOC(
                mem, sizeof(struct kefir_abi_sysv_amd64_parameter_allocation) * payload->sysv.allocation_length);
            REQUIRE_ELSE(payload->sysv.allocation != NULL, {
                KEFIR_FREE(mem, payload);
                return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE,
                                       "Failed to allocate amd64 abi function parameter allocation");
            });
            kefir_result_t res = kefir_abi_sysv_amd64_parameter_classify(mem, parameter_type, parameter_layout,
                                                                         payload->sysv.allocation);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, payload->sysv.allocation);
                KEFIR_FREE(mem, payload);
                return res;
            });
            parameter_allocation->payload = payload;
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameters_allocate(
    struct kefir_mem *mem, struct kefir_abi_amd64_function_parameters *parameter_allocation) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid  amd64 function parameter allocation"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter_allocation->payload);
    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(payload->preliminary_classification,
                    KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                    "Expected amd64 function parameters to be classified prior to allocation"));
            payload->preliminary_classification = false;
            payload->return_parameters = false;
            REQUIRE_OK(kefir_abi_sysv_amd64_parameter_allocate(mem, payload->parameter_type, payload->parameter_layout,
                                                               payload->sysv.allocation, &payload->sysv.location));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameters_allocate_return(
    struct kefir_mem *mem, struct kefir_abi_amd64_function_parameters *parameter_allocation) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid  amd64 function parameter allocation"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter_allocation->payload);
    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(payload->preliminary_classification,
                    KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                    "Expected amd64 function parameters to be classified prior to allocation"));
            payload->preliminary_classification = false;
            payload->return_parameters = true;
            REQUIRE_OK(
                kefir_abi_sysv_amd64_parameter_allocate_return(mem, payload->parameter_type, payload->parameter_layout,
                                                               payload->sysv.allocation, &payload->sysv.location));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameters_reserve(
    const struct kefir_abi_amd64_function_parameters *parameter_allocation,
    const struct kefir_abi_amd64_function_parameter_requirements *reserved) {
    REQUIRE(parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid  amd64 function parameter allocation"));
    REQUIRE(reserved != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter requirements"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter_allocation->payload);
    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            payload->sysv.location.integer_register =
                MAX(payload->sysv.location.integer_register, reserved->general_purpose_regs);
            payload->sysv.location.sse_register = MAX(payload->sysv.location.sse_register, reserved->sse_regs);
            payload->sysv.location.stack_offset = MAX(payload->sysv.location.stack_offset, reserved->stack);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameters_free(
    struct kefir_mem *mem, struct kefir_abi_amd64_function_parameters *parameter_allocation) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter allocation"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter_allocation->payload);
    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            for (kefir_size_t i = 0; i < payload->sysv.allocation_length; i++) {
                if (payload->sysv.allocation[i].type == KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER) {
                    REQUIRE_OK(kefir_abi_amd64_sysv_qwords_free(&payload->sysv.allocation[i].container, mem));
                }
            }
            KEFIR_FREE(mem, payload->sysv.allocation);
            KEFIR_FREE(mem, payload);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }

    memset(parameter_allocation, 0, sizeof(struct kefir_abi_amd64_function_parameters));
    return KEFIR_OK;
}

static kefir_result_t amd64_sysv_parameter_at(struct amd64_abi_payload *payload, kefir_size_t slot,
                                              struct kefir_abi_amd64_function_parameter *parameter) {
    REQUIRE(slot < payload->sysv.allocation_length,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided function parameter slot is out of bounds"));

    const struct kefir_abi_sysv_amd64_parameter_allocation *const param_alloc = &payload->sysv.allocation[slot];
    parameter->preliminary_classification = payload->preliminary_classification;
    switch (param_alloc->klass) {
        case KEFIR_AMD64_SYSV_PARAM_INTEGER:
            parameter->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER;
            if (!payload->preliminary_classification) {
                if (!payload->return_parameters) {
                    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(
                        KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, param_alloc->location.integer_register,
                        &parameter->direct_reg));
                } else {
                    REQUIRE_OK(kefir_abi_amd64_general_purpose_return_register(KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                                                               param_alloc->location.integer_register,
                                                                               &parameter->direct_reg));
                }
            }
            break;

        case KEFIR_AMD64_SYSV_PARAM_SSE:
            parameter->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER;
            if (!payload->preliminary_classification) {
                if (!payload->return_parameters) {
                    REQUIRE_OK(kefir_abi_amd64_sse_parameter_register(
                        KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, param_alloc->location.sse_register, &parameter->direct_reg));
                } else {
                    REQUIRE_OK(kefir_abi_amd64_sse_return_register(
                        KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, param_alloc->location.sse_register, &parameter->direct_reg));
                }
            }
            break;

        case KEFIR_AMD64_SYSV_PARAM_X87:
            parameter->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87;
            break;

        case KEFIR_AMD64_SYSV_PARAM_X87UP:
            parameter->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP;
            break;

        case KEFIR_AMD64_SYSV_PARAM_MEMORY:
            parameter->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY;
            if (!payload->preliminary_classification) {
                parameter->tos_offset = param_alloc->location.stack_offset;
            }
            break;

        case KEFIR_AMD64_SYSV_PARAM_NO_CLASS:
            if (param_alloc->type == KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER) {
                parameter->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS;
                parameter->multi_reg = (void *) param_alloc;
            } else if (param_alloc->type == KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER) {
                parameter->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE;
            } else {
                REQUIRE(param_alloc->type == KEFIR_AMD64_SYSV_INPUT_PARAM_NESTED,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 sysv abi parameter type"));
                parameter->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED;
                parameter->nested.index = param_alloc->container_reference.qword->index;
                parameter->nested.allocation = param_alloc->container_reference.qword->location;
                parameter->nested.offset = param_alloc->container_reference.offset;
                switch (param_alloc->container_reference.qword->klass) {
                    case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                        parameter->nested.location =
                            KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER;
                        break;

                    case KEFIR_AMD64_SYSV_PARAM_SSE:
                    case KEFIR_AMD64_SYSV_PARAM_SSEUP:
                        parameter->nested.location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER;
                        break;

                    case KEFIR_AMD64_SYSV_PARAM_X87:
                        parameter->nested.location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87;
                        break;

                    case KEFIR_AMD64_SYSV_PARAM_X87UP:
                        parameter->nested.location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP;
                        break;

                    case KEFIR_AMD64_SYSV_PARAM_NO_CLASS:
                        parameter->nested.location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE;
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected QWord location");
                }
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 sysv abi parameter class");
    }
    parameter->requirements.general_purpose_regs = param_alloc->requirements.integer;
    parameter->requirements.sse_regs = param_alloc->requirements.sse;
    parameter->requirements.sseup_regs = param_alloc->requirements.sseup;
    parameter->requirements.x87 = param_alloc->requirements.x87;
    parameter->requirements.memory_size = param_alloc->requirements.memory.size;
    parameter->requirements.memory_alignment = param_alloc->requirements.memory.alignment;
    parameter->payload = payload;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameters_at(
    const struct kefir_abi_amd64_function_parameters *parameter_allocation, kefir_size_t slot,
    struct kefir_abi_amd64_function_parameter *parameter) {
    REQUIRE(parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter allocation"));
    REQUIRE(parameter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 function parameter"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter_allocation->payload);

    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE_OK(amd64_sysv_parameter_at(payload, slot, parameter));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameters_requirements(
    const struct kefir_abi_amd64_function_parameters *parameter_allocation,
    struct kefir_abi_amd64_function_parameter_requirements *reqs) {
    REQUIRE(parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter allocation"));
    REQUIRE(reqs != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid pointer to amd64 function parameter requirements"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter_allocation->payload);

    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            reqs->general_purpose_regs = payload->sysv.location.integer_register;
            reqs->sse_regs = payload->sysv.location.sse_register;
            reqs->stack = payload->sysv.location.stack_offset;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameter_multireg_length(
    const struct kefir_abi_amd64_function_parameter *parameter, kefir_size_t *length_ptr) {
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter"));
    REQUIRE(parameter->location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected multiple-register amd64 function parameter"));
    REQUIRE(length_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to length"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter->payload);
    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V: {
            ASSIGN_DECL_CAST(const struct kefir_abi_sysv_amd64_parameter_allocation *, param_alloc,
                             parameter->multi_reg);
            REQUIRE(param_alloc->type == KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 sysv function parameter class"));
            *length_ptr = kefir_vector_length(&param_alloc->container.qwords);
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameter_multireg_at(
    const struct kefir_abi_amd64_function_parameter *parameter, kefir_size_t index,
    struct kefir_abi_amd64_function_parameter *param_ptr) {
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter"));
    REQUIRE(parameter->location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected multiple-register amd64 function parameter"));
    REQUIRE(param_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 function parameter"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter->payload);
    memset(param_ptr, 0, sizeof(struct kefir_abi_amd64_function_parameter));
    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V: {
            ASSIGN_DECL_CAST(const struct kefir_abi_sysv_amd64_parameter_allocation *, param_alloc,
                             parameter->multi_reg);
            REQUIRE(param_alloc->type == KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 sysv function parameter class"));
            REQUIRE(index < kefir_vector_length(&param_alloc->container.qwords),
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS,
                                    "Requested amd64 sysv function parameter qword is out of parameter bounds"));
            ASSIGN_DECL_CAST(struct kefir_abi_amd64_sysv_qword *, qword,
                             kefir_vector_at(&param_alloc->container.qwords, index));
            param_ptr->preliminary_classification = payload->preliminary_classification;
            switch (qword->klass) {
                case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                    param_ptr->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER;
                    if (!payload->preliminary_classification) {
                        if (!payload->return_parameters) {
                            REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(
                                KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, qword->location, &param_ptr->direct_reg));
                        } else {
                            REQUIRE_OK(kefir_abi_amd64_general_purpose_return_register(
                                KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, qword->location, &param_ptr->direct_reg));
                        }
                    }
                    break;

                case KEFIR_AMD64_SYSV_PARAM_SSE:
                    param_ptr->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER;
                    if (!payload->preliminary_classification) {
                        if (!payload->return_parameters) {
                            REQUIRE_OK(kefir_abi_amd64_sse_parameter_register(KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                                                              qword->location, &param_ptr->direct_reg));
                        } else {
                            REQUIRE_OK(kefir_abi_amd64_sse_return_register(KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                                                           qword->location, &param_ptr->direct_reg));
                        }
                    }
                    break;

                case KEFIR_AMD64_SYSV_PARAM_X87:
                    param_ptr->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87;
                    break;

                case KEFIR_AMD64_SYSV_PARAM_X87UP:
                    param_ptr->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP;
                    break;

                case KEFIR_AMD64_SYSV_PARAM_NO_CLASS:
                    param_ptr->location = KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 sysv qword class");
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_parameter_memory_location(
    const struct kefir_abi_amd64_function_parameter *parameter, kefir_abi_amd64_function_parameter_memory_basis_t basis,
    kefir_asm_amd64_xasmgen_register_t *reg_ptr, kefir_int64_t *offset_ptr) {
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter"));
    REQUIRE(parameter->location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY &&
                !parameter->preliminary_classification,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter"));
    REQUIRE(reg_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));
    REQUIRE(offset_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to offset"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter->payload);
    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V: {
            switch (basis) {
                case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER:
                    *reg_ptr = KEFIR_AMD64_XASMGEN_REGISTER_RSP;
                    *offset_ptr = parameter->tos_offset;
                    break;

                case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE:
                    *reg_ptr = KEFIR_AMD64_XASMGEN_REGISTER_RBP;
                    *offset_ptr = parameter->tos_offset + 2 * KEFIR_AMD64_ABI_QWORD;
                    break;
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_return_memory_location(
    const struct kefir_abi_amd64_function_parameter *parameter, kefir_abi_amd64_function_parameter_memory_basis_t basis,
    kefir_asm_amd64_xasmgen_register_t *reg_ptr, kefir_int64_t *offset_ptr) {
    REQUIRE(parameter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter"));
    REQUIRE(parameter->location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY &&
                !parameter->preliminary_classification,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function parameter"));
    REQUIRE(reg_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));
    REQUIRE(offset_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to offset"));

    ASSIGN_DECL_CAST(struct amd64_abi_payload *, payload, parameter->payload);
    switch (payload->variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V: {
            switch (basis) {
                case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER:
                case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE:
                    *reg_ptr = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
                    *offset_ptr = 0;
                    break;
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}
