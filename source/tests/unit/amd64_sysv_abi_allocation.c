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

#include <stdalign.h>
#include <inttypes.h>
#include "kefir/test/unit_test.h"
#include "kefir/ir/type.h"
#include "kefir/ir/builder.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/target/abi/amd64/parameters.h"

#define ASSERT_PARAM_REGISTER_ALLOCATION(_parameters, _index, _location, _direct_reg)        \
    do {                                                                                     \
        struct kefir_abi_amd64_function_parameter param;                                     \
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at((_parameters), (_index), &param)); \
        ASSERT(param.location == (_location));                                               \
        ASSERT(param.direct_reg == (_direct_reg));                                           \
    } while (0)

#define ASSERT_PARAM_MULTPLE_REGISTER_ALLOCATION(_parameters, _index)                             \
    do {                                                                                          \
        struct kefir_abi_amd64_function_parameter param;                                          \
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at((_parameters), (_index), &param));      \
        ASSERT(param.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS); \
    } while (0)

#define ASSERT_PARAM_NONE_ALLOCATION(_parameters, _index)                                    \
    do {                                                                                     \
        struct kefir_abi_amd64_function_parameter param;                                     \
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at((_parameters), (_index), &param)); \
        ASSERT(param.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);          \
    } while (0)

#define ASSERT_PARAM_STACK_ALLOCATION(_parameters, _index, _offset)                          \
    do {                                                                                     \
        struct kefir_abi_amd64_function_parameter param;                                     \
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at((_parameters), (_index), &param)); \
        ASSERT(param.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY);        \
        ASSERT(param.tos_offset == (_offset));                                               \
    } while (0)

#define ASSERT_NESTED_ALLOCATION(_parameters, _index, _location, _allocation, _offset)       \
    do {                                                                                     \
        struct kefir_abi_amd64_function_parameter param;                                     \
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at((_parameters), (_index), &param)); \
        ASSERT(param.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED);        \
        ASSERT(param.nested.location == (_location));                                        \
        ASSERT(param.nested.allocation == (_allocation));                                    \
        ASSERT(param.nested.offset == (_offset));                                            \
    } while (0)

#define NONE KEFIR_AMD64_SYSV_PARAMETER_LOCATION_NONE
DEFINE_CASE(amd64_sysv_abi_allocation_test1, "AMD64 System V ABI - parameter allocation") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 9, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                          KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GENERIC, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_OK(kefir_abi_amd64_function_parameters_allocate(&kft_mem, &parameters));
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 0,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RDI);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 1,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RSI);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 2,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RDX);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 3,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RCX);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 4,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_R8);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 5,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_R9);
    ASSERT_PARAM_STACK_ALLOCATION(&parameters, 6, 0);
    ASSERT_PARAM_STACK_ALLOCATION(&parameters, 7, 8);
    ASSERT_PARAM_STACK_ALLOCATION(&parameters, 8, 16);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_allocation_test2, "AMD64 System V ABI - parameter allocation #2") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 9, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                          KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GENERIC, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_OK(kefir_abi_amd64_function_parameters_allocate(&kft_mem, &parameters));
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 0,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RDI);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_XMM0);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 2,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RSI);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 3, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_XMM1);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 4,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RDX);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_XMM2);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 6,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RCX);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 7,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_R8);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 8,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_R9);
    ASSERT_PARAM_STACK_ALLOCATION(&parameters, 9, 0);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 10, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_XMM3);
    ASSERT_PARAM_STACK_ALLOCATION(&parameters, 11, 8);
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 12, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_XMM4);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_allocation_test3, "AMD64 System V ABI - parameter allocation #3") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 6, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 4));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 8));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_CHAR, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                          KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GENERIC, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_OK(kefir_abi_amd64_function_parameters_allocate(&kft_mem, &parameters));
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 0,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RDI);
    ASSERT_PARAM_MULTPLE_REGISTER_ALLOCATION(&parameters, 1);
    ASSERT_NESTED_ALLOCATION(&parameters, 2, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1,
                             0);
    ASSERT_NESTED_ALLOCATION(&parameters, 3, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1,
                             4);
    ASSERT_NESTED_ALLOCATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 2,
                             0);
    ASSERT_NESTED_ALLOCATION(&parameters, 6, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 2,
                             1);
    ASSERT_NESTED_ALLOCATION(&parameters, 7, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 2,
                             2);
    ASSERT_NESTED_ALLOCATION(&parameters, 8, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 2,
                             3);
    ASSERT_NESTED_ALLOCATION(&parameters, 9, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 2,
                             4);
    ASSERT_NESTED_ALLOCATION(&parameters, 10, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 2,
                             5);
    ASSERT_NESTED_ALLOCATION(&parameters, 11, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 2,
                             6);
    ASSERT_NESTED_ALLOCATION(&parameters, 12, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 2,
                             7);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_allocation_test4, "AMD64 System V ABI - parameter allocation #4") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 4, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 4));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                          KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GENERIC, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_OK(kefir_abi_amd64_function_parameters_allocate(&kft_mem, &parameters));
    ASSERT_PARAM_REGISTER_ALLOCATION(&parameters, 0,
                                     KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
                                     KEFIR_AMD64_XASMGEN_REGISTER_RDI);
    ASSERT_PARAM_MULTPLE_REGISTER_ALLOCATION(&parameters, 1);
    ASSERT_PARAM_NONE_ALLOCATION(&parameters, 2);
    ASSERT_NESTED_ALLOCATION(&parameters, 3, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 0);
    ASSERT_NESTED_ALLOCATION(&parameters, 4, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 4);
    ASSERT_NESTED_ALLOCATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 1, 0);
    ASSERT_NESTED_ALLOCATION(&parameters, 6, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 1, 4);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_allocation_test5, "AMD64 System V ABI - parameter allocation #5") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 6, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 1, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 1, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                          KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GENERIC, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_OK(kefir_abi_amd64_function_parameters_allocate(&kft_mem, &parameters));
    ASSERT_PARAM_STACK_ALLOCATION(&parameters, 0, 0);
    ASSERT_PARAM_STACK_ALLOCATION(&parameters, 2, 8);
    ASSERT_PARAM_MULTPLE_REGISTER_ALLOCATION(&parameters, 4);
    ASSERT_NESTED_ALLOCATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0,
                             0);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_allocation_test6, "AMD64 System V ABI - parameter allocation #6") {
    struct kefir_ir_type type1, type2;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 8, &type1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_ARRAY, 0, 10));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_STRUCT, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_STRUCT, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_INT64, 1, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_STRUCT, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 2, &type2));
    ASSERT_OK(kefir_irbuilder_type_append_from(&kft_mem, &type2, &type1, 2));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                          KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GENERIC, &type2, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type2, &layout,
                                                           &parameters));
    ASSERT_OK(kefir_abi_amd64_function_parameters_allocate(&kft_mem, &parameters));
    ASSERT_PARAM_MULTPLE_REGISTER_ALLOCATION(&parameters, 0);
    ASSERT_NESTED_ALLOCATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0,
                             0);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type2));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type1));
}
END_CASE
#undef NONE
