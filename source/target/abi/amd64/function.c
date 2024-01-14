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

#include "kefir/target/abi/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static const kefir_asm_amd64_xasmgen_register_t SYSV_CALLEE_PRESERVED_GENERAL_PURPOSE_REGS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_RBX, KEFIR_AMD64_XASMGEN_REGISTER_R12, KEFIR_AMD64_XASMGEN_REGISTER_R13,
    KEFIR_AMD64_XASMGEN_REGISTER_R14, KEFIR_AMD64_XASMGEN_REGISTER_R15};

static const kefir_size_t NUM_OF_SYSV_CALLEE_PRESERVED_GENERAL_PURPOSE_REGS =
    sizeof(SYSV_CALLEE_PRESERVED_GENERAL_PURPOSE_REGS) / sizeof(SYSV_CALLEE_PRESERVED_GENERAL_PURPOSE_REGS[0]);

static const kefir_asm_amd64_xasmgen_register_t SYSV_CALLER_PRESERVED_GENERAL_PURPOSE_REGS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_RAX, KEFIR_AMD64_XASMGEN_REGISTER_RCX, KEFIR_AMD64_XASMGEN_REGISTER_RDX,
    KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDI, KEFIR_AMD64_XASMGEN_REGISTER_R8,
    KEFIR_AMD64_XASMGEN_REGISTER_R9,  KEFIR_AMD64_XASMGEN_REGISTER_R10, KEFIR_AMD64_XASMGEN_REGISTER_R11};

static const kefir_size_t NUM_OF_SYSV_CALLER_PRESERVED_GENERAL_PURPOSE_REGS =
    sizeof(SYSV_CALLER_PRESERVED_GENERAL_PURPOSE_REGS) / sizeof(SYSV_CALLER_PRESERVED_GENERAL_PURPOSE_REGS[0]);

static const kefir_asm_amd64_xasmgen_register_t SYSV_CALLER_PRESERVED_SSE_REGS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_XMM0,  KEFIR_AMD64_XASMGEN_REGISTER_XMM1,  KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM3,  KEFIR_AMD64_XASMGEN_REGISTER_XMM4,  KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM6,  KEFIR_AMD64_XASMGEN_REGISTER_XMM7,  KEFIR_AMD64_XASMGEN_REGISTER_XMM8,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM9,  KEFIR_AMD64_XASMGEN_REGISTER_XMM10, KEFIR_AMD64_XASMGEN_REGISTER_XMM11,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM12, KEFIR_AMD64_XASMGEN_REGISTER_XMM13, KEFIR_AMD64_XASMGEN_REGISTER_XMM14,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM15};

static const kefir_size_t NUM_OF_SYSV_CALLER_PRESERVED_SSE_REGS =
    sizeof(SYSV_CALLER_PRESERVED_SSE_REGS) / sizeof(SYSV_CALLER_PRESERVED_SSE_REGS[0]);

kefir_result_t kefir_abi_amd64_get_callee_preserved_general_purpose_register(
    kefir_abi_amd64_variant_t variant, kefir_size_t index, kefir_asm_amd64_xasmgen_register_t *reg_ptr) {
    REQUIRE(reg_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(index < NUM_OF_SYSV_CALLEE_PRESERVED_GENERAL_PURPOSE_REGS,
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested register index is out of bounds"));
            *reg_ptr = SYSV_CALLEE_PRESERVED_GENERAL_PURPOSE_REGS[index];
            break;
    }
    return KEFIR_OK;
}

kefir_size_t kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(kefir_abi_amd64_variant_t variant) {
    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            return NUM_OF_SYSV_CALLEE_PRESERVED_GENERAL_PURPOSE_REGS;
    }

    return 0;
}

kefir_result_t kefir_abi_amd64_get_caller_preserved_general_purpose_register(
    kefir_abi_amd64_variant_t variant, kefir_size_t index, kefir_asm_amd64_xasmgen_register_t *reg_ptr) {
    REQUIRE(reg_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(index < NUM_OF_SYSV_CALLER_PRESERVED_GENERAL_PURPOSE_REGS,
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested register index is out of bounds"));
            *reg_ptr = SYSV_CALLER_PRESERVED_GENERAL_PURPOSE_REGS[index];
            break;
    }
    return KEFIR_OK;
}

kefir_size_t kefir_abi_amd64_num_of_caller_preserved_general_purpose_registers(kefir_abi_amd64_variant_t variant) {
    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            return NUM_OF_SYSV_CALLER_PRESERVED_GENERAL_PURPOSE_REGS;
    }

    return 0;
}

