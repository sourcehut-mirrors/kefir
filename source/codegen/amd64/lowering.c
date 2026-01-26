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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/lowering.h"
#include "kefir/target/abi/amd64/base.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
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
        kefir_id_t bigint_redundant_sign_bits;
        kefir_id_t bigint_nonzero_count;
        kefir_id_t bigint_parity;

        kefir_id_t builtin_ffs;
        kefir_id_t builtin_ffsl;
        kefir_id_t builtin_clz;
        kefir_id_t builtin_clzl;
        kefir_id_t builtin_ctz;
        kefir_id_t builtin_ctzl;
        kefir_id_t builtin_clrsb;
        kefir_id_t builtin_clrsbl;
        kefir_id_t builtin_popcount;
        kefir_id_t builtin_popcountl;
        kefir_id_t builtin_parity;
        kefir_id_t builtin_parityl;

        kefir_id_t sofxfloat_complex_float_mul;
        kefir_id_t sofxfloat_complex_double_mul;
        kefir_id_t sofxfloat_complex_long_double_mul;
        kefir_id_t sofxfloat_complex_float_div;
        kefir_id_t sofxfloat_complex_double_div;
        kefir_id_t sofxfloat_complex_long_double_div;

        kefir_id_t libgcc_udivti3;
        kefir_id_t libgcc_divti3;
        kefir_id_t libgcc_umodti3;
        kefir_id_t libgcc_modti3;
        kefir_id_t libgcc_floattisf;
        kefir_id_t libgcc_floattidf;
        kefir_id_t libgcc_floattixf;
        kefir_id_t libgcc_floatuntisf;
        kefir_id_t libgcc_floatuntidf;
        kefir_id_t libgcc_floatuntixf;

        kefir_id_t libgcc_fixsfti;
        kefir_id_t libgcc_fixdfti;
        kefir_id_t libgcc_fixxfti;
        kefir_id_t libgcc_fixunssfti;
        kefir_id_t libgcc_fixunsdfti;
        kefir_id_t libgcc_fixunsxfti;

#define DECL_DECIMAL_FNS(_prefix) \
        kefir_id_t _prefix##_addsd3; \
        kefir_id_t _prefix##_adddd3; \
        kefir_id_t _prefix##_addtd3; \
        kefir_id_t _prefix##_subsd3; \
        kefir_id_t _prefix##_subdd3; \
        kefir_id_t _prefix##_subtd3; \
        kefir_id_t _prefix##_mulsd3; \
        kefir_id_t _prefix##_muldd3; \
        kefir_id_t _prefix##_multd3; \
        kefir_id_t _prefix##_divsd3; \
        kefir_id_t _prefix##_divdd3; \
        kefir_id_t _prefix##_divtd3; \
        kefir_id_t _prefix##_eqsd3; \
        kefir_id_t _prefix##_eqdd3; \
        kefir_id_t _prefix##_eqtd3; \
        kefir_id_t _prefix##_gtsd3; \
        kefir_id_t _prefix##_gtdd3; \
        kefir_id_t _prefix##_gttd3; \
        kefir_id_t _prefix##_ltsd3; \
        kefir_id_t _prefix##_ltdd3; \
        kefir_id_t _prefix##_lttd3; \
        /* Decimal to decimal */ \
        kefir_id_t _prefix##_extendsddd2; \
        kefir_id_t _prefix##_extendsdtd2; \
        kefir_id_t _prefix##_extendddtd2; \
        kefir_id_t _prefix##_truncddsd2; \
        kefir_id_t _prefix##_trunctdsd2; \
        kefir_id_t _prefix##_trunctddd2; \
        /* Decimal32 to ... */ \
        kefir_id_t _prefix##_truncsdsf; \
        kefir_id_t _prefix##_extendsddf; \
        kefir_id_t _prefix##_extendsdxf; \
        /* Decimal64 to ... */ \
        kefir_id_t _prefix##_truncddsf; \
        kefir_id_t _prefix##_truncdddf; \
        kefir_id_t _prefix##_extendddxf; \
        /* Decimal128 to ... */ \
        kefir_id_t _prefix##_trunctdsf; \
        kefir_id_t _prefix##_trunctddf; \
        kefir_id_t _prefix##_trunctdxf; \
        /* Decimal32 from ... */ \
        kefir_id_t _prefix##_extendsfsd; \
        kefir_id_t _prefix##_truncdfsd; \
        kefir_id_t _prefix##_truncxfsd; \
        /* Decimal64 from ... */ \
        kefir_id_t _prefix##_extendsfdd; \
        kefir_id_t _prefix##_extenddfdd; \
        kefir_id_t _prefix##_truncxfdd; \
        /* Decimal128 from ... */ \
        kefir_id_t _prefix##_extendsftd; \
        kefir_id_t _prefix##_extenddftd; \
        kefir_id_t _prefix##_extendxftd; \
        /* ... to long */ \
        kefir_id_t _prefix##_fixsddi; \
        kefir_id_t _prefix##_fixdddi; \
        kefir_id_t _prefix##_fixtddi; \
        /* ... to unsigned long */ \
        kefir_id_t _prefix##_fixunssddi; \
        kefir_id_t _prefix##_fixunsdddi; \
        kefir_id_t _prefix##_fixunstddi; \
        /* long from ... */ \
        kefir_id_t _prefix##_floatdisd; \
        kefir_id_t _prefix##_floatdidd; \
        kefir_id_t _prefix##_floatditd; \
        /* unsigned  long from ... */ \
        kefir_id_t _prefix##_floatunsdisd; \
        kefir_id_t _prefix##_floatunsdidd; \
        kefir_id_t _prefix##_floatunsditd; \
 \
        kefir_id_t _prefix##_floatbitintsd; \
        kefir_id_t _prefix##_floatbitintdd; \
        kefir_id_t _prefix##_floatbitinttd; \
 \
        kefir_id_t _prefix##_fixsdbitint; \
        kefir_id_t _prefix##_fixddbitint; \
        kefir_id_t _prefix##_fixtdbitint; \
 \
        kefir_id_t _prefix##_unordsd2; \
        kefir_id_t _prefix##_unorddd2; \
        kefir_id_t _prefix##_unordtd2; \
        \
        kefir_id_t _prefix##_floattisd; \
        kefir_id_t _prefix##_floattidd; \
        kefir_id_t _prefix##_floattitd; \
        kefir_id_t _prefix##_floatunstisd; \
        kefir_id_t _prefix##_floatunstidd; \
        kefir_id_t _prefix##_floatunstitd; \
        kefir_id_t _prefix##_fixsdti; \
        kefir_id_t _prefix##_fixddti; \
        kefir_id_t _prefix##_fixtdti; \
        kefir_id_t _prefix##_fixunssdti; \
        kefir_id_t _prefix##_fixunsddti; \
        kefir_id_t _prefix##_fixunstdti

        DECL_DECIMAL_FNS(libgcc_bid);
        DECL_DECIMAL_FNS(libgcc_dpd);
#undef DECL_DECIMAL_FNS
    } runtime_fn;
};

#define RUNTIME_FN_IDENTIFIER(_name)                                                                \
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

#define DECL_RUNTIME_FN(_id, _name, _params, _returns, _init)                                             \
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
                                                      &RUNTIME_FN_IDENTIFIER(func_decl->name)));          \
                                                                                                                 \
        REQUIRE_OK(kefir_opt_module_require_runtime_function(mem, module, func_decl->name));                     \
                                                                                                                 \
        param->runtime_fn._id = func_decl->id;                                                                   \
        *func_decl_id = func_decl->id;                                                                           \
        return KEFIR_OK;                                                                                         \
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

