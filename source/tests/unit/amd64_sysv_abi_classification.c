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

#include <stdalign.h>
#include <inttypes.h>
#include "kefir/test/unit_test.h"
#include "kefir/ir/type.h"
#include "kefir/ir/builder.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/target/abi/amd64/parameters.h"
#include "kefir/codegen/system-v-amd64/registers.h"

#define ASSERT_PARAM_CLASSIFICATION(_parameters, _index, _location)                         \
    do {                                                                                    \
        struct kefir_abi_amd64_function_parameter param;                                    \
        ASSERT_OK(kefir_abi_amd64_function_parameters_at((_parameters), (_index), &param)); \
        ASSERT(param.preliminary_classification);                                           \
        ASSERT(param.location == (_location));                                              \
    } while (0)

#define ASSERT_NESTED_PARAM_CLASSIFICATION(_parameters, _index, _location, _nested_index, _nested_offset) \
    do {                                                                                                  \
        struct kefir_abi_amd64_function_parameter param;                                                  \
        ASSERT_OK(kefir_abi_amd64_function_parameters_at((_parameters), (_index), &param));               \
        ASSERT(param.preliminary_classification);                                                         \
        ASSERT(param.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED);                     \
        ASSERT(param.nested.location == (_location));                                                     \
        ASSERT(param.nested.index == (_nested_index));                                                    \
        ASSERT(param.nested.offset == (_nested_offset));                                                  \
    } while (0)

#define ASSERT_PARAM_REQUIREMENTS(_parameters, _index, _gp, _sse, _sseup, _memsize, _memalign) \
    do {                                                                                       \
        struct kefir_abi_amd64_function_parameter param;                                       \
        ASSERT_OK(kefir_abi_amd64_function_parameters_at((_parameters), (_index), &param));    \
        ASSERT(param.requirements.general_purpose_regs == (_gp));                              \
        ASSERT(param.requirements.sse_regs == (_sse));                                         \
        ASSERT(param.requirements.sseup_regs == (_sseup));                                     \
        ASSERT(param.requirements.memory_size == (_memsize));                                  \
        ASSERT(param.requirements.memory_alignment == (_memalign));                            \
    } while (0)

DEFINE_CASE(amd64_sysv_abi_classification_test1, "AMD64 System V ABI - parameter classification") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 5, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 2,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 3,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 4);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 4, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test2, "AMD64 System V ABI - parameter classification #2") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 5, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 2, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 3, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 4, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 4);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test3, "AMD64 System V ABI - parameter classification #3") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 6, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 3));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 1,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 2, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 3,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 4);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 4, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 1, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 1, 4);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test4, "AMD64 System V ABI - parameter classification #4") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 10, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 9));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test5, "AMD64 System V ABI - parameter classification #5") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 4, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 3));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 1, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test6, "AMD64 System V ABI - parameter classification #6") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 11, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 5));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 4));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 1,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 2,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 2);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 3,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 4);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 4,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 0);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 6,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 4);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 7,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 5);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 8,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 6);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 9,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 7);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 10, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test7, "AMD64 System V ABI - parameter classification #7") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 11, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 5));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 4));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 10, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test8, "AMD64 System V ABI - parameter classification #8") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 4, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 4));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 1,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 2,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 1);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 3,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 2);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 4,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 3);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 6, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 7, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 4);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test9, "AMD64 System V ABI - parameter classification #9") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 9, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 3));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_CHAR, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 2, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 3, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 4,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 5,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 1);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 6, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 7,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 2);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 8,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 4);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 9, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 10,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 6);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 11,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 7);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 12, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 13, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 14,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 15,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 1);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 16, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 17,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 2);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 18,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 4);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 19, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 20,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 6);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 21,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 7);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test10, "AMD64 System V ABI - parameter classification #10") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 5, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 2, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 3, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 4);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 4, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 5,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 6,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 4);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test11, "AMD64 System V ABI - parameter classification #11") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 5, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 3));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test12, "AMD64 System V ABI - parameter classification #12") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 5, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_UNION, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 3));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 8));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_CHAR, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 2,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 3,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 4);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 4, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 1, 0);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 6,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 7,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 1);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 8,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 2);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 9,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 3);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 10,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 4);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 11,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 5);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 12,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 6);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 13,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 7);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test13, "AMD64 System V ABI - parameter classification #13") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 5, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_UNION, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 8));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_CHAR, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 2,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 3, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 4,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 5,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 1);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 6,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 2);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 7,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 3);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 8,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 4);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 9,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 5);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 10,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 6);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 11,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 0, 7);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 12, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 13,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 0);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 14, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 15,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 16,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 1);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 17,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 2);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 18,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 3);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 19,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 4);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 20,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 5);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 21,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 6);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 22,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 7);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test14, "AMD64 System V ABI - parameter statistics") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 9, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 10));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_CHAR, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_UNION, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_BOOL, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &layout,
                                                           &parameters));
    ASSERT_PARAM_REQUIREMENTS(&parameters, 0, 1, 0, 0, 0, 0);
    ASSERT_PARAM_REQUIREMENTS(&parameters, 1, 0, 0, 0, 24, 8);
    ASSERT_PARAM_REQUIREMENTS(&parameters, 14, 0, 1, 0, 0, 0);
    ASSERT_PARAM_REQUIREMENTS(&parameters, 15, 1, 0, 0, 0, 0);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_classification_test15, "AMD64 System V ABI - external parameter classification") {
    struct kefir_ir_type type1, type2;
    struct kefir_abi_amd64_type_layout layout;
    struct kefir_abi_amd64_function_parameters parameters;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 6, &type1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_INT32, 0, 0));
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 6, &type2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type2, KEFIR_IR_TYPE_ARRAY, 0, 1));
    ASSERT_OK(kefir_irbuilder_type_append_from(&kft_mem, &type2, &type1, 1));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type2, &layout));
    ASSERT_OK(kefir_abi_amd64_function_parameters_classify(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type2, &layout,
                                                           &parameters));
    ASSERT_PARAM_CLASSIFICATION(&parameters, 0, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 1, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 2, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 3, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 4, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER, 0, 4);
    ASSERT_PARAM_CLASSIFICATION(&parameters, 5, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 6,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 0);
    ASSERT_NESTED_PARAM_CLASSIFICATION(&parameters, 7,
                                       KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER, 1, 4);
    ASSERT_OK(kefir_abi_amd64_function_parameters_free(&kft_mem, &parameters));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &layout));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type2));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type1));
}
END_CASE
