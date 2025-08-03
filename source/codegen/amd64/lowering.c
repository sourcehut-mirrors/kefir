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
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/lowering.h"
#include "kefir/target/abi/amd64/base.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define QWORD_BITS (KEFIR_AMD64_ABI_QWORD * 8)

struct lowering_param {
    struct {
        kefir_id_t bigint_set_signed;
        kefir_id_t bigint_set_unsigned;
        kefir_id_t bigint_cast_signed;
        kefir_id_t bigint_cast_unsigned;
        kefir_id_t bigint_signed_to_float;
        kefir_id_t bigint_unsigned_to_float;
        kefir_id_t bigint_signed_to_double;
        kefir_id_t bigint_unsigned_to_double;
        kefir_id_t bigint_signed_to_long_double;
        kefir_id_t bigint_unsigned_to_long_double;
        kefir_id_t bigint_signed_from_float;
        kefir_id_t bigint_unsigned_from_float;
        kefir_id_t bigint_signed_from_double;
        kefir_id_t bigint_unsigned_from_double;
        kefir_id_t bigint_signed_from_long_double;
        kefir_id_t bigint_unsigned_from_long_double;
        kefir_id_t bigint_is_zero;
        kefir_id_t bigint_negate;
        kefir_id_t bigint_invert;
        kefir_id_t bigint_add;
        kefir_id_t bigint_subtract;
        kefir_id_t bigint_signed_multiply;
        kefir_id_t bigint_unsigned_multiply;
        kefir_id_t bigint_signed_divide;
        kefir_id_t bigint_unsigned_divide;
        kefir_id_t bigint_lshift;
        kefir_id_t bigint_rshift;
        kefir_id_t bigint_arshift;
        kefir_id_t bigint_and;
        kefir_id_t bigint_or;
        kefir_id_t bigint_xor;
        kefir_id_t bigint_unsigned_compare;
        kefir_id_t bigint_signed_compare;
        kefir_id_t bigint_least_significant_nonzero;
        kefir_id_t bigint_leading_zeros;
        kefir_id_t bigint_trailing_zeros;

        kefir_id_t builtin_ffs;
        kefir_id_t builtin_ffsl;
        kefir_id_t builtin_clz;
        kefir_id_t builtin_clzl;
        kefir_id_t builtin_ctz;
        kefir_id_t builtin_ctzl;
    } runtime_fn;
};

#define BIGINT_RUNTIME_FN_IDENTIFIER(_name)                                                                \
    (struct kefir_ir_identifier) {                                                                         \
        .symbol = (_name), .type = KEFIR_IR_IDENTIFIER_FUNCTION, .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL, \
        .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT, .alias = NULL, .debug_info = {               \
            .entry = KEFIR_IR_DEBUG_ENTRY_ID_NONE                                                          \
        }                                                                                                  \
    }

#define BUILTIN_RUNTIME_FN_IDENTIFIER(_name)                                                                \
    (struct kefir_ir_identifier) {                                                                          \
        .symbol = (_name), .type = KEFIR_IR_IDENTIFIER_FUNCTION, .scope = KEFIR_IR_IDENTIFIER_SCOPE_IMPORT, \
        .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT, .alias = NULL, .debug_info = {                \
            .entry = KEFIR_IR_DEBUG_ENTRY_ID_NONE                                                           \
        }                                                                                                   \
    }

#define DECL_BIGINT_RUNTIME_FN(_id, _name, _params, _returns, _init)                                               \
    static kefir_result_t get_##_id##_function_decl_id(                                                            \
        struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module, struct kefir_opt_module *module, \
        struct lowering_param *param, kefir_id_t *func_decl_id) {                                                  \
        if (param->runtime_fn._id != KEFIR_ID_NONE) {                                                              \
            *func_decl_id = param->runtime_fn._id;                                                                 \
            return KEFIR_OK;                                                                                       \
        }                                                                                                          \
                                                                                                                   \
        kefir_id_t parameters_type_id, returns_type_id;                                                            \
        struct kefir_ir_type *parameters_type =                                                                    \
            kefir_ir_module_new_type(mem, module->ir_module, (_params), &parameters_type_id);                      \
        struct kefir_ir_type *returns_type =                                                                       \
            kefir_ir_module_new_type(mem, module->ir_module, (_returns), &returns_type_id);                        \
        REQUIRE(parameters_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));   \
        REQUIRE(returns_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));      \
        _init                                                                                                      \
                                                                                                                   \
            struct kefir_ir_function_decl *func_decl = kefir_ir_module_new_function_declaration(                   \
                mem, module->ir_module, (_name), parameters_type_id, false, returns_type_id);                      \
        REQUIRE(func_decl != NULL,                                                                                 \
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR function declaration"));            \
                                                                                                                   \
        REQUIRE_OK(kefir_ir_module_declare_identifier(mem, module->ir_module, func_decl->name,                     \
                                                      &BIGINT_RUNTIME_FN_IDENTIFIER(func_decl->name)));            \
                                                                                                                   \
        REQUIRE_OK(kefir_codegen_amd64_module_require_runtime(mem, codegen_module, func_decl->name));              \
                                                                                                                   \
        param->runtime_fn._id = func_decl->id;                                                                     \
        *func_decl_id = func_decl->id;                                                                             \
        return KEFIR_OK;                                                                                           \
    }

#define DECL_BUILTIN_RUNTIME_FN(_id, _name, _params, _returns, _init)                                            \
    static kefir_result_t get_##_id##_function_decl_id(struct kefir_mem *mem, struct kefir_opt_module *module,   \
                                                       struct lowering_param *param, kefir_id_t *func_decl_id) { \
        if (param->runtime_fn._id != KEFIR_ID_NONE) {                                                            \
            *func_decl_id = param->runtime_fn._id;                                                               \
            return KEFIR_OK;                                                                                     \
        }                                                                                                        \
                                                                                                                 \
        kefir_id_t parameters_type_id, returns_type_id;                                                          \
        struct kefir_ir_type *parameters_type =                                                                  \
            kefir_ir_module_new_type(mem, module->ir_module, (_params), &parameters_type_id);                    \
        struct kefir_ir_type *returns_type =                                                                     \
            kefir_ir_module_new_type(mem, module->ir_module, (_returns), &returns_type_id);                      \
        REQUIRE(parameters_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type")); \
        REQUIRE(returns_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));    \
        _init                                                                                                    \
                                                                                                                 \
            struct kefir_ir_function_decl *func_decl = kefir_ir_module_new_function_declaration(                 \
                mem, module->ir_module, (_name), parameters_type_id, false, returns_type_id);                    \
        REQUIRE(func_decl != NULL,                                                                               \
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR function declaration"));          \
                                                                                                                 \
        REQUIRE_OK(kefir_ir_module_declare_identifier(mem, module->ir_module, func_decl->name,                   \
                                                      &BUILTIN_RUNTIME_FN_IDENTIFIER(func_decl->name)));         \
                                                                                                                 \
        param->runtime_fn._id = func_decl->id;                                                                   \
        *func_decl_id = func_decl->id;                                                                           \
        return KEFIR_OK;                                                                                         \
    }

DECL_BIGINT_RUNTIME_FN(bigint_set_signed, BIGINT_GET_SET_SIGNED_INTEGER_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_set_unsigned, BIGINT_GET_SET_UNSIGNED_INTEGER_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_cast_signed, BIGINT_CAST_SIGNED_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_cast_unsigned, BIGINT_CAST_UNSIGNED_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_to_float, BIGINT_SIGNED_TO_FLOAT_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_to_float, BIGINT_UNSIGNED_TO_FLOAT_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_to_double, BIGINT_SIGNED_TO_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_to_double, BIGINT_UNSIGNED_TO_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_to_long_double, BIGINT_SIGNED_TO_LONG_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_to_long_double, BIGINT_UNSIGNED_TO_LONG_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_from_float, BIGINT_SIGNED_FROM_FLOAT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_from_float, BIGINT_UNSIGNED_FROM_FLOAT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_from_double, BIGINT_SIGNED_FROM_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_from_double, BIGINT_UNSIGNED_FROM_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_from_long_double, BIGINT_SIGNED_FROM_LONG_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_from_long_double, BIGINT_UNSIGNED_FROM_LONG_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_is_zero, BIGINT_IS_ZERO_FN, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT8, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_negate, BIGINT_NEGATE_FN, 2, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_invert, BIGINT_INVERT_FN, 2, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_add, BIGINT_ADD_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_subtract, BIGINT_SUBTRACT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_and, BIGINT_AND_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_or, BIGINT_OR_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_xor, BIGINT_XOR_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_multiply, BIGINT_SIGNED_MULTIPLY_FN, 6, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_multiply, BIGINT_UNSIGNED_MULTIPLY_FN, 5, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_divide, BIGINT_SIGNED_DIVIDE_FN, 5, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_divide, BIGINT_UNSIGNED_DIVIDE_FN, 5, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_lshift, BIGINT_LEFT_SHIFT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_rshift, BIGINT_RIGHT_SHIFT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_arshift, BIGINT_ARITHMETIC_RIGHT_SHIFT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_unsigned_compare, BIGINT_UNSIGNED_COMPARE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT8, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_signed_compare, BIGINT_SIGNED_COMPARE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_least_significant_nonzero, BIGINT_LEAST_SIGNIFICANT_NONZERO_FN, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_leading_zeros, BIGINT_LEADING_ZEROS_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BIGINT_RUNTIME_FN(bigint_trailing_zeros, BIGINT_TRAILING_ZEROS_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_ffs, BUILTIN_FFS_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_ffsl, BUILTIN_FFSL_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_clz, BUILTIN_CLZ_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_clzl, BUILTIN_CLZL_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_ctz, BUILTIN_CTZ_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_ctzl, BUILTIN_CTZL_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})