DECL_RUNTIME_FN(bigint_set_signed, BIGINT_GET_SET_SIGNED_INTEGER_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
})
DECL_RUNTIME_FN(bigint_set_unsigned, BIGINT_GET_SET_UNSIGNED_INTEGER_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
})
DECL_RUNTIME_FN(bigint_cast_signed, BIGINT_CAST_SIGNED_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_cast_unsigned, BIGINT_CAST_UNSIGNED_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_to_float, BIGINT_SIGNED_TO_FLOAT_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_to_float, BIGINT_UNSIGNED_TO_FLOAT_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_to_double, BIGINT_SIGNED_TO_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_to_double, BIGINT_UNSIGNED_TO_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_to_long_double, BIGINT_SIGNED_TO_LONG_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_to_long_double, BIGINT_UNSIGNED_TO_LONG_DOUBLE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_from_float, BIGINT_SIGNED_FROM_FLOAT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_from_float, BIGINT_UNSIGNED_FROM_FLOAT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_from_double, BIGINT_SIGNED_FROM_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_from_double, BIGINT_UNSIGNED_FROM_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_from_long_double, BIGINT_SIGNED_FROM_LONG_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_from_long_double, BIGINT_UNSIGNED_FROM_LONG_DOUBLE_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_is_zero, BIGINT_IS_ZERO_FN, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT8, 0, 0));
})
DECL_RUNTIME_FN(bigint_negate, BIGINT_NEGATE_FN, 2, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_invert, BIGINT_INVERT_FN, 2, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_add, BIGINT_ADD_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_subtract, BIGINT_SUBTRACT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_and, BIGINT_AND_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_or, BIGINT_OR_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_xor, BIGINT_XOR_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_multiply, BIGINT_SIGNED_MULTIPLY_FN, 6, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_multiply, BIGINT_UNSIGNED_MULTIPLY_FN, 5, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_divide, BIGINT_SIGNED_DIVIDE_FN, 5, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_divide, BIGINT_UNSIGNED_DIVIDE_FN, 5, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_lshift, BIGINT_LEFT_SHIFT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_rshift, BIGINT_RIGHT_SHIFT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_arshift, BIGINT_ARITHMETIC_RIGHT_SHIFT_FN, 3, 0, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_unsigned_compare, BIGINT_UNSIGNED_COMPARE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT8, 0, 0));
})
DECL_RUNTIME_FN(bigint_signed_compare, BIGINT_SIGNED_COMPARE_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_least_significant_nonzero, BIGINT_LEAST_SIGNIFICANT_NONZERO_FN, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_leading_zeros, BIGINT_LEADING_ZEROS_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_trailing_zeros, BIGINT_TRAILING_ZEROS_FN, 3, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_redundant_sign_bits, BIGINT_REDUNDANT_SIGN_BITS_FN, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_nonzero_count, BIGINT_NONZERO_COUNT_FN, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(bigint_parity, BIGINT_PARITY_FN, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
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
DECL_BUILTIN_RUNTIME_FN(builtin_clrsb, BUILTIN_CLRSB_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_clrsbl, BUILTIN_CLRSBL_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_popcount, BUILTIN_POPCOUNT_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_popcountl, BUILTIN_POPCOUNTL_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_parity, BUILTIN_PARITY_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(builtin_parityl, BUILTIN_PARITYL_FN, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0));
})
DECL_RUNTIME_FN(sofxfloat_complex_float_mul, KEFIR_SOFTFLOAT_COMPLEX_FLOAT_MUL, 4, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_COMPLEX_FLOAT32, 0, 0));
})
DECL_RUNTIME_FN(sofxfloat_complex_double_mul, KEFIR_SOFTFLOAT_COMPLEX_DOUBLE_MUL, 4, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_COMPLEX_FLOAT64, 0, 0));
})
DECL_RUNTIME_FN(sofxfloat_complex_long_double_mul, KEFIR_SOFTFLOAT_COMPLEX_LONG_DOUBLE_MUL, 4, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE, 0, 0));
})
DECL_RUNTIME_FN(sofxfloat_complex_float_div, KEFIR_SOFTFLOAT_COMPLEX_FLOAT_DIV, 4, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_COMPLEX_FLOAT32, 0, 0));
})
DECL_RUNTIME_FN(sofxfloat_complex_double_div, KEFIR_SOFTFLOAT_COMPLEX_DOUBLE_DIV, 4, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_COMPLEX_FLOAT64, 0, 0));
})
DECL_RUNTIME_FN(sofxfloat_complex_long_double_div, KEFIR_SOFTFLOAT_COMPLEX_LONG_DOUBLE_DIV, 4, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_divti3, LIBGCC_DIVTI3, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_udivti3, LIBGCC_UDIVTI3, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_modti3, LIBGCC_MODTI3, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_umodti3, LIBGCC_UMODTI3, 2, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_floattisf, LIBGCC_FLOATTISF, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_floattidf, LIBGCC_FLOATTIDF, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_floattixf, LIBGCC_FLOATTIXF, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_floatuntisf, LIBGCC_FLOATUNTISF, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_floatuntidf, LIBGCC_FLOATUNTIDF, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_floatuntixf, LIBGCC_FLOATUNTIXF, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_fixsfti, LIBGCC_FIXSFTI, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_fixdfti, LIBGCC_FIXDFTI, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_fixxfti, LIBGCC_FIXXFTI, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_fixunssfti, LIBGCC_FIXUNSSFTI, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_fixunsdfti, LIBGCC_FIXUNSDFTI, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
DECL_BUILTIN_RUNTIME_FN(libgcc_fixunsxfti, LIBGCC_FIXUNSXFTI, 1, 1, {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0));
})
#define DECL_DECIMAL_FNS(_prefix, _encoding) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_addsd3, LIBGCC_DECIMAL_ADDSD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_adddd3, LIBGCC_DECIMAL_ADDDD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_addtd3, LIBGCC_DECIMAL_ADDTD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_subsd3, LIBGCC_DECIMAL_SUBSD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_subdd3, LIBGCC_DECIMAL_SUBDD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_subtd3, LIBGCC_DECIMAL_SUBTD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_mulsd3, LIBGCC_DECIMAL_MULSD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_muldd3, LIBGCC_DECIMAL_MULDD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_multd3, LIBGCC_DECIMAL_MULTD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_divsd3, LIBGCC_DECIMAL_DIVSD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_divdd3, LIBGCC_DECIMAL_DIVDD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_divtd3, LIBGCC_DECIMAL_DIVTD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_eqsd3, LIBGCC_DECIMAL_EQSD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_eqdd3, LIBGCC_DECIMAL_EQDD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_eqtd3, LIBGCC_DECIMAL_EQTD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_gtsd3, LIBGCC_DECIMAL_GTSD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_gtdd3, LIBGCC_DECIMAL_GTDD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_gttd3, LIBGCC_DECIMAL_GTTD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_ltsd3, LIBGCC_DECIMAL_LTSD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_ltdd3, LIBGCC_DECIMAL_LTDD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_lttd3, LIBGCC_DECIMAL_LTTD3(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
/* Decimal to decimal */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendsddd2, LIBGCC_DECIMAL_EXTENDSDDD2(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendsdtd2, LIBGCC_DECIMAL_EXTENDSDTD2(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendddtd2, LIBGCC_DECIMAL_EXTENDDDTD2(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_truncddsd2, LIBGCC_DECIMAL_TRUNCDDSD2(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_trunctdsd2, LIBGCC_DECIMAL_TRUNCTDSD2(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_trunctddd2, LIBGCC_DECIMAL_TRUNCTDDD2(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
/* Decimal32 to ... */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_truncsdsf, LIBGCC_DECIMAL_TRUNCSDSF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendsddf, LIBGCC_DECIMAL_EXTENDSDDF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendsdxf, LIBGCC_DECIMAL_EXTENDSDXF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0)); \
}) \
/* Decimal64 to ... */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_truncddsf, LIBGCC_DECIMAL_TRUNCDDSF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_truncdddf, LIBGCC_DECIMAL_TRUNCDDDF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendddxf, LIBGCC_DECIMAL_EXTENDDDXF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0)); \
}) \
/* Decimal128 to ... */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_trunctdsf, LIBGCC_DECIMAL_TRUNCTDSF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_trunctddf, LIBGCC_DECIMAL_TRUNCTDDF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_FLOAT64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_trunctdxf, LIBGCC_DECIMAL_TRUNCTDXF(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0)); \
}) \
/* Decimal32 from ... */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendsfsd, LIBGCC_DECIMAL_EXTENDSFSD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_truncdfsd, LIBGCC_DECIMAL_TRUNCDFSD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_truncxfsd, LIBGCC_DECIMAL_TRUNCXFSD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
/* Decimal64 from ... */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendsfdd, LIBGCC_DECIMAL_EXTENDSFDD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extenddfdd, LIBGCC_DECIMAL_EXTENDDFDD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_truncxfdd, LIBGCC_DECIMAL_TRUNCXFDD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
/* Decimal128 from ... */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendsftd, LIBGCC_DECIMAL_EXTENDSFTD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extenddftd, LIBGCC_DECIMAL_EXTENDDFTD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_FLOAT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_extendxftd, LIBGCC_DECIMAL_EXTENDXFTD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
/* ... to long */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixsddi, LIBGCC_DECIMAL_FIXSDDI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixdddi, LIBGCC_DECIMAL_FIXDDDI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixtddi, LIBGCC_DECIMAL_FIXTDDI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
}) \
/* ... to unsigned long */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixunssddi, LIBGCC_DECIMAL_FIXUNSSDDI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixunsdddi, LIBGCC_DECIMAL_FIXUNSDDDI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixunstddi, LIBGCC_DECIMAL_FIXUNSTDDI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
}) \
/* long from ... */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatdisd, LIBGCC_DECIMAL_FLOATDISD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatdidd, LIBGCC_DECIMAL_FLOATDIDD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatditd, LIBGCC_DECIMAL_FLOATDITD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
/* unsigned long from ... */ \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatunsdisd, LIBGCC_DECIMAL_FLOATUNSDISD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatunsdidd, LIBGCC_DECIMAL_FLOATUNSDIDD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatunsditd, LIBGCC_DECIMAL_FLOATUNSDITD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatbitintsd, LIBGCC_DECIMAL_FLOATBITINTSD(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatbitintdd, LIBGCC_DECIMAL_FLOATBITINTDD(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatbitinttd, LIBGCC_DECIMAL_FLOATBITINTTD(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixsdbitint, LIBGCC_DECIMAL_FIXSDBITINT(_encoding), 3, 0, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixddbitint, LIBGCC_DECIMAL_FIXDDBITINT(_encoding), 3, 0, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixtdbitint, LIBGCC_DECIMAL_FIXTDBITINT(_encoding), 3, 0, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_unordsd2, LIBGCC_DECIMAL_UNORDSD2(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_unorddd2, LIBGCC_DECIMAL_UNORDDD2(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_unordtd2, LIBGCC_DECIMAL_UNORDTD2(_encoding), 2, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floattisd, LIBGCC_DECIMAL_FLOATTISD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floattidd, LIBGCC_DECIMAL_FLOATTIDD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floattitd, LIBGCC_DECIMAL_FLOATTITD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatunstisd, LIBGCC_DECIMAL_FLOATUNSTISD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatunstidd, LIBGCC_DECIMAL_FLOATUNSTIDD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_floatunstitd, LIBGCC_DECIMAL_FLOATUNSTITD(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixsdti, LIBGCC_DECIMAL_FIXSDTI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixddti, LIBGCC_DECIMAL_FIXDDTI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixtdti, LIBGCC_DECIMAL_FIXTDTI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixunssdti, LIBGCC_DECIMAL_FIXUNSSDTI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL32, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixunsddti, LIBGCC_DECIMAL_FIXUNSDDTI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL64, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
}) \
DECL_BUILTIN_RUNTIME_FN(_prefix##_fixunstdti, LIBGCC_DECIMAL_FIXUNSTDTI(_encoding), 1, 1, { \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, parameters_type, KEFIR_IR_TYPE_DECIMAL128, 0, 0)); \
    REQUIRE_OK(kefir_irbuilder_type_append(mem, returns_type, KEFIR_IR_TYPE_INT128, 0, 0)); \
})

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
DECL_DECIMAL_FNS(libgcc_bid, LIBGCC_DECIMAL_BID)
DECL_DECIMAL_FNS(libgcc_dpd, LIBGCC_DECIMAL_DPD)
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#undef DECL_DECIMAL_FNS

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

static kefir_result_t lower_instruction(struct kefir_mem *mem, struct kefir_opt_module *module,
                                        struct kefir_opt_function *func, const struct kefir_optimizer_configuration *configuration, struct lowering_param *param,
                                        const struct kefir_opt_instruction *instr,
                                        kefir_opt_instruction_ref_t *replacement_ref) {
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
                REQUIRE_OK(get_bigint_set_signed_function_decl_id(mem, module, param, &func_decl_id));

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
                REQUIRE_OK(get_bigint_set_unsigned_function_decl_id(mem, module, param, &func_decl_id));

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
                    REQUIRE_OK(get_bigint_set_signed_function_decl_id(mem, module, param, &func_decl_id));
                } else {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            src_bitwidth, &casted_arg_ref));
                    REQUIRE_OK(get_bigint_set_unsigned_function_decl_id(mem, module, param, &func_decl_id));
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
                    REQUIRE_OK(get_bigint_cast_signed_function_decl_id(mem, module, param, &func_decl_id));
                } else {
                    REQUIRE_OK(get_bigint_cast_unsigned_function_decl_id(mem, module, param, &func_decl_id));
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
                        REQUIRE_OK(get_bigint_signed_to_float_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT:
                        REQUIRE_OK(get_bigint_unsigned_to_float_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE:
                        REQUIRE_OK(get_bigint_signed_to_double_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE:
                        REQUIRE_OK(get_bigint_unsigned_to_double_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(
                            get_bigint_signed_to_long_double_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE:
                        REQUIRE_OK(
                            get_bigint_unsigned_to_long_double_function_decl_id(mem, module, param, &func_decl_id));
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
                        REQUIRE_OK(get_bigint_signed_from_float_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(get_bigint_unsigned_from_float_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(get_bigint_signed_from_double_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(get_bigint_unsigned_from_double_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED:
                        REQUIRE_OK(
                            get_bigint_signed_from_long_double_function_decl_id(mem, module, param, &func_decl_id));
                        break;

                    case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED:
                        REQUIRE_OK(
                            get_bigint_unsigned_from_long_double_function_decl_id(mem, module, param, &func_decl_id));
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
                REQUIRE_OK(get_bigint_is_zero_function_decl_id(mem, module, param, &func_decl_id));

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

            kefir_bool_t is_control_flow;
            REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, original_instr_ref, &is_control_flow));

            if (bitwidth <= QWORD_BITS) {
                if (bitwidth <= 8) {
                    REQUIRE_OK(kefir_opt_code_builder_int8_store(mem, &func->code, block_id, location_arg_ref,
                                                                 value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= 16) {
                    REQUIRE_OK(kefir_opt_code_builder_int16_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= 24) {
                    kefir_opt_instruction_ref_t shift_const1_ref, store1_ref, store2_ref, shift1_ref, location1_const_ref,
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
                                                                 &memflags, &store2_ref));
                    if (is_control_flow) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store1_ref));
                        *replacement_ref = store2_ref;
                    } else {
                        REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, store1_ref, store2_ref, replacement_ref));
                    }
                } else if (bitwidth <= 32) {
                    REQUIRE_OK(kefir_opt_code_builder_int32_store(mem, &func->code, block_id, location_arg_ref,
                                                                  value_arg_ref, &memflags, replacement_ref));
                } else if (bitwidth <= 40) {
                    kefir_opt_instruction_ref_t shift_const1_ref, store1_ref, store2_ref, shift1_ref, location1_const_ref,
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
                                                                 &memflags, &store2_ref));
                    if (is_control_flow) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store1_ref));
                        *replacement_ref = store2_ref;
                    } else {
                        REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, store1_ref, store2_ref, replacement_ref));
                    }
                } else if (bitwidth <= 48) {
                    kefir_opt_instruction_ref_t shift_const1_ref, store1_ref, store2_ref, shift1_ref, location1_const_ref,
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
                                                                  &memflags, &store2_ref));
                    if (is_control_flow) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store1_ref));
                            *replacement_ref = store2_ref;
                    } else {
                        REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, store1_ref, store2_ref, replacement_ref));
                    }
                } else if (bitwidth <= 56) {
                    kefir_opt_instruction_ref_t shift_const1_ref, shift_const2_ref, store1_ref, shift1_ref, shift2_ref,
                        location1_const_ref, location2_const_ref, location1_ref, location2_ref, store2_ref, store3_ref;
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
                                                                 &memflags, &store3_ref));

                    if (is_control_flow) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store1_ref));
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block_id, original_instr_ref, store2_ref));
                        *replacement_ref = store3_ref;
                    } else {
                        kefir_opt_instruction_ref_t pair1_ref;
                        REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, store1_ref, store2_ref, &pair1_ref));
                        REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, pair1_ref, store3_ref, replacement_ref));
                    }
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
                REQUIRE_OK(get_bigint_negate_function_decl_id(mem, module, param, &func_decl_id));

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
                REQUIRE_OK(get_bigint_invert_function_decl_id(mem, module, param, &func_decl_id));

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
                REQUIRE_OK(get_bigint_is_zero_function_decl_id(mem, module, param, &func_decl_id));

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
            REQUIRE_OK(_fn(mem, module, param, &func_decl_id));                                                        \
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
                REQUIRE_OK(get_bigint_unsigned_multiply_function_decl_id(mem, module, param, &func_decl_id));

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
                REQUIRE_OK(get_bigint_signed_multiply_function_decl_id(mem, module, param, &func_decl_id));

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
                REQUIRE_OK(get_bigint_unsigned_divide_function_decl_id(mem, module, param, &func_decl_id));
                REQUIRE_OK(get_bigint_cast_unsigned_function_decl_id(mem, module, param, &copy_func_decl_id));

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
                REQUIRE_OK(get_bigint_signed_divide_function_decl_id(mem, module, param, &func_decl_id));
                REQUIRE_OK(get_bigint_cast_signed_function_decl_id(mem, module, param, &copy_func_decl_id));

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
                REQUIRE_OK(get_bigint_unsigned_divide_function_decl_id(mem, module, param, &func_decl_id));
                REQUIRE_OK(get_bigint_cast_unsigned_function_decl_id(mem, module, param, &copy_func_decl_id));

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
                REQUIRE_OK(get_bigint_signed_divide_function_decl_id(mem, module, param, &func_decl_id));
                REQUIRE_OK(get_bigint_cast_signed_function_decl_id(mem, module, param, &copy_func_decl_id));

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
            REQUIRE_OK(_fn(mem, module, param, &func_decl_id));                                                        \
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
            REQUIRE_OK(_fn(mem, module, param, &func_decl_id));                                                        \
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
                REQUIRE_OK(get_bigint_rshift_function_decl_id(mem, module, param, &shr_func_decl_id));
                if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_EXTRACT_SIGNED) {
                    REQUIRE_OK(get_bigint_cast_signed_function_decl_id(mem, module, param, &cast_func_decl_id));
                } else {
                    REQUIRE_OK(get_bigint_cast_unsigned_function_decl_id(mem, module, param, &cast_func_decl_id));
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
                REQUIRE_OK(get_bigint_lshift_function_decl_id(mem, module, param, &shl_func_decl_id));
                REQUIRE_OK(get_bigint_cast_unsigned_function_decl_id(mem, module, param, &cast_func_decl_id));
                REQUIRE_OK(get_bigint_or_function_decl_id(mem, module, param, &or_func_decl_id));
                REQUIRE_OK(get_bigint_and_function_decl_id(mem, module, param, &and_func_decl_id));

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
                    mem, module, param, &least_significant_nonzero_func_decl_id));

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
                REQUIRE_OK(get_bigint_leading_zeros_function_decl_id(mem, module, param, &leading_zeros_func_decl_id));

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
                REQUIRE_OK(
                    get_bigint_trailing_zeros_function_decl_id(mem, module, param, &trailing_zeros_func_decl_id));

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

        case KEFIR_OPT_OPCODE_BITINT_BUILTIN_CLRSB: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t extract_ref, call_ref, offset_ref;

                kefir_id_t clrsb_func_decl_id = KEFIR_ID_NONE;
                if (bitwidth < 32) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                          bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_clrsb_function_decl_id(mem, module, param, &clrsb_func_decl_id));
                } else if (bitwidth == 32) {
                    REQUIRE_OK(get_builtin_clrsb_function_decl_id(mem, module, param, &clrsb_func_decl_id));
                    extract_ref = arg_ref;
                } else if (bitwidth < QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, block_id, arg_ref, 0,
                                                                          bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_clrsbl_function_decl_id(mem, module, param, &clrsb_func_decl_id));
                } else {
                    REQUIRE_OK(get_builtin_clrsbl_function_decl_id(mem, module, param, &clrsb_func_decl_id));
                    extract_ref = arg_ref;
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, clrsb_func_decl_id, 1,
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
                kefir_id_t redundant_sign_bits_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_redundant_sign_bits_function_decl_id(mem, module, param,
                                                                           &redundant_sign_bits_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                kefir_opt_instruction_ref_t bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id,
                                                             redundant_sign_bits_func_decl_id, 2, KEFIR_ID_NONE,
                                                             &call_node_id, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_BUILTIN_POPCOUNT: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t extract_ref;

                kefir_id_t popcount_func_decl_id = KEFIR_ID_NONE;
                if (bitwidth < 32) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_popcount_function_decl_id(mem, module, param, &popcount_func_decl_id));
                } else if (bitwidth == 32) {
                    REQUIRE_OK(get_builtin_popcount_function_decl_id(mem, module, param, &popcount_func_decl_id));
                    extract_ref = arg_ref;
                } else if (bitwidth < QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_popcountl_function_decl_id(mem, module, param, &popcount_func_decl_id));
                } else {
                    REQUIRE_OK(get_builtin_popcountl_function_decl_id(mem, module, param, &popcount_func_decl_id));
                    extract_ref = arg_ref;
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, popcount_func_decl_id, 1,
                                                             KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, extract_ref));
            } else {
                kefir_id_t nonzero_count_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_nonzero_count_function_decl_id(mem, module, param, &nonzero_count_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                kefir_opt_instruction_ref_t bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, nonzero_count_func_decl_id, 2,
                                                             KEFIR_ID_NONE, &call_node_id, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_BITINT_BUILTIN_PARITY: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= QWORD_BITS) {
                kefir_opt_instruction_ref_t extract_ref;

                kefir_id_t parity_func_decl_id = KEFIR_ID_NONE;
                if (bitwidth < 32) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_parity_function_decl_id(mem, module, param, &parity_func_decl_id));
                } else if (bitwidth == 32) {
                    REQUIRE_OK(get_builtin_parity_function_decl_id(mem, module, param, &parity_func_decl_id));
                    extract_ref = arg_ref;
                } else if (bitwidth < QWORD_BITS) {
                    REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, block_id, arg_ref, 0,
                                                                            bitwidth, &extract_ref));
                    REQUIRE_OK(get_builtin_parityl_function_decl_id(mem, module, param, &parity_func_decl_id));
                } else {
                    REQUIRE_OK(get_builtin_parityl_function_decl_id(mem, module, param, &parity_func_decl_id));
                    extract_ref = arg_ref;
                }

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, parity_func_decl_id, 1,
                                                             KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, extract_ref));
            } else {
                kefir_id_t parity_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_bigint_parity_function_decl_id(mem, module, param, &parity_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                kefir_opt_instruction_ref_t bitwidth_ref;
                REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, bitwidth, &bitwidth_ref));
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, parity_func_decl_id, 2,
                                                             KEFIR_ID_NONE, &call_node_id, replacement_ref));

                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_NEG: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, result_real_ref, result_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_neg(mem, &func->code, block_id, arg1_real_ref, &result_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_neg(mem, &func->code, block_id, arg1_imag_ref, &result_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_ADD: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref, result_real_ref, result_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float32_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float32_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float32_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float32_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_float32_add(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &result_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_float32_add(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &result_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_float32_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_ADD: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref, result_real_ref, result_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float64_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float64_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float64_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float64_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_float64_add(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &result_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_float64_add(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &result_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_float64_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_ADD: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref, result_real_ref, result_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_add(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &result_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_add(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &result_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_SUB: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref, result_real_ref, result_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float32_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float32_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float32_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float32_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_float32_sub(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &result_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_float32_sub(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &result_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_float32_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_SUB: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref, result_real_ref, result_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float64_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float64_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float64_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_float64_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_float64_sub(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &result_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_float64_sub(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &result_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_float64_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_SUB: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref, result_real_ref, result_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_sub(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &result_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_sub(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &result_imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_MUL:
            if (!func->ir_func->flags.cx_limited_range) {
                const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
                const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

                kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref;
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float32_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float32_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float32_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float32_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));

                kefir_id_t mulsc3_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_sofxfloat_complex_float_mul_function_decl_id(mem, module, param, &mulsc3_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, mulsc3_func_decl_id, 4,
                                                            KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg1_imag_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, arg2_imag_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_MUL:
            if (!func->ir_func->flags.cx_limited_range) {
                const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
                const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

                kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref;
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float64_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float64_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float64_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float64_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));

                kefir_id_t muldc3_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_sofxfloat_complex_double_mul_function_decl_id(mem, module, param, &muldc3_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, muldc3_func_decl_id, 4,
                                                            KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg1_imag_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, arg2_imag_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_MUL: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg1_ref,
                                                                            &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg2_ref,
                                                                            &arg2_imag_ref));

            if (!func->ir_func->flags.cx_limited_range) {
                kefir_id_t mulxc3_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_sofxfloat_complex_long_double_mul_function_decl_id(mem, module, param, &mulxc3_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, mulxc3_func_decl_id, 4,
                                                            KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg1_imag_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, arg2_imag_ref));
            } else {
                kefir_opt_instruction_ref_t result_real_ac_ref, result_real_bd_ref, result_imag_ad_ref, result_imag_bc_ref, result_real_ref, result_imag_ref;
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &result_real_ac_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &result_real_bd_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg1_real_ref, arg2_imag_ref, &result_imag_ad_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg1_imag_ref, arg2_real_ref, &result_imag_bc_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_sub(mem, &func->code, block_id, result_real_ac_ref, result_real_bd_ref, &result_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_add(mem, &func->code, block_id, result_imag_ad_ref, result_imag_bc_ref, &result_imag_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_long_double_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_DIV:
            if (!func->ir_func->flags.cx_limited_range) {
                const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
                const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

                kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref;
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float32_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float32_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float32_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float32_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));

                kefir_id_t divsc3_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_sofxfloat_complex_float_div_function_decl_id(mem, module, param, &divsc3_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, divsc3_func_decl_id, 4,
                                                            KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg1_imag_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, arg2_imag_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_DIV:
            if (!func->ir_func->flags.cx_limited_range) {
                const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
                const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

                kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref;
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float64_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float64_imaginary(mem, &func->code, block_id, arg1_ref, &arg1_imag_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float64_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_float64_imaginary(mem, &func->code, block_id, arg2_ref, &arg2_imag_ref));

                kefir_id_t divdc3_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_sofxfloat_complex_double_div_function_decl_id(mem, module, param, &divdc3_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, divdc3_func_decl_id, 4,
                                                            KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg1_imag_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, arg2_imag_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_DIV: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg1_ref,
                                                                            &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg2_ref,
                                                                            &arg2_imag_ref));
            if (!func->ir_func->flags.cx_limited_range) {
                kefir_id_t divxc3_func_decl_id = KEFIR_ID_NONE;
                REQUIRE_OK(get_sofxfloat_complex_long_double_div_function_decl_id(mem, module, param, &divxc3_func_decl_id));

                kefir_opt_call_id_t call_node_id;
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, divxc3_func_decl_id, 4,
                                                            KEFIR_ID_NONE, &call_node_id, replacement_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg1_imag_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg2_real_ref));
                REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 3, arg2_imag_ref));
            } else {
                kefir_opt_instruction_ref_t dividend_real_ac_ref, dividend_real_bd_ref, dividend_imag_ad_ref, dividend_imag_bc_ref, dividend_real_ref, dividend_imag_ref,
                    arg2_real_squre_ref, arg2_imag_squre_ref, divisor_ref, result_real_ref, result_imag_ref;
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &dividend_real_ac_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &dividend_real_bd_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg1_real_ref, arg2_imag_ref, &dividend_imag_ad_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg1_imag_ref, arg2_real_ref, &dividend_imag_bc_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_add(mem, &func->code, block_id, dividend_real_ac_ref, dividend_real_bd_ref, &dividend_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_sub(mem, &func->code, block_id, dividend_imag_bc_ref, dividend_imag_ad_ref, &dividend_imag_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg2_real_ref, arg2_real_ref, &arg2_real_squre_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_mul(mem, &func->code, block_id, arg2_imag_ref, arg2_imag_ref, &arg2_imag_squre_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_add(mem, &func->code, block_id, arg2_real_squre_ref, arg2_imag_squre_ref, &divisor_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_div(mem, &func->code, block_id, dividend_real_ref, divisor_ref, &result_real_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_long_double_div(mem, &func->code, block_id, dividend_imag_ref, divisor_ref, &result_imag_ref));
                REQUIRE_OK(
                    kefir_opt_code_builder_complex_long_double_from(mem, &func->code, block_id, result_real_ref, result_imag_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_EQUALS: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref, arg2_real_ref, arg2_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg1_ref,
                                                                            &arg1_imag_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg2_ref, &arg2_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg2_ref,
                                                                            &arg2_imag_ref));
                                                                            
            kefir_opt_instruction_ref_t real_cmp_ref, imag_cmp_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_long_double_equals(mem, &func->code, block_id, arg1_real_ref, arg2_real_ref, &real_cmp_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_long_double_equals(mem, &func->code, block_id, arg1_imag_ref, arg2_imag_ref, &imag_cmp_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int64_and(mem, &func->code, block_id, real_cmp_ref, imag_cmp_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT: {
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0];

            kefir_opt_instruction_ref_t arg1_real_ref, arg1_imag_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, arg1_ref, &arg1_real_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, arg1_ref,
                                                                            &arg1_imag_ref));
                                                                            
            REQUIRE_OK(
                kefir_opt_code_builder_long_double_pair_truncate_1bit(mem, &func->code, block_id, arg1_real_ref, arg1_imag_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD: {
            kefir_opt_instruction_ref_t instr_ref = instr->id;
            const kefir_opt_instruction_ref_t location_ref = instr->operation.parameters.refs[0];
            struct kefir_opt_memory_access_flags flags = instr->operation.parameters.memory_access.flags;

            kefir_opt_instruction_ref_t imag_offset_ref, imag_location_ref, real_ref, imag_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, &imag_offset_ref));
            REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, location_ref, imag_offset_ref, &imag_location_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_load(mem, &func->code, block_id, location_ref, &flags, &real_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_load(mem, &func->code, block_id, imag_location_ref, &flags, &imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_from(mem, &func->code, block_id, real_ref, imag_ref, replacement_ref));

            kefir_bool_t is_control_flow;
            REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, instr_ref, &is_control_flow));
            if (is_control_flow) {
                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_ref, real_ref));
                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_ref, imag_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE: {
            kefir_opt_instruction_ref_t instr_ref = instr->id;
            const kefir_opt_instruction_ref_t location_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t value_ref = instr->operation.parameters.refs[1];
            struct kefir_opt_memory_access_flags flags = instr->operation.parameters.memory_access.flags;

            kefir_opt_instruction_ref_t imag_offset_ref, imag_location_ref, real_ref, imag_ref, real_store_ref, imag_store_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, &imag_offset_ref));
            REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, location_ref, imag_offset_ref, &imag_location_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_real(mem, &func->code, block_id, value_ref, &real_ref));
            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_imaginary(mem, &func->code, block_id, value_ref, &imag_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_store(mem, &func->code, block_id, location_ref, real_ref, &flags, &real_store_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_store(mem, &func->code, block_id, imag_location_ref, imag_ref, &flags, &imag_store_ref));

            kefir_bool_t is_control_flow;
            REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, instr_ref, &is_control_flow));
            if (is_control_flow) {
                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_ref, real_store_ref));
                REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_ref, imag_store_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
            }
        } break;

#define BINARY_OP(_fn) \
        do { \
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0]; \
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1]; \
 \
            kefir_id_t func_decl_id = KEFIR_ID_NONE; \
            if (configuration->decimal_encoding == KEFIR_DECIMAL_ENCODING_BID) { \
                REQUIRE_OK(get_libgcc_bid_##_fn(mem, module, param, &func_decl_id)); \
            } else { \
                REQUIRE_OK(get_libgcc_dpd_##_fn(mem, module, param, &func_decl_id)); \
            } \
 \
            kefir_opt_call_id_t call_node_id; \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, \
                                                        KEFIR_ID_NONE, &call_node_id, replacement_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg2_ref)); \
        } while (0)

        case KEFIR_OPT_OPCODE_DECIMAL32_ADD:
            BINARY_OP(addsd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_ADD:
            BINARY_OP(adddd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_ADD:
            BINARY_OP(addtd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_SUB:
            BINARY_OP(subsd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_SUB:
            BINARY_OP(subdd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_SUB:
            BINARY_OP(subtd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_MUL:
            BINARY_OP(mulsd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_MUL:
            BINARY_OP(muldd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_MUL:
            BINARY_OP(multd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_DIV:
            BINARY_OP(divsd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_DIV:
            BINARY_OP(divdd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_DIV:
            BINARY_OP(divtd3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_EQUAL:
            BINARY_OP(eqsd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_int32_bool_not(mem, &func->code, block_id, *replacement_ref, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_EQUAL:
            BINARY_OP(eqdd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_int32_bool_not(mem, &func->code, block_id, *replacement_ref, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_EQUAL:
            BINARY_OP(eqtd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_int32_bool_not(mem, &func->code, block_id, *replacement_ref, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_GREATER: {
            kefir_opt_instruction_ref_t zero_ref;
            BINARY_OP(gtsd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0, &zero_ref));
            REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, KEFIR_OPT_COMPARISON_INT32_GREATER, *replacement_ref, zero_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL64_GREATER: {
            kefir_opt_instruction_ref_t zero_ref;
            BINARY_OP(gtdd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0, &zero_ref));
            REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, KEFIR_OPT_COMPARISON_INT32_GREATER, *replacement_ref, zero_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL128_GREATER: {
            kefir_opt_instruction_ref_t zero_ref;
            BINARY_OP(gttd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0, &zero_ref));
            REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, KEFIR_OPT_COMPARISON_INT32_GREATER, *replacement_ref, zero_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL32_LESS: {
            kefir_opt_instruction_ref_t zero_ref;
            BINARY_OP(ltsd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0, &zero_ref));
            REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, KEFIR_OPT_COMPARISON_INT32_LESSER, *replacement_ref, zero_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL64_LESS: {
            kefir_opt_instruction_ref_t zero_ref;
            BINARY_OP(ltdd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0, &zero_ref));
            REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, KEFIR_OPT_COMPARISON_INT32_LESSER, *replacement_ref, zero_ref, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL128_LESS: {
            kefir_opt_instruction_ref_t zero_ref;
            BINARY_OP(lttd3_function_decl_id);
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, 0, &zero_ref));
            REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, KEFIR_OPT_COMPARISON_INT32_LESSER, *replacement_ref, zero_ref, replacement_ref));
        } break;

#undef BINARY_OP

#define UNARY_OP(_fn) \
        do { \
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0]; \
 \
            kefir_id_t func_decl_id = KEFIR_ID_NONE; \
            if (configuration->decimal_encoding == KEFIR_DECIMAL_ENCODING_BID) { \
                REQUIRE_OK(get_libgcc_bid_##_fn(mem, module, param, &func_decl_id)); \
            } else { \
                REQUIRE_OK(get_libgcc_dpd_##_fn(mem, module, param, &func_decl_id)); \
            } \
 \
 \
            kefir_opt_call_id_t call_node_id; \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 1, \
                                                        KEFIR_ID_NONE, &call_node_id, replacement_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_ref)); \
        } while (0)

        // Decimal to decimal
        case KEFIR_OPT_OPCODE_DECIMAL32_TO_DECIMAL64:
            UNARY_OP(extendsddd2_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_TO_DECIMAL128:
            UNARY_OP(extendsdtd2_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_TO_DECIMAL128:
            UNARY_OP(extendddtd2_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_TO_DECIMAL32:
            UNARY_OP(truncddsd2_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_TO_DECIMAL32:
            UNARY_OP(trunctdsd2_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_TO_DECIMAL64:
            UNARY_OP(trunctddd2_function_decl_id);
            break;

        // Decimal32 to ...
        case KEFIR_OPT_OPCODE_DECIMAL32_TO_FLOAT32:
            UNARY_OP(truncsdsf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_TO_FLOAT64:
            UNARY_OP(extendsddf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_TO_LONG_DOUBLE:
            UNARY_OP(extendsdxf_function_decl_id);
            break;

        // Decimal64 to ...
        case KEFIR_OPT_OPCODE_DECIMAL64_TO_FLOAT32:
            UNARY_OP(truncddsf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_TO_FLOAT64:
            UNARY_OP(truncdddf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_TO_LONG_DOUBLE:
            UNARY_OP(extendddxf_function_decl_id);
            break;

        // Decimal128 to ...
        case KEFIR_OPT_OPCODE_DECIMAL128_TO_FLOAT32:
            UNARY_OP(trunctdsf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_TO_FLOAT64:
            UNARY_OP(trunctddf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_TO_LONG_DOUBLE:
            UNARY_OP(trunctdxf_function_decl_id);
            break;
            
        // Decimal32 from ...
        case KEFIR_OPT_OPCODE_FLOAT32_TO_DECIMAL32:
            UNARY_OP(extendsfsd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_TO_DECIMAL32:
            UNARY_OP(truncdfsd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_DECIMAL32:
            UNARY_OP(truncxfsd_function_decl_id);
            break;

        // Decimal64 from ...
        case KEFIR_OPT_OPCODE_FLOAT32_TO_DECIMAL64:
            UNARY_OP(extendsfdd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_TO_DECIMAL64:
            UNARY_OP(extenddfdd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_DECIMAL64:
            UNARY_OP(truncxfdd_function_decl_id);
            break;

        // Decimal128 from ...
        case KEFIR_OPT_OPCODE_FLOAT32_TO_DECIMAL128:
            UNARY_OP(extendsftd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_TO_DECIMAL128:
            UNARY_OP(extenddftd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_DECIMAL128:
            UNARY_OP(extendxftd_function_decl_id);
            break;

        // ... to long
        case KEFIR_OPT_OPCODE_DECIMAL32_TO_INT:
            UNARY_OP(fixsddi_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_TO_INT:
            UNARY_OP(fixdddi_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_TO_INT:
            UNARY_OP(fixtddi_function_decl_id);
            break;

        // ... to unsigned long
        case KEFIR_OPT_OPCODE_DECIMAL32_TO_UINT:
            UNARY_OP(fixunssddi_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_TO_UINT:
            UNARY_OP(fixunsdddi_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_TO_UINT:
            UNARY_OP(fixunstddi_function_decl_id);
            break;

        // long from ...
        case KEFIR_OPT_OPCODE_INT_TO_DECIMAL32:
            UNARY_OP(floatdisd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT_TO_DECIMAL64:
            UNARY_OP(floatdidd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT_TO_DECIMAL128:
            UNARY_OP(floatditd_function_decl_id);
            break;

        // unsigned  long from ...
        case KEFIR_OPT_OPCODE_UINT_TO_DECIMAL32:
            UNARY_OP(floatunsdisd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_UINT_TO_DECIMAL64:
            UNARY_OP(floatunsdidd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_UINT_TO_DECIMAL128:
            UNARY_OP(floatunsditd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_TO_DECIMAL32:
            UNARY_OP(floattisd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_TO_DECIMAL64:
            UNARY_OP(floattidd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_TO_DECIMAL128:
            UNARY_OP(floattitd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_TO_DECIMAL32:
            UNARY_OP(floatunstisd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_TO_DECIMAL64:
            UNARY_OP(floatunstidd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_TO_DECIMAL128:
            UNARY_OP(floatunstitd_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_FROM_DECIMAL32:
            UNARY_OP(fixsdti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_FROM_DECIMAL64:
            UNARY_OP(fixddti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_FROM_DECIMAL128:
            UNARY_OP(fixtdti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_FROM_DECIMAL32:
            UNARY_OP(fixunssdti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_FROM_DECIMAL64:
            UNARY_OP(fixunsddti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_FROM_DECIMAL128:
            UNARY_OP(fixunstdti_function_decl_id);
            break;

#undef UNARY_OP

#define BITINT_TO_DEC(_fn, _sign) \
        do { \
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0]; \
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth; \
            const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS; \
 \
            kefir_opt_instruction_ref_t call_node_id, value_ref, value_store_ref, value_pair_ref, bitwidth_ref; \
            REQUIRE_OK(kefir_opt_code_builder_temporary_object( \
                mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref)); \
            REQUIRE_OK(kefir_opt_code_builder_bitint_store(mem, &func->code, block_id, bitwidth, value_ref, arg_ref, &(struct kefir_opt_memory_access_flags){0}, &value_store_ref)); \
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, bitwidth * (_sign), &bitwidth_ref)); \
            REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, value_store_ref, &value_pair_ref)); \
 \
            kefir_id_t func_decl_id = KEFIR_ID_NONE; \
            if (configuration->decimal_encoding == KEFIR_DECIMAL_ENCODING_BID) { \
                REQUIRE_OK(get_libgcc_bid_##_fn(mem, module, param, &func_decl_id)); \
            } else { \
                REQUIRE_OK(get_libgcc_dpd_##_fn(mem, module, param, &func_decl_id)); \
            } \
 \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, \
                                                            KEFIR_ID_NONE, &call_node_id, replacement_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_pair_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref)); \
        } while (0)

        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DECIMAL32:
            if (!configuration->imprecise_decimal_bitint_conv) {
                BITINT_TO_DEC(floatbitintsd_function_decl_id, -1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_signed_to_long_double(mem, &func->code, block_id, instr->operation.parameters.bitwidth, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_decimal32(mem, &func->code, block_id, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DECIMAL32:
            if (!configuration->imprecise_decimal_bitint_conv) {
                BITINT_TO_DEC(floatbitintsd_function_decl_id, 1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_to_long_double(mem, &func->code, block_id, instr->operation.parameters.bitwidth, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_decimal32(mem, &func->code, block_id, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DECIMAL64:
            if (!configuration->imprecise_decimal_bitint_conv) {
                BITINT_TO_DEC(floatbitintdd_function_decl_id, -1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_signed_to_long_double(mem, &func->code, block_id, instr->operation.parameters.bitwidth, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_decimal64(mem, &func->code, block_id, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DECIMAL64:
            if (!configuration->imprecise_decimal_bitint_conv) {
                BITINT_TO_DEC(floatbitintdd_function_decl_id, 1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_to_long_double(mem, &func->code, block_id, instr->operation.parameters.bitwidth, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_decimal64(mem, &func->code, block_id, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DECIMAL128:
            if (!configuration->imprecise_decimal_bitint_conv) {
                BITINT_TO_DEC(floatbitinttd_function_decl_id, -1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_signed_to_long_double(mem, &func->code, block_id, instr->operation.parameters.bitwidth, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_decimal128(mem, &func->code, block_id, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DECIMAL128:
            if (!configuration->imprecise_decimal_bitint_conv) {
                BITINT_TO_DEC(floatbitinttd_function_decl_id, 1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_to_long_double(mem, &func->code, block_id, instr->operation.parameters.bitwidth, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_decimal128(mem, &func->code, block_id, long_double_ref, replacement_ref));
            }
            break;

#undef BITINT_TO_DEC

#define DEC_TO_BITINT(_fn, _sign) \
        do { \
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0]; \
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth; \
            const kefir_size_t qwords = (bitwidth + QWORD_BITS - 1) / QWORD_BITS; \
 \
            kefir_opt_instruction_ref_t call_node_id, value_ref, pair_ref, call_ref, bitwidth_ref; \
            REQUIRE_OK(kefir_opt_code_builder_temporary_object( \
                mem, &func->code, block_id, qwords * KEFIR_AMD64_ABI_QWORD, KEFIR_AMD64_ABI_QWORD, &value_ref)); \
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, bitwidth * (_sign), &bitwidth_ref)); \
 \
            kefir_id_t func_decl_id = KEFIR_ID_NONE; \
            if (configuration->decimal_encoding == KEFIR_DECIMAL_ENCODING_BID) { \
                REQUIRE_OK(get_libgcc_bid_##_fn(mem, module, param, &func_decl_id)); \
            } else { \
                REQUIRE_OK(get_libgcc_dpd_##_fn(mem, module, param, &func_decl_id)); \
            } \
 \
 \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 3, \
                                                            KEFIR_ID_NONE, &call_node_id, &call_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, value_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, bitwidth_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 2, arg_ref)); \
 \
            REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, call_ref, &pair_ref)); \
            REQUIRE_OK(kefir_opt_code_builder_bitint_load(mem, &func->code, block_id, bitwidth, pair_ref, &(struct kefir_opt_memory_access_flags){0}, replacement_ref)); \
        } while (0)

        case KEFIR_OPT_OPCODE_DECIMAL32_TO_BITINT_SIGNED:
            if (!configuration->imprecise_decimal_bitint_conv) {
                DEC_TO_BITINT(fixsdbitint_function_decl_id, -1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                REQUIRE_OK(kefir_opt_code_builder_decimal32_to_long_double(mem, &func->code, block_id, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_bitint_signed(mem, &func->code, block_id, bitwidth, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_TO_BITINT_UNSIGNED:
            if (!configuration->imprecise_decimal_bitint_conv) {
                DEC_TO_BITINT(fixsdbitint_function_decl_id, 1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                REQUIRE_OK(kefir_opt_code_builder_decimal32_to_long_double(mem, &func->code, block_id, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_bitint_unsigned(mem, &func->code, block_id, bitwidth, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_TO_BITINT_SIGNED:
            if (!configuration->imprecise_decimal_bitint_conv) {
                DEC_TO_BITINT(fixddbitint_function_decl_id, -1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                REQUIRE_OK(kefir_opt_code_builder_decimal64_to_long_double(mem, &func->code, block_id, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_bitint_signed(mem, &func->code, block_id, bitwidth, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_TO_BITINT_UNSIGNED:
            if (!configuration->imprecise_decimal_bitint_conv) {
                DEC_TO_BITINT(fixddbitint_function_decl_id, 1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                REQUIRE_OK(kefir_opt_code_builder_decimal64_to_long_double(mem, &func->code, block_id, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_bitint_unsigned(mem, &func->code, block_id, bitwidth, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_TO_BITINT_SIGNED:
            if (!configuration->imprecise_decimal_bitint_conv) {
                DEC_TO_BITINT(fixtdbitint_function_decl_id, -1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                REQUIRE_OK(kefir_opt_code_builder_decimal128_to_long_double(mem, &func->code, block_id, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_bitint_signed(mem, &func->code, block_id, bitwidth, long_double_ref, replacement_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_TO_BITINT_UNSIGNED:
            if (!configuration->imprecise_decimal_bitint_conv) {
                DEC_TO_BITINT(fixtdbitint_function_decl_id, 1);
            } else {
                kefir_opt_instruction_ref_t long_double_ref;
                kefir_size_t bitwidth = instr->operation.parameters.bitwidth;
                REQUIRE_OK(kefir_opt_code_builder_decimal128_to_long_double(mem, &func->code, block_id, instr->operation.parameters.refs[0], &long_double_ref));
                REQUIRE_OK(kefir_opt_code_builder_long_double_to_bitint_unsigned(mem, &func->code, block_id, bitwidth, long_double_ref, replacement_ref));
            }
            break;

#undef DEC_TO_BITINT

        case KEFIR_OPT_OPCODE_DECIMAL32_ATOMIC_LOAD:
            REQUIRE_OK(kefir_opt_code_builder_atomic_load32(mem, &func->code, block_id, instr->operation.parameters.refs[0], instr->operation.parameters.atomic_op.model,
                                                            replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_ATOMIC_LOAD:
            REQUIRE_OK(kefir_opt_code_builder_atomic_load64(mem, &func->code, block_id, instr->operation.parameters.refs[0], instr->operation.parameters.atomic_op.model,
                                                            replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_ATOMIC_LOAD: {
            kefir_opt_instruction_ref_t instr_id = instr->id;
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.atomic_op.model;

            kefir_id_t type_id;
            struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 1, &type_id);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_DECIMAL128, 0, 0));

            kefir_opt_instruction_ref_t value_ref, copy_ref, pair_ref;
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &value_ref));
            REQUIRE_OK(kefir_opt_code_builder_atomic_copy_memory_from(
                mem, &func->code, block_id, value_ref, location_arg_ref, memorder, type_id, 0, &copy_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, copy_ref, &pair_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_decimal128_load(mem, &func->code, block_id, pair_ref, &(struct kefir_opt_memory_access_flags){0}, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, copy_ref));
            REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL32_ATOMIC_STORE:
            REQUIRE_OK(kefir_opt_code_builder_atomic_store32(mem, &func->code, block_id, instr->operation.parameters.refs[0], instr->operation.parameters.refs[1], instr->operation.parameters.atomic_op.model,
                                                            replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_ATOMIC_STORE:
            REQUIRE_OK(kefir_opt_code_builder_atomic_store64(mem, &func->code, block_id, instr->operation.parameters.refs[0], instr->operation.parameters.refs[1], instr->operation.parameters.atomic_op.model,
                                                            replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_ATOMIC_STORE: {
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t value_arg_ref = instr->operation.parameters.refs[1];
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.atomic_op.model;

            kefir_id_t type_id;
            struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 1, &type_id);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_DECIMAL128, 0, 0));

            kefir_opt_instruction_ref_t value_ref, copy_ref, pair_ref;
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &value_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_decimal128_store(mem, &func->code, block_id, value_ref, value_arg_ref, &(struct kefir_opt_memory_access_flags){0}, &copy_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, copy_ref, &pair_ref));
            REQUIRE_OK(kefir_opt_code_builder_atomic_copy_memory_to(
                mem, &func->code, block_id, location_arg_ref, pair_ref, memorder, type_id, 0, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_DECIMAL32_ATOMIC_CMPXCHG:
            REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange32(mem, &func->code, block_id, instr->operation.parameters.refs[0], instr->operation.parameters.refs[1], instr->operation.parameters.refs[2], instr->operation.parameters.atomic_op.model,
                                                            replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_ATOMIC_CMPXCHG:
            REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange64(mem, &func->code, block_id, instr->operation.parameters.refs[0], instr->operation.parameters.refs[1], instr->operation.parameters.refs[2], instr->operation.parameters.atomic_op.model,
                                                            replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_ATOMIC_CMPXCHG: {
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t compare_value_arg_ref = instr->operation.parameters.refs[1];
            const kefir_opt_instruction_ref_t new_value_arg_ref = instr->operation.parameters.refs[2];
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.atomic_op.model;

            kefir_id_t type_id;
            struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 1, &type_id);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_DECIMAL128, 0, 0));

            kefir_opt_instruction_ref_t compare_value_ref, compare_value_copy_ref, compare_value_pair_ref;
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &compare_value_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_decimal128_store(mem, &func->code, block_id, compare_value_ref, compare_value_arg_ref, &(struct kefir_opt_memory_access_flags){0}, &compare_value_copy_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_pair(mem, &func->code, block_id, compare_value_ref, compare_value_copy_ref, &compare_value_pair_ref));

            kefir_opt_instruction_ref_t new_value_ref, new_value_copy_ref, new_value_pair_ref;
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &new_value_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_decimal128_store(mem, &func->code, block_id, new_value_ref, new_value_arg_ref, &(struct kefir_opt_memory_access_flags){0}, &new_value_copy_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_pair(mem, &func->code, block_id, new_value_ref, new_value_copy_ref, &new_value_pair_ref));

            REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange_memory(
                mem, &func->code, block_id, location_arg_ref, compare_value_pair_ref, new_value_pair_ref, memorder,
                type_id, 0, replacement_ref));
        } break;

#define ISNAN_IMPL(_fn) \
        do { \
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0]; \
 \
            kefir_id_t func_decl_id = KEFIR_ID_NONE; \
            if (configuration->decimal_encoding == KEFIR_DECIMAL_ENCODING_BID) { \
                REQUIRE_OK(get_libgcc_bid_##_fn(mem, module, param, &func_decl_id)); \
            } else { \
                REQUIRE_OK(get_libgcc_dpd_##_fn(mem, module, param, &func_decl_id)); \
            } \
 \
 \
            kefir_opt_call_id_t call_node_id; \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, \
                                                        KEFIR_ID_NONE, &call_node_id, replacement_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg1_ref)); \
        } while (0)

        case KEFIR_OPT_OPCODE_DECIMAL32_ISNAN:
            ISNAN_IMPL(unordsd2_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_ISNAN:
            ISNAN_IMPL(unorddd2_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_ISNAN:
            ISNAN_IMPL(unordtd2_function_decl_id);
            break;

#undef ISNAN_IMPL

#define INT128_BIN_OP(_fn) \
        do { \
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0]; \
            const kefir_opt_instruction_ref_t arg2_ref = instr->operation.parameters.refs[1]; \
 \
            kefir_id_t func_decl_id = KEFIR_ID_NONE; \
            REQUIRE_OK(get_libgcc_##_fn(mem, module, param, &func_decl_id)); \
 \
 \
            kefir_opt_call_id_t call_node_id; \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 2, \
                                                        KEFIR_ID_NONE, &call_node_id, replacement_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 1, arg2_ref)); \
        } while (0)

        case KEFIR_OPT_OPCODE_INT128_IDIV:
            INT128_BIN_OP(divti3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UDIV:
            INT128_BIN_OP(udivti3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_IMOD:
            INT128_BIN_OP(modti3_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UMOD:
            INT128_BIN_OP(umodti3_function_decl_id);
            break;

#undef INT128_BIN_OP

#define INT128_UN_OP(_fn) \
        do { \
            const kefir_opt_instruction_ref_t arg1_ref = instr->operation.parameters.refs[0]; \
 \
            kefir_id_t func_decl_id = KEFIR_ID_NONE; \
            REQUIRE_OK(get_libgcc_##_fn(mem, module, param, &func_decl_id)); \
 \
 \
            kefir_opt_call_id_t call_node_id; \
            REQUIRE_OK(kefir_opt_code_container_new_call(mem, &func->code, block_id, func_decl_id, 1, \
                                                        KEFIR_ID_NONE, &call_node_id, replacement_ref)); \
            REQUIRE_OK(kefir_opt_code_container_call_set_argument(mem, &func->code, call_node_id, 0, arg1_ref)); \
        } while (0)

        case KEFIR_OPT_OPCODE_INT128_SIGNED_TO_FLOAT32:
            INT128_UN_OP(floattisf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_TO_FLOAT64:
            INT128_UN_OP(floattidf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_TO_LONG_DOUBLE:
            INT128_UN_OP(floattixf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_TO_FLOAT32:
            INT128_UN_OP(floatuntisf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_TO_FLOAT64:
            INT128_UN_OP(floatuntidf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_TO_LONG_DOUBLE:
            INT128_UN_OP(floatuntixf_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_FROM_FLOAT32:
            INT128_UN_OP(fixsfti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_FROM_FLOAT64:
            INT128_UN_OP(fixdfti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_FROM_LONG_DOUBLE:
            INT128_UN_OP(fixxfti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_FROM_FLOAT32:
            INT128_UN_OP(fixunssfti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_FROM_FLOAT64:
            INT128_UN_OP(fixunsdfti_function_decl_id);
            break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_FROM_LONG_DOUBLE:
            INT128_UN_OP(fixunsxfti_function_decl_id);
            break;

#undef INT128_UN_OP

        case KEFIR_OPT_OPCODE_INT128_ATOMIC_LOAD: {
            kefir_opt_instruction_ref_t instr_id = instr->id;
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.atomic_op.model;

            kefir_id_t type_id;
            struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 1, &type_id);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_DECIMAL128, 0, 0));

            kefir_opt_instruction_ref_t value_ref, copy_ref, pair_ref;
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &value_ref));
            REQUIRE_OK(kefir_opt_code_builder_atomic_copy_memory_from(
                mem, &func->code, block_id, value_ref, location_arg_ref, memorder, type_id, 0, &copy_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, copy_ref, &pair_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int128_load(mem, &func->code, block_id, pair_ref, &(struct kefir_opt_memory_access_flags){0}, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block_id, instr_id, copy_ref));
            REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
        } break;

        case KEFIR_OPT_OPCODE_INT128_ATOMIC_STORE: {
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t value_arg_ref = instr->operation.parameters.refs[1];
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.atomic_op.model;

            kefir_id_t type_id;
            struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 1, &type_id);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_DECIMAL128, 0, 0));

            kefir_opt_instruction_ref_t value_ref, copy_ref, pair_ref;
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &value_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int128_store(mem, &func->code, block_id, value_ref, value_arg_ref, &(struct kefir_opt_memory_access_flags){0}, &copy_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, copy_ref, &pair_ref));
            REQUIRE_OK(kefir_opt_code_builder_atomic_copy_memory_to(
                mem, &func->code, block_id, location_arg_ref, pair_ref, memorder, type_id, 0, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_INT128_ATOMIC_CMPXCHG: {
            const kefir_opt_instruction_ref_t location_arg_ref = instr->operation.parameters.refs[0];
            const kefir_opt_instruction_ref_t compare_value_arg_ref = instr->operation.parameters.refs[1];
            const kefir_opt_instruction_ref_t new_value_arg_ref = instr->operation.parameters.refs[2];
            const kefir_opt_memory_order_t memorder = instr->operation.parameters.atomic_op.model;

            kefir_id_t type_id;
            struct kefir_ir_type *type = kefir_ir_module_new_type(mem, module->ir_module, 1, &type_id);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_DECIMAL128, 0, 0));

            kefir_opt_instruction_ref_t compare_value_ref, compare_value_copy_ref, compare_value_pair_ref;
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &compare_value_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int128_store(mem, &func->code, block_id, compare_value_ref, compare_value_arg_ref, &(struct kefir_opt_memory_access_flags){0}, &compare_value_copy_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_pair(mem, &func->code, block_id, compare_value_ref, compare_value_copy_ref, &compare_value_pair_ref));

            kefir_opt_instruction_ref_t new_value_ref, new_value_copy_ref, new_value_pair_ref;
            REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &new_value_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int128_store(mem, &func->code, block_id, new_value_ref, new_value_arg_ref, &(struct kefir_opt_memory_access_flags){0}, &new_value_copy_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_pair(mem, &func->code, block_id, new_value_ref, new_value_copy_ref, &new_value_pair_ref));

            REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange_memory(
                mem, &func->code, block_id, location_arg_ref, compare_value_pair_ref, new_value_pair_ref, memorder,
                type_id, 0, replacement_ref));
        } break;

        case KEFIR_OPT_OPCODE_INT128_SIGNED_TO_BITINT: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= 8 * KEFIR_AMD64_ABI_QWORD) {
                REQUIRE_OK(kefir_opt_code_builder_int128_lower_half(mem, &func->code, block_id, arg_ref, replacement_ref));
            } else {
                kefir_opt_instruction_ref_t value_ref, value_store_ref, value_store_pair_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_int128_store(mem, &func->code, block_id, value_ref, arg_ref, &(struct kefir_opt_memory_access_flags){0}, &value_store_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, value_store_ref, &value_store_pair_ref));
                REQUIRE_OK(kefir_opt_code_builder_bitint_cast_signed(mem, &func->code, block_id, bitwidth, 2 * 8 * KEFIR_AMD64_ABI_QWORD, value_store_pair_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_INT128_UNSIGNED_TO_BITINT: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth <= 8 * KEFIR_AMD64_ABI_QWORD) {
                REQUIRE_OK(kefir_opt_code_builder_int128_lower_half(mem, &func->code, block_id, arg_ref, replacement_ref));
            } else {
                kefir_opt_instruction_ref_t value_ref, value_store_ref, value_store_pair_ref;
                REQUIRE_OK(kefir_opt_code_builder_temporary_object(
                    mem, &func->code, block_id, 2 * KEFIR_AMD64_ABI_QWORD, 2 * KEFIR_AMD64_ABI_QWORD, &value_ref));
                REQUIRE_OK(kefir_opt_code_builder_int128_store(mem, &func->code, block_id, value_ref, arg_ref, &(struct kefir_opt_memory_access_flags){0}, &value_store_ref));
                REQUIRE_OK(kefir_opt_code_builder_pair(mem, &func->code, block_id, value_ref, value_store_ref, &value_store_pair_ref));
                REQUIRE_OK(kefir_opt_code_builder_bitint_cast_unsigned(mem, &func->code, block_id, bitwidth, 2 * 8 * KEFIR_AMD64_ABI_QWORD, value_store_pair_ref, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_INT128_FROM_BITINT_SIGNED: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth < 8 * KEFIR_AMD64_ABI_QWORD) {
                kefir_opt_instruction_ref_t int64_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_get_signed(mem, &func->code, block_id, bitwidth, arg_ref, &int64_ref));
                REQUIRE_OK(kefir_opt_code_builder_int128_sign_extend_64bits(mem, &func->code, block_id, int64_ref, replacement_ref));
            } else {
                kefir_opt_instruction_ref_t int128_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_cast_signed(mem, &func->code, block_id, bitwidth, 2 * 8 * KEFIR_AMD64_ABI_QWORD, arg_ref, &int128_ref));
                REQUIRE_OK(kefir_opt_code_builder_int128_load(mem, &func->code, block_id, int128_ref, &(struct kefir_opt_memory_access_flags){0}, replacement_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_INT128_FROM_BITINT_UNSIGNED: {
            const kefir_opt_instruction_ref_t arg_ref = instr->operation.parameters.refs[0];
            const kefir_size_t bitwidth = instr->operation.parameters.bitwidth;

            if (bitwidth < 8 * KEFIR_AMD64_ABI_QWORD) {
                kefir_opt_instruction_ref_t int64_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_get_unsigned(mem, &func->code, block_id, bitwidth, arg_ref, &int64_ref));
                REQUIRE_OK(kefir_opt_code_builder_int128_zero_extend_64bits(mem, &func->code, block_id, int64_ref, replacement_ref));
            } else {
                kefir_opt_instruction_ref_t int128_ref;
                REQUIRE_OK(kefir_opt_code_builder_bitint_cast_unsigned(mem, &func->code, block_id, bitwidth, 2 * 8 * KEFIR_AMD64_ABI_QWORD, arg_ref, &int128_ref));
                REQUIRE_OK(kefir_opt_code_builder_int128_load(mem, &func->code, block_id, int128_ref, &(struct kefir_opt_memory_access_flags){0}, replacement_ref));
            }
        } break;

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t lower_function(struct kefir_mem *mem, struct kefir_opt_module *module,
                                     struct kefir_opt_function *func, const struct kefir_optimizer_configuration *configuration, struct lowering_param *param) {
    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL;

        for (kefir_opt_code_block_instr_head(&func->code, block, &instr_id); instr_id != KEFIR_ID_NONE;) {
            REQUIRE_OK(kefir_opt_code_debug_info_next_instruction_code_reference_of(&func->debug_info, instr_id));
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));

            kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
            REQUIRE_OK(lower_instruction(mem, module, func, configuration, param, instr, &replacement_ref));

            if (replacement_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr->id));
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

            REQUIRE_OK(kefir_opt_code_debug_info_next_instruction_code_reference(
                &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_lower_function(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                  struct kefir_opt_function *func, const struct kefir_optimizer_configuration *configuration) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid optimizer function"));
    REQUIRE(configuration != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid optimizer configuration"));

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
                                                  .bigint_redundant_sign_bits = KEFIR_ID_NONE,
                                                  .bigint_nonzero_count = KEFIR_ID_NONE,
                                                  .bigint_parity = KEFIR_ID_NONE,
                                                  .builtin_ffs = KEFIR_ID_NONE,
                                                  .builtin_ffsl = KEFIR_ID_NONE,
                                                  .builtin_clz = KEFIR_ID_NONE,
                                                  .builtin_clzl = KEFIR_ID_NONE,
                                                  .builtin_ctz = KEFIR_ID_NONE,
                                                  .builtin_ctzl = KEFIR_ID_NONE,
                                                  .builtin_clrsb = KEFIR_ID_NONE,
                                                  .builtin_clrsbl = KEFIR_ID_NONE,
                                                  .builtin_popcount = KEFIR_ID_NONE,
                                                  .builtin_popcountl = KEFIR_ID_NONE,
                                                  .builtin_parity = KEFIR_ID_NONE,
                                                  .builtin_parityl = KEFIR_ID_NONE,
                                                  .sofxfloat_complex_float_mul = KEFIR_ID_NONE,
                                                  .sofxfloat_complex_double_mul = KEFIR_ID_NONE,
                                                  .sofxfloat_complex_long_double_mul = KEFIR_ID_NONE,
                                                  .sofxfloat_complex_float_div = KEFIR_ID_NONE,
                                                  .sofxfloat_complex_double_div = KEFIR_ID_NONE,
                                                  .sofxfloat_complex_long_double_div = KEFIR_ID_NONE,

                                                  .libgcc_udivti3 = KEFIR_ID_NONE,
                                                  .libgcc_divti3 = KEFIR_ID_NONE,
                                                  .libgcc_umodti3 = KEFIR_ID_NONE,
                                                  .libgcc_modti3 = KEFIR_ID_NONE,

                                                  .libgcc_floattisf = KEFIR_ID_NONE,
                                                  .libgcc_floattidf = KEFIR_ID_NONE,
                                                  .libgcc_floattixf = KEFIR_ID_NONE,
                                                  .libgcc_floatuntisf = KEFIR_ID_NONE,
                                                  .libgcc_floatuntidf = KEFIR_ID_NONE,
                                                  .libgcc_floatuntixf = KEFIR_ID_NONE,

                                                  .libgcc_fixsfti = KEFIR_ID_NONE,
                                                  .libgcc_fixdfti = KEFIR_ID_NONE,
                                                  .libgcc_fixxfti = KEFIR_ID_NONE,
                                                  .libgcc_fixunssfti = KEFIR_ID_NONE,
                                                  .libgcc_fixunsdfti = KEFIR_ID_NONE,
                                                  .libgcc_fixunsxfti = KEFIR_ID_NONE,

#define INIT_DECIMAL_FNS(_prefix) \
                                                ._prefix##_addsd3 = KEFIR_ID_NONE, \
                                                ._prefix##_adddd3 = KEFIR_ID_NONE, \
                                                ._prefix##_addtd3 = KEFIR_ID_NONE, \
                                                ._prefix##_subsd3 = KEFIR_ID_NONE, \
                                                ._prefix##_subdd3 = KEFIR_ID_NONE, \
                                                ._prefix##_subtd3 = KEFIR_ID_NONE, \
                                                ._prefix##_mulsd3 = KEFIR_ID_NONE, \
                                                ._prefix##_muldd3 = KEFIR_ID_NONE, \
                                                ._prefix##_multd3 = KEFIR_ID_NONE, \
                                                ._prefix##_divsd3 = KEFIR_ID_NONE, \
                                                ._prefix##_divdd3 = KEFIR_ID_NONE, \
                                                ._prefix##_divtd3 = KEFIR_ID_NONE, \
                                                ._prefix##_eqsd3 = KEFIR_ID_NONE, \
                                                ._prefix##_eqdd3 = KEFIR_ID_NONE, \
                                                ._prefix##_eqtd3 = KEFIR_ID_NONE, \
                                                ._prefix##_gtsd3 = KEFIR_ID_NONE, \
                                                ._prefix##_gtdd3 = KEFIR_ID_NONE, \
                                                ._prefix##_gttd3 = KEFIR_ID_NONE, \
                                                ._prefix##_ltsd3 = KEFIR_ID_NONE, \
                                                ._prefix##_ltdd3 = KEFIR_ID_NONE, \
                                                ._prefix##_lttd3 = KEFIR_ID_NONE, \
                                                /* Decimal to decimal */ \
                                                ._prefix##_extendsddd2 = KEFIR_ID_NONE, \
                                                ._prefix##_extendsdtd2 = KEFIR_ID_NONE, \
                                                ._prefix##_extendddtd2 = KEFIR_ID_NONE, \
                                                ._prefix##_truncddsd2 = KEFIR_ID_NONE, \
                                                ._prefix##_trunctdsd2 = KEFIR_ID_NONE, \
                                                ._prefix##_trunctddd2 = KEFIR_ID_NONE, \
                                                /* Decimal32 to ... */ \
                                                ._prefix##_truncsdsf = KEFIR_ID_NONE, \
                                                ._prefix##_extendsddf = KEFIR_ID_NONE, \
                                                ._prefix##_extendsdxf = KEFIR_ID_NONE, \
                                                /* Decimal64 to ... */ \
                                                ._prefix##_truncddsf = KEFIR_ID_NONE, \
                                                ._prefix##_truncdddf = KEFIR_ID_NONE, \
                                                ._prefix##_extendddxf = KEFIR_ID_NONE, \
                                                /* Decimal128 to ... */ \
                                                ._prefix##_trunctdsf = KEFIR_ID_NONE, \
                                                ._prefix##_trunctddf = KEFIR_ID_NONE, \
                                                ._prefix##_trunctdxf = KEFIR_ID_NONE, \
                                                /* Decimal32 from ... */ \
                                                ._prefix##_extendsfsd = KEFIR_ID_NONE, \
                                                ._prefix##_truncdfsd = KEFIR_ID_NONE, \
                                                ._prefix##_truncxfsd = KEFIR_ID_NONE, \
                                                /* Decimal64 from ... */ \
                                                ._prefix##_extendsfdd = KEFIR_ID_NONE, \
                                                ._prefix##_extenddfdd = KEFIR_ID_NONE, \
                                                ._prefix##_truncxfdd = KEFIR_ID_NONE, \
                                                /* Decimal128 from ... */ \
                                                ._prefix##_extendsftd = KEFIR_ID_NONE, \
                                                ._prefix##_extenddftd = KEFIR_ID_NONE, \
                                                ._prefix##_extendxftd = KEFIR_ID_NONE, \
                                                /* ... to long */ \
                                                ._prefix##_fixsddi = KEFIR_ID_NONE, \
                                                ._prefix##_fixdddi = KEFIR_ID_NONE, \
                                                ._prefix##_fixtddi = KEFIR_ID_NONE, \
                                                /* ... to unsigned long */ \
                                                ._prefix##_fixunssddi = KEFIR_ID_NONE, \
                                                ._prefix##_fixunsdddi = KEFIR_ID_NONE, \
                                                ._prefix##_fixunstddi = KEFIR_ID_NONE, \
                                                /* long from ... */ \
                                                ._prefix##_floatdisd = KEFIR_ID_NONE, \
                                                ._prefix##_floatdidd = KEFIR_ID_NONE, \
                                                ._prefix##_floatditd = KEFIR_ID_NONE, \
                                                /* unsigned  long from ... */ \
                                                ._prefix##_floatunsdisd = KEFIR_ID_NONE, \
                                                ._prefix##_floatunsdidd = KEFIR_ID_NONE, \
                                                ._prefix##_floatunsditd = KEFIR_ID_NONE, \
                                                ._prefix##_floatbitintsd = KEFIR_ID_NONE, \
                                                ._prefix##_floatbitintdd = KEFIR_ID_NONE, \
                                                ._prefix##_floatbitinttd = KEFIR_ID_NONE, \
                                                ._prefix##_fixsdbitint = KEFIR_ID_NONE, \
                                                ._prefix##_fixddbitint = KEFIR_ID_NONE, \
                                                ._prefix##_fixtdbitint = KEFIR_ID_NONE, \
                                                ._prefix##_unordsd2 = KEFIR_ID_NONE, \
                                                ._prefix##_unorddd2 = KEFIR_ID_NONE, \
                                                ._prefix##_unordtd2 = KEFIR_ID_NONE, \
                                                ._prefix##_floattisd = KEFIR_ID_NONE, \
                                                ._prefix##_floattidd = KEFIR_ID_NONE, \
                                                ._prefix##_floattitd = KEFIR_ID_NONE, \
                                                ._prefix##_floatunstisd = KEFIR_ID_NONE, \
                                                ._prefix##_floatunstidd = KEFIR_ID_NONE, \
                                                ._prefix##_floatunstitd = KEFIR_ID_NONE, \
                                                ._prefix##_fixsdti = KEFIR_ID_NONE, \
                                                ._prefix##_fixddti = KEFIR_ID_NONE, \
                                                ._prefix##_fixtdti = KEFIR_ID_NONE, \
                                                ._prefix##_fixunssdti = KEFIR_ID_NONE, \
                                                ._prefix##_fixunsddti = KEFIR_ID_NONE, \
                                                ._prefix##_fixunstdti = KEFIR_ID_NONE
                                                INIT_DECIMAL_FNS(libgcc_bid),
                                                INIT_DECIMAL_FNS(libgcc_dpd)
#undef INIT_DECIMAL_FNS
                                            }};
    REQUIRE_OK(lower_function(mem, module, func, configuration, &param));
    return KEFIR_OK;
}