kefir_result_t kefir_abi_amd64_get_caller_preserved_sse_register(kefir_abi_amd64_variant_t variant, kefir_size_t index,
                                                                 kefir_asm_amd64_xasmgen_register_t *reg_ptr) {
    REQUIRE(reg_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register"));

    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE(index < NUM_OF_SYSV_CALLER_PRESERVED_SSE_REGS,
                    KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested register index is out of bounds"));
            *reg_ptr = SYSV_CALLER_PRESERVED_SSE_REGS[index];
            break;
    }
    return KEFIR_OK;
}

kefir_size_t kefir_abi_amd64_num_of_caller_preserved_sse_registers(kefir_abi_amd64_variant_t variant) {
    switch (variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            return NUM_OF_SYSV_CALLER_PRESERVED_SSE_REGS;
    }

    return 0;
}

struct abi_amd64_payload {
    const struct kefir_ir_function_decl *decl;
    kefir_abi_amd64_variant_t variant;
    struct {
        struct kefir_abi_amd64_type_layout layout;
        struct kefir_abi_amd64_function_parameters parameters;
    } parameters;

    struct {
        struct kefir_abi_amd64_type_layout layout;
        struct kefir_abi_amd64_function_parameters parameters;
        bool implicit_parameter;
    } returns;
};

static kefir_result_t function_alloc_return(struct kefir_mem *mem, struct abi_amd64_payload *payload) {
    REQUIRE(kefir_ir_type_children(payload->decl->result) <= 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected IR function to have return type count less than 2"));
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, payload->variant, payload->decl->result, &payload->returns.layout));
    kefir_result_t res = kefir_abi_amd64_function_parameters_classify(
        mem, payload->variant, payload->decl->result, &payload->returns.layout, &payload->returns.parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &payload->returns.layout);
        return res;
    });
    res = kefir_abi_amd64_function_parameters_allocate_return(mem, &payload->returns.parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_function_parameters_free(mem, &payload->returns.parameters);
        kefir_abi_amd64_type_layout_free(mem, &payload->returns.layout);
        return res;
    });
    payload->returns.implicit_parameter = false;
    if (kefir_ir_type_children(payload->decl->result) > 0) {
        struct kefir_abi_amd64_function_parameter param;
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at(&payload->returns.parameters, 0, &param));
        payload->returns.implicit_parameter = param.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY;
    }
    return KEFIR_OK;
}