static kefir_result_t new_bitint_type(struct kefir_mem *mem, struct kefir_opt_module *module, kefir_size_t width,
                                      struct kefir_ir_type **type_ptr, kefir_id_t *type_id_ptr) {
    struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 1, type_id_ptr);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));

    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_BITINT, 0, width));
    ASSIGN_PTR(type_ptr, type);
    return KEFIR_OK;
}

static kefir_result_t new_bitint_low_level_type(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                kefir_size_t width, struct kefir_ir_type **type_ptr,
                                                kefir_id_t *type_id_ptr) {
    struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 3, type_id_ptr);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));

    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_STRUCT, 0, 1));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_ARRAY, 0, (width + 7) / 8));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSIGN_PTR(type_ptr, type);
    return KEFIR_OK;
}

static kefir_result_t lower_instruction(struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module,
                                        struct kefir_opt_module *module, struct kefir_opt_function *func,
                                        struct lowering_param *param, const struct kefir_opt_instruction *instr,
                                        kefir_opt_instruction_ref_t *replacement_ref) {
    UNUSED(mem);
    UNUSED(module);
    UNUSED(func);
    UNUSED(instr);
    UNUSED(replacement_ref);

    const kefir_opt_block_id_t block_id = instr->block_id;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST: {
            const struct kefir_bigint *bigint;
            REQUIRE_OK(
                kefir_ir_module_get_bigint(module->ir_module, instr->operation.parameters.imm.bitint_ref, &bigint));
            if (bigint->bitwidth <= QWORD_BITS) {
                kefir_int64_t value;
                REQUIRE_OK(kefir_bigint_get_signed(bigint, &value));
                REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, value, replacement_ref));
            } else {
                // Intentionally left blank
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST: {
            const struct kefir_bigint *bigint;
            REQUIRE_OK(
                kefir_ir_module_get_bigint(module->ir_module, instr->operation.parameters.imm.bitint_ref, &bigint));
            if (bigint->bitwidth <= QWORD_BITS) {
                kefir_uint64_t value;
                REQUIRE_OK(kefir_bigint_get_unsigned(bigint, &value));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, value, replacement_ref));
            } else {
                // Intentionally left blank
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_GET_SIGNED:
            if (instr->operation.parameters.bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0], 0,
                    instr->operation.parameters.bitwidth, replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int64_load(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0],
                    &(struct kefir_opt_memory_access_flags) {.load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND,
                                                             .volatile_access = false},
                    replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_GET_UNSIGNED:
            if (instr->operation.parameters.bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0], 0,
                    instr->operation.parameters.bitwidth, replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int64_load(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0],
                    &(struct kefir_opt_memory_access_flags) {.load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND,
                                                             .volatile_access = false},
                    replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_FROM_SIGNED:
            if (instr->operation.parameters.bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0], 0,
                    instr->operation.parameters.bitwidth, replacement_ref));
            } else {
                const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
                const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_set_signed_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_instruction_ref_t call_node_id, call_ref, value_ref, bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_FROM_UNSIGNED:
            if (instr->operation.parameters.bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(
                    mem, &func->code, block_id, instr->operation.parameters.refs[0], 0,
                    instr->operation.parameters.bitwidth, replacement_ref));
            } else {
                const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
                const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_set_unsigned_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_instruction_ref_t call_node_id, call_ref, value_ref, bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_CAST_SIGNED:
        case KEFIR_OPT_OPCODE_BITINT_CAST_UNSIGNED: {
            const kefir_bool_t signed_cast = instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_CAST_SIGNED;

            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
            const kefir_size_t src_bitwidth = instr->operation.parameters.src_bitwidth;
            if (bitwidth <= QWORD_BITS && src_bitwidth <= QWORD_BITS) {
                if (signed_cast) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(
                        mem, &func->code, block_id, arg_ref, 0, MIN(bitwidth, src_bitwidth), replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(
                        mem, &func->code, block_id, arg_ref, 0, MIN(bitwidth, src_bitwidth), replacement_ref));
                }
            } else if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t value_ref;
                REQUIRE_OK(kefir_opt_code_builder_int64_load(
                    mem, &func->code, block_id, arg_ref,
                    &(struct kefir_opt_memory_access_flags) {.load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND,
                                                             .volatile_access = false},
                    &value_ref));
                if (signed_cast) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, value_ref, 0,
                                                                          bitwidth, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, value_ref, 0,
                                                                            bitwidth, replacement_ref));
                }
            } else if (src_bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t casted_arg_ref;
                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                if (signed_cast) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                          src_bitwidth, &casted_arg_ref));
                    REQUIRE_OK(
                        get_bigint_set_signed_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            src_bitwidth, &casted_arg_ref));
                    REQUIRE_OK(
                        get_bigint_set_unsigned_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                }

                kefir_opt_instruction_ref_t call_node_id, call_ref, value_ref, bitwidth_ref;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, casted_arg_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            } else {
                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                if (signed_cast) {
                    REQUIRE_OK(
                        get_bigint_cast_signed_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                } else {
                    REQUIRE_OK(
                        get_bigint_cast_unsigned_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                }

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, MIN(bitwidth, src_bitwidth), NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t call_node_id, call_ref, value_ref, bitwidth_ref, src_bitwidth_ref,
                    init_value_ref, init_value_pair_ref;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, src_bitwidth, &src_bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_ref, arg_ref,
                                                              bitint_type_id, 0, &init_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, init_value_ref,
                                                       &init_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, init_value_pair_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, src_bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT:
        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT:
        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE:
        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE:
        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t value_ref;
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                              bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_int_to_float32(mem, &func->code, block_id, value_ref,
                                                                         replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                                bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_uint_to_float32(mem, &func->code, block_id, value_ref,
                                                                          replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                              bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_int_to_float64(mem, &func->code, block_id, value_ref,
                                                                         replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                                bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_uint_to_float64(mem, &func->code, block_id, value_ref,
                                                                          replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                              bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_int_to_long_double(mem, &func->code, block_id, value_ref,
                                                                             replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                                bitwidth, &value_ref));
                        REQUIRE_OK(kefir_opt_code_builder_uint_to_long_double(mem, &func->code, block_id, value_ref,
                                                                              replacement_ref));
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction optimizer opcode");
                }
            } else {
                kefir_opt_instruction_ref_t value_copy_ref, tmp_ref, init_value_copy_ref, init_value_copy_pair_ref,
                    bitwidth_ref;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &value_copy_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &tmp_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_copy_ref, arg_ref,
                                                              bitint_type_id, 0, &init_value_copy_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_copy_ref, init_value_copy_ref,
                                                       &init_value_copy_pair_ref));

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT:
                        REQUIRE_OK(get_bigint_signed_to_float_function_decl_id(mem, codegen_module, module, param,
                                                                               &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT:
                        REQUIRE_OK(get_bigint_unsigned_to_float_function_decl_id(mem, codegen_module, module, param,
                                                                                 &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE:
                        REQUIRE_OK(get_bigint_signed_to_double_function_decl_id(mem, codegen_module, module, param,
                                                                                &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE:
                        REQUIRE_OK(get_bigint_unsigned_to_double_function_decl_id(mem, codegen_module, module, param,
                                                                                  &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(get_bigint_signed_to_long_double_function_decl_id(mem, codegen_module, module, param,
                                                                                     &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(get_bigint_unsigned_to_long_double_function_decl_id(mem, codegen_module, module,
                                                                                       param, &func_decl_id));
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction optimizer opcode");
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0,
                                                                      init_value_copy_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, tmp_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED:
        case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED:
        case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED:
        case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_float32_to_int(mem, &func->code, block_id, arg_ref,
                                                                         replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_float32_to_uint(mem, &func->code, block_id, arg_ref,
                                                                          replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_float64_to_int(mem, &func->code, block_id, arg_ref,
                                                                         replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_float64_to_uint(mem, &func->code, block_id, arg_ref,
                                                                          replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_long_double_to_int(mem, &func->code, block_id, arg_ref,
                                                                             replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(kefir_opt_code_builder_long_double_to_uint(mem, &func->code, block_id, arg_ref,
                                                                              replacement_ref));
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction optimizer opcode");
                }
            } else {
                kefir_opt_instruction_ref_t value_ref, bitwidth_ref, call_ref;
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED:
                        REQUIRE_OK(get_bigint_signed_from_float_function_decl_id(mem, codegen_module, module, param,
                                                                                 &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(get_bigint_unsigned_from_float_function_decl_id(mem, codegen_module, module, param,
                                                                                   &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(get_bigint_signed_from_double_function_decl_id(mem, codegen_module, module, param,
                                                                                  &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(get_bigint_unsigned_from_double_function_decl_id(mem, codegen_module, module, param,
                                                                                    &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(get_bigint_signed_from_long_double_function_decl_id(mem, codegen_module, module,
                                                                                       param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(get_bigint_unsigned_from_long_double_function_decl_id(mem, codegen_module, module,
                                                                                         param, &func_decl_id));
                        break;

                    default:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction optimizer opcode");
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_TO_BOOL: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            kefir_opt_instruction_ref_t value_ref;
            if (bitwidth <= 8) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_int8_to_bool(mem, &func->code, block_id, value_ref, replacement_ref));
            } else if (bitwidth <= 16) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_int16_to_bool(mem, &func->code, block_id, value_ref, replacement_ref));
            } else if (bitwidth <= 32) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_int32_to_bool(mem, &func->code, block_id, value_ref, replacement_ref));
            } else if (bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_int64_to_bool(mem, &func->code, block_id, value_ref, replacement_ref));
            } else {
                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_is_zero_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_call_id_t call_node_id;
                kefir_opt_instruction_ref_t call_ref, bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_int8_bool_not(mem, &func->code, block_id, call_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_LOAD: {
            const kefir_opt_instruction_ref_t instr_id = instr->id;
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            struct kefir_opt_memory_access_flags memflags = instr->operation.parameters.bitint_memflags;
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                const struct kefir_opt_memory_access_flags load_memflags = {
                    .load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND, .volatile_access = memflags.volatile_access};
                kefir_opt_instruction_ref_t value_ref;
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                &value_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &value_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &value_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_int64_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &value_ref));
                }

                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, value_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));

                switch (memflags.load_extension) {
                    case KEFIR_OPT_MEMORY_LOAD_NOEXTEND:
                        *replacement_ref = value_ref;
                        break;

                    case KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, value_ref, 0,
                                                                              bitwidth, replacement_ref));
                        break;

                    case KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, value_ref,
                                                                                0, bitwidth, replacement_ref));
                        break;
                }
            } else {
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t value_ref, copy_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_ref, arg_ref,
                                                              bitint_type_id, 0, &copy_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, copy_ref, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, copy_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE: {
            const kefir_opt_instruction_ref_t instr_id = instr->id;
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            struct kefir_opt_memory_access_flags memflags = instr->operation.parameters.bitint_memflags;
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_bool_t update_control_flow = true;
                const struct kefir_opt_memory_access_flags load_memflags = {
                    .load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND, .volatile_access = memflags.volatile_access};
                kefir_opt_instruction_ref_t value_ref;
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                &value_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &value_ref));
                } else if (bitwidth <= 24) {
                    kefir_opt_instruction_ref_t part1_ref, part2_ref, location2_offset_ref, location2_ref, mask_ref,
                        masked_ref, shift_ref, shifted_ref;

                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2, &location2_offset_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, arg_ref,
                                                                location2_offset_ref, &location2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0xffffull, &mask_ref));
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 16, &shift_ref));

                    REQUIRE_OK(kefir_opt_code_builder_int16_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &part1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int8_load(mem, &func->code, block_id, location2_ref,
                                                                &load_memflags, &part2_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_int64_and(mem, &func->code, block_id, part1_ref, mask_ref, &masked_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_lshift(mem, &func->code, block_id, part2_ref, shift_ref,
                                                                   &shifted_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_or(mem, &func->code, block_id, masked_ref, shifted_ref,
                                                               &value_ref));

                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part1_ref));
                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part2_ref));
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));

                    update_control_flow = false;
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &value_ref));
                } else if (bitwidth <= 40) {
                    kefir_opt_instruction_ref_t part1_ref, part2_ref, location2_offset_ref, location2_ref, mask_ref,
                        masked_ref, shift_ref, shifted_ref;

                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 4, &location2_offset_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, arg_ref,
                                                                location2_offset_ref, &location2_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0xffffffffull, &mask_ref));
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 32, &shift_ref));

                    REQUIRE_OK(kefir_opt_code_builder_int32_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &part1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int8_load(mem, &func->code, block_id, location2_ref,
                                                                &load_memflags, &part2_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_int64_and(mem, &func->code, block_id, part1_ref, mask_ref, &masked_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_lshift(mem, &func->code, block_id, part2_ref, shift_ref,
                                                                   &shifted_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_or(mem, &func->code, block_id, masked_ref, shifted_ref,
                                                               &value_ref));

                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part1_ref));
                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part2_ref));
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));

                    update_control_flow = false;
                } else if (bitwidth <= 48) {
                    kefir_opt_instruction_ref_t part1_ref, part2_ref, location2_offset_ref, location2_ref, mask_ref,
                        masked_ref, shift_ref, shifted_ref;

                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 4, &location2_offset_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, arg_ref,
                                                                location2_offset_ref, &location2_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0xffffffffull, &mask_ref));
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 32, &shift_ref));

                    REQUIRE_OK(kefir_opt_code_builder_int32_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &part1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int16_load(mem, &func->code, block_id, location2_ref,
                                                                 &load_memflags, &part2_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_int64_and(mem, &func->code, block_id, part1_ref, mask_ref, &masked_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_lshift(mem, &func->code, block_id, part2_ref, shift_ref,
                                                                   &shifted_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_or(mem, &func->code, block_id, masked_ref, shifted_ref,
                                                               &value_ref));

                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part1_ref));
                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part2_ref));
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));

                    update_control_flow = false;
                } else if (bitwidth <= 56) {
                    kefir_opt_instruction_ref_t part1_ref, part2_ref, part3_ref, location2_offset_ref,
                        location3_offset_ref, location2_ref, location3_ref, mask_ref, mask2_ref, masked_ref,
                        masked2_ref, shift_ref, shift2_ref, shifted_ref, shifted2_ref, part12_ref;

                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 4, &location2_offset_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 6, &location3_offset_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, arg_ref,
                                                                location2_offset_ref, &location2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, arg_ref,
                                                                location3_offset_ref, &location3_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0xffffffffull, &mask_ref));
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0xffffull, &mask2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 32, &shift_ref));
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 48, &shift2_ref));

                    REQUIRE_OK(kefir_opt_code_builder_int32_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &part1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int16_load(mem, &func->code, block_id, location2_ref,
                                                                 &load_memflags, &part2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int8_load(mem, &func->code, block_id, location3_ref,
                                                                &load_memflags, &part3_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_int64_and(mem, &func->code, block_id, part1_ref, mask_ref, &masked_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_and(mem, &func->code, block_id, part2_ref, mask2_ref,
                                                                &masked2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_lshift(mem, &func->code, block_id, masked2_ref, shift_ref,
                                                                   &shifted_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_lshift(mem, &func->code, block_id, part3_ref, shift2_ref,
                                                                   &shifted2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_or(mem, &func->code, block_id, masked_ref, shifted_ref,
                                                               &part12_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_or(mem, &func->code, block_id, part12_ref, shifted2_ref,
                                                               &value_ref));

                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part1_ref));
                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part2_ref));
                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, part3_ref));
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));

                    update_control_flow = false;
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_int64_load(mem, &func->code, block_id, arg_ref, &load_memflags,
                                                                 &value_ref));
                }

                if (update_control_flow) {
                    REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, value_ref));
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                }

                switch (memflags.load_extension) {
                    case KEFIR_OPT_MEMORY_LOAD_NOEXTEND:
                        *replacement_ref = value_ref;
                        break;

                    case KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, value_ref, 0,
                                                                              bitwidth, replacement_ref));
                        break;

                    case KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, value_ref,
                                                                                0, bitwidth, replacement_ref));
                        break;
                }
            } else {
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t value_ref, copy_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_ref, arg_ref,
                                                              bitint_type_id, 0, &copy_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, copy_ref, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, copy_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_STORE: {
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t value_arg_ref = instr->operation.parameters.refs[1];
            struct kefir_opt_memory_access_flags memflags = instr->operation.parameters.bitint_memflags;
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_store(mem, &func->code, block_id, location_arg_ref,
                                                                 value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_int64_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, replacement_ref));
                }
            } else {
                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, location_arg_ref,
                                                              value_arg_ref, bitint_type_id, 0, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE: {
            const kefir_opt_instruction_ref_t original_instr_ref = instr->id;
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t value_arg_ref = instr->operation.parameters.refs[1];
            struct kefir_opt_memory_access_flags memflags = instr->operation.parameters.bitint_memflags;
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_store(mem, &func->code, block_id, location_arg_ref,
                                                                 value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= 24) {
                    kefir_opt_instruction_ref_t shift_const1_ref, store1_ref, shift1_ref, location1_const_ref,
                        location1_ref;
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 16, &shift_const1_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2, &location1_const_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_rshift(mem, &func->code, block_id, value_arg_ref,
                                                                   shift_const1_ref, &shift1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, location_arg_ref,
                                                                location1_const_ref, &location1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int16_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, &store1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int8_store(mem, &func->code, block_id, location1_ref, shift1_ref,
                                                                 &memflags, replacement_ref));
                    REQUIRE_OK(
                        kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store1_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= 40) {
                    kefir_opt_instruction_ref_t shift_const1_ref, store1_ref, shift1_ref, location1_const_ref,
                        location1_ref;
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 32, &shift_const1_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 4, &location1_const_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_rshift(mem, &func->code, block_id, value_arg_ref,
                                                                   shift_const1_ref, &shift1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, location_arg_ref,
                                                                location1_const_ref, &location1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int32_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, &store1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int8_store(mem, &func->code, block_id, location1_ref, shift1_ref,
                                                                 &memflags, replacement_ref));
                    REQUIRE_OK(
                        kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store1_ref));
                } else if (bitwidth <= 48) {
                    kefir_opt_instruction_ref_t shift_const1_ref, store1_ref, shift1_ref, location1_const_ref,
                        location1_ref;
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 32, &shift_const1_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 4, &location1_const_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_rshift(mem, &func->code, block_id, value_arg_ref,
                                                                   shift_const1_ref, &shift1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, location_arg_ref,
                                                                location1_const_ref, &location1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int32_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, &store1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int16_store(mem, &func->code, block_id, location1_ref, shift1_ref,
                                                                  &memflags, replacement_ref));
                    REQUIRE_OK(
                        kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store1_ref));
                } else if (bitwidth <= 56) {
                    kefir_opt_instruction_ref_t shift_const1_ref, shift_const2_ref, store1_ref, shift1_ref, shift2_ref,
                        location1_const_ref, location2_const_ref, location1_ref, location2_ref, store2_ref;
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 32, &shift_const1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 16, &shift_const2_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 4, &location1_const_ref));
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2, &location2_const_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_rshift(mem, &func->code, block_id, value_arg_ref,
                                                                   shift_const1_ref, &shift1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_rshift(mem, &func->code, block_id, shift1_ref,
                                                                   shift_const2_ref, &shift2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, location_arg_ref,
                                                                location1_const_ref, &location1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, location1_ref,
                                                                location2_const_ref, &location2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int32_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, &store1_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int16_store(mem, &func->code, block_id, location1_ref, shift1_ref,
                                                                  &memflags, &store2_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int8_store(mem, &func->code, block_id, location2_ref, shift2_ref,
                                                                 &memflags, replacement_ref));

                    REQUIRE_OK(
                        kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store1_ref));
                    REQUIRE_OK(
                        kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store2_ref));
                } else if (bitwidth <= QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_int64_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, replacement_ref));
                }
            } else {
                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_low_level_type(mem, module, bitwidth, NULL, &bitint_type_id));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, location_arg_ref,
                                                              value_arg_ref, bitint_type_id, 0, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_LOAD: {
            const kefir_opt_instruction_ref_t instr_id = instr->id;
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            struct kefir_opt_memory_access_flags memflags = instr->operation.parameters.bitint_memflags;
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.bitint_atomic_memorder;
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t value_ref;
                if (bitwidth <= 8) {
                    REQUIRE_OK(
                        kefir_opt_code_builder_atomic_load8(mem, &func->code, block_id, arg_ref, memorder, &value_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_load16(mem, &func->code, block_id, arg_ref, memorder,
                                                                    &value_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_load32(mem, &func->code, block_id, arg_ref, memorder,
                                                                    &value_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_load64(mem, &func->code, block_id, arg_ref, memorder,
                                                                    &value_ref));
                }

                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, value_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));

                switch (memflags.load_extension) {
                    case KEFIR_OPT_MEMORY_LOAD_NOEXTEND:
                        *replacement_ref = value_ref;
                        break;

                    case KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, value_ref, 0,
                                                                              bitwidth, replacement_ref));
                        break;

                    case KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND:
                        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, value_ref,
                                                                                0, bitwidth, replacement_ref));
                        break;
                }
            } else {
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t value_ref, copy_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_atomic_copy_memory_from(
                    mem, &func->code, block_id, value_ref, arg_ref, memorder, bitint_type_id, 0, &copy_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, copy_ref, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, copy_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_STORE: {
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t value_arg_ref = instr->operation.parameters.refs[1];
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.bitint_atomic_memorder;
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_store8(mem, &func->code, block_id, location_arg_ref,
                                                                    value_arg_ref, memorder, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_store16(mem, &func->code, block_id, location_arg_ref,
                                                                     value_arg_ref, memorder, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_store32(mem, &func->code, block_id, location_arg_ref,
                                                                     value_arg_ref, memorder, replacement_ref));
                } else if (bitwidth <= QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_store64(mem, &func->code, block_id, location_arg_ref,
                                                                     value_arg_ref, memorder, replacement_ref));
                }
            } else {
                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));
                REQUIRE_OK(kefir_opt_code_builder_atomic_copy_memory_to(mem, &func->code, block_id, location_arg_ref,
                                                                        value_arg_ref, memorder, bitint_type_id, 0,
                                                                        replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_COMPARE_EXCHANGE: {
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t compare_value_arg_ref = instr->operation.parameters.refs[1];
            const kefir_opt_instruction_ref_t new_value_arg_ref = instr->operation.parameters.refs[2];
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.bitint_atomic_memorder;
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange8(
                        mem, &func->code, block_id, location_arg_ref, compare_value_arg_ref, new_value_arg_ref,
                        memorder, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange16(
                        mem, &func->code, block_id, location_arg_ref, compare_value_arg_ref, new_value_arg_ref,
                        memorder, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange32(
                        mem, &func->code, block_id, location_arg_ref, compare_value_arg_ref, new_value_arg_ref,
                        memorder, replacement_ref));
                } else if (bitwidth <= QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange64(
                        mem, &func->code, block_id, location_arg_ref, compare_value_arg_ref, new_value_arg_ref,
                        memorder, replacement_ref));
                }
            } else {
                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));
                REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange_memory(
                    mem, &func->code, block_id, location_arg_ref, compare_value_arg_ref, new_value_arg_ref, memorder,
                    bitint_type_id, 0, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_NEGATE: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_neg(mem, &func->code, block_id, arg_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_neg(mem, &func->code, block_id, arg_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_neg(mem, &func->code, block_id, arg_ref, replacement_ref));
                } else if (bitwidth <= QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_int64_neg(mem, &func->code, block_id, arg_ref, replacement_ref));
                }
            } else {
                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_opt_instruction_ref_t value_ref, init_value_ref, init_value_pair_ref, bitwidth_ref, call_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_ref, arg_ref,
                                                              bitint_type_id, 0, &init_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, init_value_ref,
                                                       &init_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_negate_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, init_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));

                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_INVERT: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_not(mem, &func->code, block_id, arg_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_not(mem, &func->code, block_id, arg_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_not(mem, &func->code, block_id, arg_ref, replacement_ref));
                } else if (bitwidth <= QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_int64_not(mem, &func->code, block_id, arg_ref, replacement_ref));
                }
            } else {
                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_opt_instruction_ref_t value_ref, init_value_ref, init_value_pair_ref, bitwidth_ref, call_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_ref, arg_ref,
                                                              bitint_type_id, 0, &init_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, init_value_ref,
                                                       &init_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_invert_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, init_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));

                REQUIRE_OK(
                    kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_BOOL_NOT: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t value_ref;
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                        bitwidth, &value_ref));
                if (bitwidth <= 8) {
                    REQUIRE_OK(
                        kefir_opt_code_builder_int8_bool_not(mem, &func->code, block_id, value_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(
                        kefir_opt_code_builder_int16_bool_not(mem, &func->code, block_id, value_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(
                        kefir_opt_code_builder_int32_bool_not(mem, &func->code, block_id, value_ref, replacement_ref));
                } else if (bitwidth <= QWORD_BITS) {
                    REQUIRE_OK(
                        kefir_opt_code_builder_int64_bool_not(mem, &func->code, block_id, value_ref, replacement_ref));
                }
            } else {
                kefir_opt_instruction_ref_t bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_is_zero_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, KEFIR_ID_NONE,
                                                             &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
            }
        } break;

#define BINOP(_suffix, _fn)                                                                                            \
    {                                                                                                                  \
        const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];                              \
        const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];                              \
        const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;                                            \
                                                                                                                       \
        if (bitwidth <= QWORD_BITS) {                                                                                  \
            if (bitwidth <= 8) {                                                                                       \
                REQUIRE_OK(kefir_opt_code_builder_int8_##_suffix(mem, &func->code, block_id, arg1_ref, arg2_ref,       \
                                                                 replacement_ref));                                    \
            } else if (bitwidth <= 16) {                                                                               \
                REQUIRE_OK(kefir_opt_code_builder_int16_##_suffix(mem, &func->code, block_id, arg1_ref, arg2_ref,      \
                                                                  replacement_ref));                                   \
            } else if (bitwidth <= 32) {                                                                               \
                REQUIRE_OK(kefir_opt_code_builder_int32_##_suffix(mem, &func->code, block_id, arg1_ref, arg2_ref,      \
                                                                  replacement_ref));                                   \
            } else if (bitwidth <= QWORD_BITS) {                                                                       \
                REQUIRE_OK(kefir_opt_code_builder_int64_##_suffix(mem, &func->code, block_id, arg1_ref, arg2_ref,      \
                                                                  replacement_ref));                                   \
            }                                                                                                          \
        } else {                                                                                                       \
            const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;                                      \
                                                                                                                       \
            kefir_id_t func_decl_id = KEFIR_ID_NONE;                                                                   \
            REQUIRE_OK(_fn(mem, codegen_module, module, param, &func_decl_id));                                        \
                                                                                                                       \
            kefir_id_t bitint_type_id;                                                                                 \
            REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));                                 \
                                                                                                                       \
            kefir_opt_instruction_ref_t bitwidth_ref, value_ref, init_value_ref, init_value_pair_ref, call_ref;        \
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(                                                        \
                mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref));       \
            REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, value_ref, arg1_ref,             \
                                                          bitint_type_id, 0, &init_value_ref));                        \
            REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, init_value_ref,              \
                                                   &init_value_pair_ref));                                             \
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));     \
                                                                                                                       \
            kefir_opt_call_id_t call_node_id;                                                                          \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,   \
                                                         &call_node_id, &call_ref));                                   \
            REQUIRE_OK(                                                                                                \
                kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, init_value_pair_ref));   \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg2_ref));       \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));   \
                                                                                                                       \
            REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, replacement_ref)); \
        }                                                                                                              \
    }

        case KEFIR_OPT_OPCODE_BITINT_ADD:
            BINOP(add, get_bigint_add_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_BITINT_SUB:
            BINOP(sub, get_bigint_subtract_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_BITINT_AND:
            BINOP(and, get_bigint_and_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_BITINT_OR:
            BINOP(or, get_bigint_or_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_BITINT_XOR:
            BINOP(xor, get_bigint_xor_function_decl_id);
            break;

#undef BINOP

        case KEFIR_OPT_OPCODE_BITINT_UMUL: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t arg1_value_ref, arg2_value_ref;
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg1_ref, 0,
                                                                        bitwidth, &arg1_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg2_ref, 0,
                                                                        bitwidth, &arg2_value_ref));

                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_uint8_mul(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_uint16_mul(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_uint32_mul(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_uint64_mul(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                }
            } else {
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(
                    get_bigint_unsigned_multiply_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t result_value_ref, bitwidth_ref, call_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &result_value_ref));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 5, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, result_value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg1_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 4, bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref, call_ref,
                                                       replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_IMUL: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t arg1_value_ref, arg2_value_ref;
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg1_ref, 0, bitwidth,
                                                                      &arg1_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg2_ref, 0, bitwidth,
                                                                      &arg2_value_ref));

                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_mul(mem, &func->code, block_id, arg1_value_ref,
                                                               arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_mul(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_mul(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_int64_mul(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                }
            } else {
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(
                    get_bigint_signed_multiply_function_decl_id(mem, codegen_module, module, param, &func_decl_id));

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t result_value_ref, tmp_value_ref, lhs_value_ref, lhs_init_value_ref,
                    lhs_init_value_pair_ref, bitwidth_ref, call_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &result_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &tmp_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &lhs_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, lhs_value_ref, arg1_ref,
                                                              bitint_type_id, 0, &lhs_init_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, lhs_value_ref, lhs_init_value_ref,
                                                       &lhs_init_value_pair_ref));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 6, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, result_value_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, tmp_value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2,
                                                                      lhs_init_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, arg2_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 4, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 5, bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref, call_ref,
                                                       replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_UDIV: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t arg1_value_ref, arg2_value_ref;
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg1_ref, 0,
                                                                        bitwidth, &arg1_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg2_ref, 0,
                                                                        bitwidth, &arg2_value_ref));

                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_uint8_div(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_uint16_div(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_uint32_div(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_uint64_div(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                }
            } else {
                const kefir_size_t lhs_qwords = (2 * bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t copy_func_decl_id = KEFIR_ID_NONE, func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(
                    get_bigint_unsigned_divide_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                REQUIRE_OK(
                    get_bigint_cast_unsigned_function_decl_id(mem, codegen_module, module, param, &copy_func_decl_id));

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t result_value_ref, init_result_value_ref, init_result_value_pair_ref,
                    remainder_value_ref, lhs_bitwidth_ref, rhs_bitwidth_ref, copy_call_ref, copy_call_pair_ref,
                    call_ref;

                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2 * bitwidth, &lhs_bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &rhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   lhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &result_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   lhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &remainder_value_ref));

                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, result_value_ref, arg1_ref,
                                                              bitint_type_id, 0, &init_result_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref,
                                                       init_result_value_ref, &init_result_value_pair_ref));

                kefir_opt_call_id_t copy_call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, copy_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &copy_call_node_id, &copy_call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 0,
                                                                      init_result_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 1,
                                                                      rhs_bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 2,
                                                                      lhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref, copy_call_ref,
                                                       &copy_call_pair_ref));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 5, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, copy_call_pair_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, remainder_value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, lhs_bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 4, rhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref, call_ref,
                                                       replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_IDIV: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t arg1_value_ref, arg2_value_ref;
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg1_ref, 0, bitwidth,
                                                                      &arg1_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg2_ref, 0, bitwidth,
                                                                      &arg2_value_ref));

                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_div(mem, &func->code, block_id, arg1_value_ref,
                                                               arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_div(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_div(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_int64_div(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                }
            } else {
                const kefir_size_t lhs_qwords = (2 * bitwidth + QWORD_BITS - 1) / QWORD_BITS;
                const kefir_size_t rhs_qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t copy_func_decl_id = KEFIR_ID_NONE, func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(
                    get_bigint_signed_divide_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                REQUIRE_OK(
                    get_bigint_cast_signed_function_decl_id(mem, codegen_module, module, param, &copy_func_decl_id));

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t result_value_ref, init_result_value_ref, init_result_value_pair_ref,
                    remainder_value_ref, rhs_value_ref, init_rhs_value_ref, init_rhs_value_pair, lhs_bitwidth_ref,
                    rhs_bitwidth_ref, copy_call_ref, copy_call_pair_ref, call_ref;

                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2 * bitwidth, &lhs_bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &rhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   lhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &result_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   lhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &remainder_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   rhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &rhs_value_ref));

                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, result_value_ref, arg1_ref,
                                                              bitint_type_id, 0, &init_result_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, rhs_value_ref, arg2_ref,
                                                              bitint_type_id, 0, &init_rhs_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref,
                                                       init_result_value_ref, &init_result_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, rhs_value_ref, init_rhs_value_ref,
                                                       &init_rhs_value_pair));

                kefir_opt_call_id_t copy_call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, copy_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &copy_call_node_id, &copy_call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 0,
                                                                      init_result_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 1,
                                                                      rhs_bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 2,
                                                                      lhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref, copy_call_ref,
                                                       &copy_call_pair_ref));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 5, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, copy_call_pair_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, remainder_value_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, init_rhs_value_pair));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, lhs_bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 4, rhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref, call_ref,
                                                       replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_UMOD: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t arg1_value_ref, arg2_value_ref;
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg1_ref, 0,
                                                                        bitwidth, &arg1_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg2_ref, 0,
                                                                        bitwidth, &arg2_value_ref));

                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_uint8_mod(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_uint16_mod(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_uint32_mod(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_uint64_mod(mem, &func->code, block_id, arg1_value_ref,
                                                                 arg2_value_ref, replacement_ref));
                }
            } else {
                const kefir_size_t lhs_qwords = (2 * bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t copy_func_decl_id = KEFIR_ID_NONE, func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(
                    get_bigint_unsigned_divide_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                REQUIRE_OK(
                    get_bigint_cast_unsigned_function_decl_id(mem, codegen_module, module, param, &copy_func_decl_id));

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t lhs_value_ref, init_lhs_value_ref, init_lhs_value_pair_ref,
                    remainder_value_ref, lhs_bitwidth_ref, rhs_bitwidth_ref, copy_call_ref, copy_call_pair_ref,
                    call_ref;

                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2 * bitwidth, &lhs_bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &rhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   lhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &lhs_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   lhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &remainder_value_ref));

                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, lhs_value_ref, arg1_ref,
                                                              bitint_type_id, 0, &init_lhs_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, lhs_value_ref, init_lhs_value_ref,
                                                       &init_lhs_value_pair_ref));

                kefir_opt_call_id_t copy_call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, copy_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &copy_call_node_id, &copy_call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 0,
                                                                      init_lhs_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 1,
                                                                      rhs_bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 2,
                                                                      lhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, lhs_value_ref, copy_call_ref,
                                                       &copy_call_pair_ref));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 5, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, copy_call_pair_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, remainder_value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, lhs_bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 4, rhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, remainder_value_ref, call_ref,
                                                       replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_IMOD: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t arg1_value_ref, arg2_value_ref;
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg1_ref, 0, bitwidth,
                                                                      &arg1_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg2_ref, 0, bitwidth,
                                                                      &arg2_value_ref));

                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_mod(mem, &func->code, block_id, arg1_value_ref,
                                                               arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_mod(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_mod(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_int64_mod(mem, &func->code, block_id, arg1_value_ref,
                                                                arg2_value_ref, replacement_ref));
                }
            } else {
                const kefir_size_t lhs_qwords = (2 * bitwidth + QWORD_BITS - 1) / QWORD_BITS;
                const kefir_size_t rhs_qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t copy_func_decl_id = KEFIR_ID_NONE, func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(
                    get_bigint_signed_divide_function_decl_id(mem, codegen_module, module, param, &func_decl_id));
                REQUIRE_OK(
                    get_bigint_cast_signed_function_decl_id(mem, codegen_module, module, param, &copy_func_decl_id));

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t lhs_value_ref, init_lhs_value_ref, init_lhs_value_pair_ref,
                    remainder_value_ref, rhs_value_ref, init_rhs_value_ref, init_rhs_value_pair, lhs_bitwidth_ref,
                    rhs_bitwidth_ref, copy_call_ref, copy_call_pair_ref, call_ref;

                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2 * bitwidth, &lhs_bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &rhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   lhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &lhs_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   lhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &remainder_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   rhs_qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &rhs_value_ref));

                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, lhs_value_ref, arg1_ref,
                                                              bitint_type_id, 0, &init_lhs_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, rhs_value_ref, arg2_ref,
                                                              bitint_type_id, 0, &init_rhs_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, lhs_value_ref, init_lhs_value_ref,
                                                       &init_lhs_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, rhs_value_ref, init_rhs_value_ref,
                                                       &init_rhs_value_pair));

                kefir_opt_call_id_t copy_call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, copy_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &copy_call_node_id, &copy_call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 0,
                                                                      init_lhs_value_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 1,
                                                                      rhs_bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, copy_call_node_id, 2,
                                                                      lhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, lhs_value_ref, copy_call_ref,
                                                       &copy_call_pair_ref));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 5, KEFIR_ID_NONE,
                                                             &call_node_id, &call_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, copy_call_pair_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, remainder_value_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, lhs_bitwidth_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 4, rhs_bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, remainder_value_ref, call_ref,
                                                       replacement_ref));
            }
        } break;

#define SHIFT_OP(_id, _fn)                                                                                             \
    do {                                                                                                               \
        const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];                              \
        const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];                              \
        const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;                                            \
                                                                                                                       \
        if (bitwidth <= QWORD_BITS) {                                                                                  \
            kefir_opt_instruction_ref_t value_ref;                                                                     \
            switch (instr->operation.opcode) {                                                                         \
                case KEFIR_OPT_OPCODE_BITINT_LSHIFT:                                                                   \
                    value_ref = arg1_ref;                                                                              \
                    break;                                                                                             \
                                                                                                                       \
                case KEFIR_OPT_OPCODE_BITINT_RSHIFT:                                                                   \
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg1_ref, 0,   \
                                                                            bitwidth, &value_ref));                    \
                    break;                                                                                             \
                                                                                                                       \
                case KEFIR_OPT_OPCODE_BITINT_ARSHIFT:                                                                  \
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg1_ref, 0,     \
                                                                          bitwidth, &value_ref));                      \
                    break;                                                                                             \
                                                                                                                       \
                default:                                                                                               \
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");            \
            }                                                                                                          \
            if (bitwidth <= 8) {                                                                                       \
                REQUIRE_OK(kefir_opt_code_builder_int8_##_id(mem, &func->code, block_id, arg1_ref, arg2_ref,           \
                                                             replacement_ref));                                        \
            } else if (bitwidth <= 16) {                                                                               \
                REQUIRE_OK(kefir_opt_code_builder_int16_##_id(mem, &func->code, block_id, arg1_ref, arg2_ref,          \
                                                              replacement_ref));                                       \
            } else if (bitwidth <= 32) {                                                                               \
                REQUIRE_OK(kefir_opt_code_builder_int32_##_id(mem, &func->code, block_id, arg1_ref, arg2_ref,          \
                                                              replacement_ref));                                       \
            } else {                                                                                                   \
                REQUIRE_OK(kefir_opt_code_builder_int64_##_id(mem, &func->code, block_id, arg1_ref, arg2_ref,          \
                                                              replacement_ref));                                       \
            }                                                                                                          \
        } else {                                                                                                       \
            const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;                                      \
                                                                                                                       \
            kefir_id_t func_decl_id = KEFIR_ID_NONE;                                                                   \
            REQUIRE_OK(_fn(mem, codegen_module, module, param, &func_decl_id));                                        \
                                                                                                                       \
            kefir_id_t bitint_type_id;                                                                                 \
            REQUIRE_OK(new_bitint_type(mem, module, bitwidth, NULL, &bitint_type_id));                                 \
                                                                                                                       \
            kefir_opt_instruction_ref_t result_value_ref, init_result_value_ref, init_result_value_pair_ref,           \
                bitwidth_ref, call_ref;                                                                                \
                                                                                                                       \
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));     \
                                                                                                                       \
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,                             \
                                                               qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD,  \
                                                               &result_value_ref));                                    \
                                                                                                                       \
            REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, result_value_ref, arg1_ref,      \
                                                          bitint_type_id, 0, &init_result_value_ref));                 \
            REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref,                       \
                                                   init_result_value_ref, &init_result_value_pair_ref));               \
                                                                                                                       \
            kefir_opt_call_id_t call_node_id;                                                                          \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,   \
                                                         &call_node_id, &call_ref));                                   \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0,                   \
                                                                  init_result_value_pair_ref));                        \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg2_ref));       \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));   \
                                                                                                                       \
            REQUIRE_OK(                                                                                                \
                kefir_opt_code_builder_pair(mem, &func->code, block_id, result_value_ref, call_ref, replacement_ref)); \
        }                                                                                                              \
    } while (0)

        case KEFIR_OPT_OPCODE_BITINT_LSHIFT:
            SHIFT_OP(lshift, get_bigint_lshift_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_BITINT_RSHIFT:
            SHIFT_OP(rshift, get_bigint_rshift_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_BITINT_ARSHIFT:
            SHIFT_OP(arshift, get_bigint_arshift_function_decl_id);
            break;

#undef SHIFT_OP

#define CMP_OP(_extract, _cmp, _fn, _fn_res)                                                                           \
    do {                                                                                                               \
        const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];                              \
        const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];                              \
        const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;                                            \
                                                                                                                       \
        if (bitwidth <= QWORD_BITS) {                                                                                  \
            kefir_opt_instruction_ref_t arg1_value_ref, arg2_value_ref;                                                \
            REQUIRE_OK(_extract(mem, &func->code, block_id, arg1_ref, 0, bitwidth, &arg1_value_ref));                  \
            REQUIRE_OK(_extract(mem, &func->code, block_id, arg2_ref, 0, bitwidth, &arg2_value_ref));                  \
                                                                                                                       \
            if (bitwidth <= 8) {                                                                                       \
                REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id,                           \
                                                                 KEFIR_OPT_COMPARISON_INT8_##_cmp, arg1_value_ref,     \
                                                                 arg2_value_ref, replacement_ref));                    \
            } else if (bitwidth <= 16) {                                                                               \
                REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id,                           \
                                                                 KEFIR_OPT_COMPARISON_INT16_##_cmp, arg1_value_ref,    \
                                                                 arg2_value_ref, replacement_ref));                    \
            } else if (bitwidth <= 32) {                                                                               \
                REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id,                           \
                                                                 KEFIR_OPT_COMPARISON_INT32_##_cmp, arg1_value_ref,    \
                                                                 arg2_value_ref, replacement_ref));                    \
            } else {                                                                                                   \
                REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id,                           \
                                                                 KEFIR_OPT_COMPARISON_INT64_##_cmp, arg1_value_ref,    \
                                                                 arg2_value_ref, replacement_ref));                    \
            }                                                                                                          \
        } else {                                                                                                       \
            kefir_id_t func_decl_id;                                                                                   \
            REQUIRE_OK(_fn(mem, codegen_module, module, param, &func_decl_id));                                        \
                                                                                                                       \
            kefir_opt_instruction_ref_t bitwidth_ref, call_ref, expected_ref;                                          \
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));     \
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, (_fn_res), &expected_ref));     \
                                                                                                                       \
            kefir_opt_call_id_t call_node_id;                                                                          \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, KEFIR_ID_NONE,   \
                                                         &call_node_id, &call_ref));                                   \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_ref));       \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg2_ref));       \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, bitwidth_ref));   \
                                                                                                                       \
            REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id,                               \
                                                             KEFIR_OPT_COMPARISON_INT8_EQUALS, call_ref, expected_ref, \
                                                             replacement_ref));                                        \
        }                                                                                                              \
    } while (0)

        case KEFIR_OPT_OPCODE_BITINT_EQUAL:
            CMP_OP(kefir_opt_code_builder_bits_extract_unsigned, EQUALS, get_bigint_unsigned_compare_function_decl_id,
                   0);
            break;

        case KEFIR_OPT_OPCODE_BITINT_GREATER:
            CMP_OP(kefir_opt_code_builder_bits_extract_signed, GREATER, get_bigint_signed_compare_function_decl_id, 1);
            break;

        case KEFIR_OPT_OPCODE_BITINT_ABOVE:
            CMP_OP(kefir_opt_code_builder_bits_extract_unsigned, ABOVE, get_bigint_unsigned_compare_function_decl_id,
                   1);
            break;

        case KEFIR_OPT_OPCODE_BITINT_LESS:
            CMP_OP(kefir_opt_code_builder_bits_extract_signed, LESSER, get_bigint_signed_compare_function_decl_id, -1);
            break;

        case KEFIR_OPT_OPCODE_BITINT_BELOW:
            CMP_OP(kefir_opt_code_builder_bits_extract_unsigned, BELOW, get_bigint_unsigned_compare_function_decl_id,
                   -1);
            break;