static kefir_result_t function_alloc_params(struct kefir_mem *mem, struct abi_amd64_payload *payload) {
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, payload->decl->params,
                                           &payload->parameters.layout));

    kefir_result_t res = kefir_abi_amd64_function_parameters_classify(
        mem, payload->variant, payload->decl->params, &payload->parameters.layout, &payload->parameters.parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &payload->parameters.layout);
        return res;
    });
    if (payload->returns.implicit_parameter) {
        REQUIRE_OK(kefir_abi_amd64_function_parameters_reserve(
            &payload->parameters.parameters,
            &(struct kefir_abi_amd64_function_parameter_requirements){.general_purpose_regs = 1}));
    }
    res = kefir_abi_amd64_function_parameters_allocate(mem, &payload->parameters.parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_function_parameters_free(mem, &payload->parameters.parameters);
        kefir_abi_amd64_type_layout_free(mem, &payload->parameters.layout);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_alloc(struct kefir_mem *mem, kefir_abi_amd64_variant_t variant,
                                                   const struct kefir_ir_function_decl *decl,
                                                   struct kefir_abi_amd64_function_decl *abi_decl) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function declaration"));
    REQUIRE(abi_decl != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 function declaration"));

    struct abi_amd64_payload *payload = KEFIR_MALLOC(mem, sizeof(struct abi_amd64_payload));
    REQUIRE(payload != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 abi function declaration"));

    payload->decl = decl;
    payload->variant = variant;
    kefir_result_t res = function_alloc_return(mem, payload);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, payload);
        return res;
    });
    res = function_alloc_params(mem, payload);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_function_parameters_free(mem, &payload->returns.parameters);
        kefir_abi_amd64_type_layout_free(mem, &payload->returns.layout);
        KEFIR_FREE(mem, payload);
        return res;
    });

    abi_decl->payload = payload;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_free(struct kefir_mem *mem,
                                                  struct kefir_abi_amd64_function_decl *abi_decl) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(abi_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function declaration"));

    ASSIGN_DECL_CAST(struct abi_amd64_payload *, payload, abi_decl->payload);

    REQUIRE_OK(kefir_abi_amd64_function_parameters_free(mem, &payload->returns.parameters));
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &payload->returns.layout));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_free(mem, &payload->parameters.parameters));
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &payload->parameters.layout));
    KEFIR_FREE(mem, payload);
    abi_decl->payload = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_ir(const struct kefir_abi_amd64_function_decl *abi_decl,
                                                const struct kefir_ir_function_decl **ir_decl) {
    REQUIRE(abi_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function declaration"));
    REQUIRE(ir_decl != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR function declaration"));

    ASSIGN_DECL_CAST(struct abi_amd64_payload *, payload, abi_decl->payload);
    *ir_decl = payload->decl;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_parameters(const struct kefir_abi_amd64_function_decl *abi_decl,
                                                        const struct kefir_abi_amd64_function_parameters **params_ptr) {
    REQUIRE(abi_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function declaration"));
    REQUIRE(params_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 parameters"));

    ASSIGN_DECL_CAST(struct abi_amd64_payload *, payload, abi_decl->payload);
    *params_ptr = &payload->parameters.parameters;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_parameters_layout(
    const struct kefir_abi_amd64_function_decl *abi_decl, const struct kefir_abi_amd64_type_layout **type_layout_ptr) {
    REQUIRE(abi_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function declaration"));
    REQUIRE(type_layout_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 type layout"));

    ASSIGN_DECL_CAST(struct abi_amd64_payload *, payload, abi_decl->payload);
    *type_layout_ptr = &payload->parameters.layout;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_parameters_sse_reqs(const struct kefir_abi_amd64_function_decl *abi_decl,
                                                                 kefir_bool_t *sse_reqs,
                                                                 kefir_asm_amd64_xasmgen_register_t *sse_reqs_reg) {
    REQUIRE(abi_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function declaration"));
    REQUIRE(sse_reqs != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    REQUIRE(sse_reqs_reg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to register"));

    ASSIGN_DECL_CAST(struct abi_amd64_payload *, payload, abi_decl->payload);
    if (payload->variant == KEFIR_ABI_AMD64_VARIANT_SYSTEM_V && payload->decl->vararg) {
        *sse_reqs = true;
        *sse_reqs_reg = KEFIR_AMD64_XASMGEN_REGISTER_RAX;
    } else {
        *sse_reqs = false;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_returns(const struct kefir_abi_amd64_function_decl *abi_decl,
                                                     const struct kefir_abi_amd64_function_parameters **params_ptr) {
    REQUIRE(abi_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function declaration"));
    REQUIRE(params_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 return parameters"));

    ASSIGN_DECL_CAST(struct abi_amd64_payload *, payload, abi_decl->payload);
    *params_ptr = &payload->returns.parameters;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_returns_layout(
    const struct kefir_abi_amd64_function_decl *abi_decl, const struct kefir_abi_amd64_type_layout **type_layout_ptr) {
    REQUIRE(abi_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function declaration"));
    REQUIRE(type_layout_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 type layout"));

    ASSIGN_DECL_CAST(struct abi_amd64_payload *, payload, abi_decl->payload);
    *type_layout_ptr = &payload->returns.layout;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_function_decl_returns_implicit_parameter(
    const struct kefir_abi_amd64_function_decl *abi_decl, kefir_bool_t *implicit_param,
    kefir_asm_amd64_xasmgen_register_t *implicit_param_reg) {
    REQUIRE(abi_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 function declaration"));
    REQUIRE(implicit_param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to implicit parameter flag"));
    REQUIRE(implicit_param_reg != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to implicit parameter register"));

    ASSIGN_DECL_CAST(struct abi_amd64_payload *, payload, abi_decl->payload);
    *implicit_param = payload->returns.implicit_parameter;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(payload->variant, 0, implicit_param_reg));
    return KEFIR_OK;
}