#undef CMP_OP

        case KEFIR_OPT_OPCODE_BITINT_EXTRACT_SIGNED:
        case KEFIR_OPT_OPCODE_BITINT_EXTRACT_UNSIGNED: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
            const kefir_size_t offset = instr->operation.parameters.bitint_bitfield.offset;
            const kefir_size_t length = instr->operation.parameters.bitint_bitfield.length;
            REQUIRE(
                offset + length <= bitwidth,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Extracted bit-precise integer width exceeds container width"));

            if (bitwidth <= QWORD_BITS) {
                if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_EXTRACT_SIGNED) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg1_ref, offset,
                                                                          length, replacement_ref));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg1_ref,
                                                                            offset, length, replacement_ref));
                }
            } else {
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t shr_func_decl_id = KEFIR_ID_NONE, cast_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_rshift_function_decl_id(mem, codegen_module, module, param, &shr_func_decl_id));
                if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_EXTRACT_SIGNED) {
                    REQUIRE_OK(get_bigint_cast_signed_function_decl_id(mem, codegen_module, module, param,
                                                                       &cast_func_decl_id));
                } else {
                    REQUIRE_OK(get_bigint_cast_unsigned_function_decl_id(mem, codegen_module, module, param,
                                                                         &cast_func_decl_id));
                }

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, offset + length, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t copy_value_ref, init_copy_value_ref, init_copy_value_pair_ref, shr_call_ref,
                    shr_call_ref_pair, cast_call_ref, bitwidth_ref, offset_ref, length_ref;

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &copy_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, copy_value_ref, arg1_ref,
                                                              bitint_type_id, 0, &init_copy_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, copy_value_ref, init_copy_value_ref,
                                                       &init_copy_value_pair_ref));

                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, offset, &offset_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, length, &length_ref));

                if (offset > 0) {
                    kefir_opt_call_id_t shr_call_node_id;
                    REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, shr_func_decl_id, 3,
                                                                 KEFIR_ID_NONE, &shr_call_node_id, &shr_call_ref));
                    REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, shr_call_node_id, 0,
                                                                          init_copy_value_pair_ref));
                    REQUIRE_OK(
                        kefir_opt_code_container_call_set_argument(mem, &func->code, shr_call_node_id, 1, offset_ref));
                    REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, shr_call_node_id, 2,
                                                                          bitwidth_ref));

                    REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, init_copy_value_pair_ref,
                                                           shr_call_ref, &shr_call_ref_pair));
                } else {
                    shr_call_ref_pair = init_copy_value_pair_ref;
                }

                kefir_opt_call_id_t cast_call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, cast_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &cast_call_node_id, &cast_call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, cast_call_node_id, 0,
                                                                      shr_call_ref_pair));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, cast_call_node_id, 1, length_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, cast_call_node_id, 2, bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, shr_call_ref_pair, cast_call_ref,
                                                       replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_INSERT: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
            const kefir_size_t offset = instr->operation.parameters.bitint_bitfield.offset;
            const kefir_size_t length = instr->operation.parameters.bitint_bitfield.length;
            REQUIRE(
                offset + length <= bitwidth,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Extracted bit-precise integer width exceeds container width"));

            if (bitwidth <= QWORD_BITS) {
                REQUIRE_OK(kefir_opt_code_builder_bits_insert(mem, &func->code, block_id, arg1_ref, arg2_ref, offset,
                                                              length, replacement_ref));
            } else {
                const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS;

                kefir_id_t shl_func_decl_id = KEFIR_ID_NONE, cast_func_decl_id = KEFIR_ID_NONE,
                           or_func_decl_id = KEFIR_ID_NONE, and_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_lshift_function_decl_id(mem, codegen_module, module, param, &shl_func_decl_id));
                REQUIRE_OK(
                    get_bigint_cast_unsigned_function_decl_id(mem, codegen_module, module, param, &cast_func_decl_id));
                REQUIRE_OK(get_bigint_or_function_decl_id(mem, codegen_module, module, param, &or_func_decl_id));
                REQUIRE_OK(get_bigint_and_function_decl_id(mem, codegen_module, module, param, &and_func_decl_id));

                kefir_id_t bitint_type_id;
                REQUIRE_OK(new_bitint_type(mem, module, length, NULL, &bitint_type_id));

                kefir_opt_instruction_ref_t copy_value_ref, init_copy_value_ref, init_copy_value_pair_ref,
                    copy_target_ref, init_copy_target_ref, init_copy_target_pair_ref, shl_call_ref, shl_call_ref_pair,
                    cast_call_ref, cast_call_pair_ref, or_call_ref, and_call_ref, and_call_pair_ref, mask_ref,
                    bitwidth_ref, offset_ref, length_ref;

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &copy_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, copy_value_ref, arg2_ref,
                                                              bitint_type_id, 0, &init_copy_value_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, copy_value_ref, init_copy_value_ref,
                                                       &init_copy_value_pair_ref));

                REQUIRE_OK(kefir_opt_code_builder_temporary_object(mem, &func->code, block_id,
                                                                   qwords * KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_AMD64_ABI_QWORD, &copy_target_ref));
                REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, &func->code, block_id, copy_target_ref, arg1_ref,
                                                              bitint_type_id, 0, &init_copy_target_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, copy_target_ref,
                                                       init_copy_target_ref, &init_copy_target_pair_ref));

                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, offset, &offset_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, length, &length_ref));

                kefir_opt_call_id_t cast_call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, cast_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &cast_call_node_id, &cast_call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, cast_call_node_id, 0,
                                                                      init_copy_value_pair_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, cast_call_node_id, 1, length_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, cast_call_node_id, 2, bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, init_copy_value_pair_ref,
                                                       cast_call_ref, &cast_call_pair_ref));

                if (offset > 0) {
                    kefir_opt_call_id_t shr_call_node_id;
                    REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, shl_func_decl_id, 3,
                                                                 KEFIR_ID_NONE, &shr_call_node_id, &shl_call_ref));
                    REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, shr_call_node_id, 0,
                                                                          cast_call_pair_ref));
                    REQUIRE_OK(
                        kefir_opt_code_container_call_set_argument(mem, &func->code, shr_call_node_id, 1, offset_ref));
                    REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, shr_call_node_id, 2,
                                                                          bitwidth_ref));

                    REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, init_copy_value_pair_ref,
                                                           shl_call_ref, &shl_call_ref_pair));
                } else {
                    shl_call_ref_pair = cast_call_pair_ref;
                }

                kefir_id_t mask_bigint_id;
                struct kefir_bigint mask_bigint;
                REQUIRE_OK(kefir_bigint_init(&mask_bigint));

                kefir_result_t res = kefir_bigint_resize_nocast(mem, &mask_bigint, bitwidth);
                REQUIRE_CHAIN(&res, kefir_bigint_set_unsigned_value(&mask_bigint, 0));
                REQUIRE_CHAIN(&res, kefir_bigint_invert(&mask_bigint));
                REQUIRE_CHAIN(&res, kefir_bigint_left_shift(&mask_bigint, bitwidth - length));
                REQUIRE_CHAIN(&res, kefir_bigint_right_shift(&mask_bigint, bitwidth - length - offset));
                REQUIRE_CHAIN(&res, kefir_bigint_invert(&mask_bigint));
                REQUIRE_CHAIN(&res, kefir_ir_module_new_bigint(mem, module->ir_module, &mask_bigint, &mask_bigint_id));
                REQUIRE_ELSE(res == KEFIR_OK, {
                    kefir_bigint_free(mem, &mask_bigint);
                    return res;
                });
                REQUIRE_OK(kefir_bigint_free(mem, &mask_bigint));
                REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_constant(mem, &func->code, block_id, mask_bigint_id,
                                                                           &mask_ref));

                kefir_opt_call_id_t and_call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, and_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &and_call_node_id, &and_call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, and_call_node_id, 0,
                                                                      init_copy_target_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, and_call_node_id, 1, mask_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, and_call_node_id, 2, bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, init_copy_target_pair_ref,
                                                       and_call_ref, &and_call_pair_ref));

                kefir_opt_call_id_t or_call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, or_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &or_call_node_id, &or_call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, or_call_node_id, 0,
                                                                      and_call_pair_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, or_call_node_id, 1,
                                                                      shl_call_ref_pair));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, &func->code, or_call_node_id, 2, bitwidth_ref));

                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, init_copy_target_pair_ref,
                                                       or_call_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_BUILTIN_FFS: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t extract_ref;

                kefir_id_t ffs_func_decl_id = KEFIR_ID_NONE;
                if (bitwidth < 32) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                          bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_ffs_function_decl_id(mem, module, param, &ffs_func_decl_id));
                } else if (bitwidth == 32) {
                    REQUIRE_OK(get_builtin_ffs_function_decl_id(mem, module, param, &ffs_func_decl_id));
                    extract_ref = arg_ref;
                } else if (bitwidth < QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                          bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_ffsl_function_decl_id(mem, module, param, &ffs_func_decl_id));
                } else {
                    REQUIRE_OK(get_builtin_ffsl_function_decl_id(mem, module, param, &ffs_func_decl_id));
                    extract_ref = arg_ref;
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, ffs_func_decl_id, 1,
                                                             KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, extract_ref));
            } else {
                kefir_id_t least_significant_nonzero_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_least_significant_nonzero_function_decl_id(
                    mem, codegen_module, module, param, &least_significant_nonzero_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                kefir_opt_instruction_ref_t bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id,
                                                             least_significant_nonzero_func_decl_id, 2, KEFIR_ID_NONE,
                                                             &call_node_id, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_BUILTIN_CLZ: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t extract_ref, call_ref, offset_ref;

                kefir_id_t clz_func_decl_id = KEFIR_ID_NONE;
                if (bitwidth < 32) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_clz_function_decl_id(mem, module, param, &clz_func_decl_id));
                } else if (bitwidth == 32) {
                    REQUIRE_OK(get_builtin_clz_function_decl_id(mem, module, param, &clz_func_decl_id));
                    extract_ref = arg_ref;
                } else if (bitwidth < QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_clzl_function_decl_id(mem, module, param, &clz_func_decl_id));
                } else {
                    REQUIRE_OK(get_builtin_clzl_function_decl_id(mem, module, param, &clz_func_decl_id));
                    extract_ref = arg_ref;
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, clz_func_decl_id, 1,
                                                             KEFIR_ID_NONE, &call_node_id, &call_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, extract_ref));

                if (bitwidth < 32) {
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 32 - bitwidth, &offset_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int32_sub(mem, &func->code, block_id, call_ref, offset_ref,
                                                                replacement_ref));
                } else if (bitwidth == 32) {
                    *replacement_ref = call_ref;
                } else if (bitwidth < QWORD_BITS) {
                    REQUIRE_OK(
                        kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 64 - bitwidth, &offset_ref));
                    REQUIRE_OK(kefir_opt_code_builder_int32_sub(mem, &func->code, block_id, call_ref, offset_ref,
                                                                replacement_ref));
                } else {
                    *replacement_ref = call_ref;
                }
            } else {
                kefir_id_t leading_zeros_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_leading_zeros_function_decl_id(mem, codegen_module, module, param,
                                                                     &leading_zeros_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                kefir_opt_instruction_ref_t bitwidth_ref, zero_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0, &zero_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, leading_zeros_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &call_node_id, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, zero_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_BUILTIN_CTZ: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t extract_ref;

                kefir_id_t ctz_func_decl_id = KEFIR_ID_NONE;
                if (bitwidth < 32) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_ctz_function_decl_id(mem, module, param, &ctz_func_decl_id));
                } else if (bitwidth == 32) {
                    REQUIRE_OK(get_builtin_ctz_function_decl_id(mem, module, param, &ctz_func_decl_id));
                    extract_ref = arg_ref;
                } else if (bitwidth < QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_ctzl_function_decl_id(mem, module, param, &ctz_func_decl_id));
                } else {
                    REQUIRE_OK(get_builtin_ctzl_function_decl_id(mem, module, param, &ctz_func_decl_id));
                    extract_ref = arg_ref;
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, ctz_func_decl_id, 1,
                                                             KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, extract_ref));
            } else {
                kefir_id_t trailing_zeros_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_trailing_zeros_function_decl_id(mem, codegen_module, module, param,
                                                                      &trailing_zeros_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                kefir_opt_instruction_ref_t bitwidth_ref, zero_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0, &zero_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, trailing_zeros_func_decl_id, 3,
                                                             KEFIR_ID_NONE, &call_node_id, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, zero_ref));
            }
        } break;

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t lower_function(struct kefir_mem *mem, struct kefir_codegen_amd64_module *codegen_module,
                                     struct kefir_opt_module *module, struct kefir_opt_function *func,
                                     struct lowering_param *param) {
    UNUSED(module);

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL;

        for (kefir_opt_code_block_instr_head(&func->code, block, &instr_id); instr_id != KEFIR_ID_NONE;) {
            REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(&func->debug_info, instr_id));
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));

            kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
            REQUIRE_OK(lower_instruction(mem, codegen_module, module, func, param, instr, &replacement_ref));

            if (replacement_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr->id));
                REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, instr->id,
                                                                            replacement_ref));
                if (instr->control_flow.prev != KEFIR_ID_NONE || instr->control_flow.next != KEFIR_ID_NONE) {
                    const struct kefir_opt_instruction *replacement_instr = NULL;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, replacement_ref, &replacement_instr));
                    if (replacement_instr->control_flow.prev == KEFIR_ID_NONE &&
                        replacement_instr->control_flow.next == KEFIR_ID_NONE) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block->id, instr_id, replacement_ref));
                    }
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                }
                kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
                REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, prev_instr_id));
            } else {
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
            }

            REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
                &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_lower_module(struct kefir_mem *mem,
                                                struct kefir_codegen_amd64_module *codegen_module,
                                                struct kefir_opt_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid memory allocator"));
    REQUIRE(codegen_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid amd64 code generator module"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid optimizer module"));

    struct lowering_param param = {.runtime_fn = {.bigint_set_signed = KEFIR_ID_NONE,
                                                  .bigint_set_unsigned = KEFIR_ID_NONE,
                                                  .bigint_cast_signed = KEFIR_ID_NONE,
                                                  .bigint_cast_unsigned = KEFIR_ID_NONE,
                                                  .bigint_signed_to_float = KEFIR_ID_NONE,
                                                  .bigint_unsigned_to_float = KEFIR_ID_NONE,
                                                  .bigint_signed_to_double = KEFIR_ID_NONE,
                                                  .bigint_unsigned_to_double = KEFIR_ID_NONE,
                                                  .bigint_signed_to_long_double = KEFIR_ID_NONE,
                                                  .bigint_unsigned_to_long_double = KEFIR_ID_NONE,
                                                  .bigint_signed_from_float = KEFIR_ID_NONE,
                                                  .bigint_unsigned_from_float = KEFIR_ID_NONE,
                                                  .bigint_signed_from_double = KEFIR_ID_NONE,
                                                  .bigint_unsigned_from_double = KEFIR_ID_NONE,
                                                  .bigint_signed_from_long_double = KEFIR_ID_NONE,
                                                  .bigint_unsigned_from_long_double = KEFIR_ID_NONE,
                                                  .bigint_is_zero = KEFIR_ID_NONE,
                                                  .bigint_negate = KEFIR_ID_NONE,
                                                  .bigint_invert = KEFIR_ID_NONE,
                                                  .bigint_add = KEFIR_ID_NONE,
                                                  .bigint_subtract = KEFIR_ID_NONE,
                                                  .bigint_signed_multiply = KEFIR_ID_NONE,
                                                  .bigint_unsigned_multiply = KEFIR_ID_NONE,
                                                  .bigint_signed_divide = KEFIR_ID_NONE,
                                                  .bigint_unsigned_divide = KEFIR_ID_NONE,
                                                  .bigint_lshift = KEFIR_ID_NONE,
                                                  .bigint_rshift = KEFIR_ID_NONE,
                                                  .bigint_arshift = KEFIR_ID_NONE,
                                                  .bigint_and = KEFIR_ID_NONE,
                                                  .bigint_or = KEFIR_ID_NONE,
                                                  .bigint_xor = KEFIR_ID_NONE,
                                                  .bigint_unsigned_compare = KEFIR_ID_NONE,
                                                  .bigint_signed_compare = KEFIR_ID_NONE,
                                                  .bigint_least_significant_nonzero = KEFIR_ID_NONE,
                                                  .bigint_leading_zeros = KEFIR_ID_NONE,
                                                  .bigint_trailing_zeros = KEFIR_ID_NONE,
                                                  .builtin_ffs = KEFIR_ID_NONE,
                                                  .builtin_ffsl = KEFIR_ID_NONE,
                                                  .builtin_clz = KEFIR_ID_NONE,
                                                  .builtin_clzl = KEFIR_ID_NONE,
                                                  .builtin_ctz = KEFIR_ID_NONE,
                                                  .builtin_ctzl = KEFIR_ID_NONE}};
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func = kefir_ir_module_function_iter(module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {

        struct kefir_opt_function *func;
        REQUIRE_OK(kefir_opt_module_get_function(module, ir_func->declaration->id, &func));
        REQUIRE_OK(lower_function(mem, codegen_module, module, func, &param));
    }
    return KEFIR_OK;
}
