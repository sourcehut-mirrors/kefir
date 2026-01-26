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

#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ir/module.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t obtain_real_part(const struct kefir_ast_type_traits *type_traits, struct kefir_irbuilder_block *builder, const struct kefir_ast_type **origin) {
    kefir_ast_type_data_model_classification_t classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, *origin, &classification));
    if (KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(*origin)) {
        kefir_ast_type_data_model_classification_t classification;
        REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, *origin, &classification));
        switch (classification) {
            case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:{
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, 0.0f, 0.0f));
                *origin = kefir_ast_type_corresponding_real_type(*origin);
                REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
            } break;

            case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE: {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, 0.0));
                *origin = kefir_ast_type_corresponding_real_type(*origin);
                REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
            } break;

            case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:{
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST, 0.0));
                *origin = kefir_ast_type_corresponding_real_type(*origin);
                REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
            } break;

            default:
                // Intentionally left blank
                break;
        }
    } else {
        switch (classification) {
            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:{
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_REAL, 0));
                *origin = kefir_ast_type_corresponding_real_type(*origin);
                REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
            } break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE: {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_REAL, 0));
                *origin = kefir_ast_type_corresponding_real_type(*origin);
                REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
            } break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:{
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_LOAD, 0));
                *origin = kefir_ast_type_corresponding_real_type(*origin);
                REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
            } break;

            default:
                // Intentionally left blank
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_float32(struct kefir_mem *mem, struct kefir_ir_module *module,
                                      struct kefir_irbuilder_block *builder,
                                      const struct kefir_ast_type_traits *type_traits,
                                      const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(type_traits, builder, &origin));

    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_SIGNED_TO_FLOAT,
                                                       origin->bitprecise.width));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UNSIGNED_TO_FLOAT,
                                                       origin->bitprecise.width));
        }
    } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(origin)) {
        switch (origin->tag) {
            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_TO_FLOAT32,
                                                        origin->bitprecise.width));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_TO_FLOAT32,
                                                        origin->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
        }
    } else if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_signed_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_TO_FLOAT32, 0));
        } else {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_unsigned_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT_TO_FLOAT32, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32) {
        // Intentionally left blank
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_FLOAT32, 0));
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin) || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT32, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_FLOAT32, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_FLOAT32, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_FLOAT32, 0));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to floating-point values");
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_float64(struct kefir_mem *mem, struct kefir_ir_module *module,
                                      struct kefir_irbuilder_block *builder,
                                      const struct kefir_ast_type_traits *type_traits,
                                      const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(type_traits, builder, &origin));

    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_SIGNED_TO_DOUBLE,
                                                       origin->bitprecise.width));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UNSIGNED_TO_DOUBLE,
                                                       origin->bitprecise.width));
        }
    } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(origin)) {
        switch (origin->tag) {
            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_TO_FLOAT64,
                                                        origin->bitprecise.width));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_TO_FLOAT64,
                                                        origin->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
        }
    } else if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_signed_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_TO_FLOAT64, 0));
        } else {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_unsigned_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT_TO_FLOAT64, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_FLOAT64, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32) {
        // Intentionally left blank
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin) || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT64, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_FLOAT64, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_FLOAT64, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_FLOAT64, 0));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to floating-point values");
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_long_double(struct kefir_mem *mem, struct kefir_ir_module *module,
                                          struct kefir_irbuilder_block *builder,
                                          const struct kefir_ast_type_traits *type_traits,
                                          const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(type_traits, builder, &origin));
    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE,
                                                       origin->bitprecise.width));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE,
                                                       origin->bitprecise.width));
        }
    } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(origin)) {
        switch (origin->tag) {
            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_TO_LONG_DOUBLE,
                                                        origin->bitprecise.width));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_TO_LONG_DOUBLE,
                                                        origin->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
        }
    } else if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_signed_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_TO_LONG_DOUBLE, 0));
        } else {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_signed_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT_TO_LONG_DOUBLE, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_LONG_DOUBLE, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_LONG_DOUBLE, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64) {
        // Intentionally left blank
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_LONG_DOUBLE, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_LONG_DOUBLE, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_LONG_DOUBLE, 0));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to floating-point values");
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_decimal32(struct kefir_mem *mem, struct kefir_ir_module *module,
                                        struct kefir_irbuilder_block *builder,
                                        const struct kefir_ast_type_traits *type_traits,
                                        const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(type_traits, builder, &origin));

    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_SIGNED_TO_DECIMAL32,
                                                       origin->bitprecise.width));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UNSIGNED_TO_DECIMAL32,
                                                       origin->bitprecise.width));
        }
    } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(origin)) {
        switch (origin->tag) {
            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_TO_DECIMAL32,
                                                        origin->bitprecise.width));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_TO_DECIMAL32,
                                                        origin->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
        }
    } else if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_signed_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_TO_DECIMAL32, 0));
        } else {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_unsigned_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT_TO_DECIMAL32, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_DECIMAL32, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_DECIMAL32, 0));
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin) || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_DECIMAL32, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32) {
        // Intentionally left blank
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_DECIMAL32, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_DECIMAL32, 0));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to decimal floating-point values");
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_decimal64(struct kefir_mem *mem, struct kefir_ir_module *module,
                                        struct kefir_irbuilder_block *builder,
                                        const struct kefir_ast_type_traits *type_traits,
                                        const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(type_traits, builder, &origin));

    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_SIGNED_TO_DECIMAL64,
                                                       origin->bitprecise.width));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UNSIGNED_TO_DECIMAL64,
                                                       origin->bitprecise.width));
        }
    } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(origin)) {
        switch (origin->tag) {
            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_TO_DECIMAL64,
                                                        origin->bitprecise.width));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_TO_DECIMAL64,
                                                        origin->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
        }
    } else if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_signed_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_TO_DECIMAL64, 0));
        } else {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_unsigned_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT_TO_DECIMAL64, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_DECIMAL64, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_DECIMAL64, 0));
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin) || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_DECIMAL64, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_DECIMAL64, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64) {
        // Intentionally left blank
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_DECIMAL64, 0));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to decimal floating-point values");
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_decimal128(struct kefir_mem *mem, struct kefir_ir_module *module,
                                         struct kefir_irbuilder_block *builder,
                                         const struct kefir_ast_type_traits *type_traits,
                                         const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(type_traits, builder, &origin));

    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_SIGNED_TO_DECIMAL128,
                                                       origin->bitprecise.width));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UNSIGNED_TO_DECIMAL128,
                                                       origin->bitprecise.width));
        }
    } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(origin)) {
        switch (origin->tag) {
            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_TO_DECIMAL128,
                                                        origin->bitprecise.width));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_TO_DECIMAL128,
                                                        origin->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
        }
    } else if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_signed_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_TO_DECIMAL128, 0));
        } else {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, module, builder, type_traits, origin,
                                                    kefir_ast_type_unsigned_long_long()));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT_TO_DECIMAL128, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_DECIMAL128, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_DECIMAL128, 0));
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin) || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_DECIMAL128, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_DECIMAL128, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_DECIMAL128, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64) {
        // Intentionally left blank
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to decimal floating-point values");
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_complex_float(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            struct kefir_irbuilder_block *builder,
                                            const struct kefir_ast_type_traits *type_traits,
                                            const struct kefir_ast_type *origin) {
    kefir_id_t complex_float_type_id;
    struct kefir_ir_type *complex_float_type = kefir_ir_module_new_type(mem, module, 1, &complex_float_type_id);
    REQUIRE(complex_float_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));

    REQUIRE_OK(kefir_ir_type_append(complex_float_type, KEFIR_IR_TYPE_COMPLEX_FLOAT32, 0, 0));

    kefir_ast_type_data_model_classification_t classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &classification));
    switch (classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            // Intentionally left blank
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_REAL, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_FLOAT32, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_FLOAT32, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_FROM, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_REAL, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT32, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT32, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_FROM, 0));
            break;

        default:
            if (KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(origin)) {
                REQUIRE_OK(cast_to_float32(mem, module, builder, type_traits, origin->imaginary.real_type));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, 0.0f, 0.0f));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_FROM, 0));
            } else {
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to complex float"));
                REQUIRE_OK(cast_to_float32(mem, module, builder, type_traits, origin));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, 0.0f, 0.0f));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_FROM, 0));
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_complex_double(struct kefir_mem *mem, struct kefir_ir_module *module,
                                             struct kefir_irbuilder_block *builder,
                                             const struct kefir_ast_type_traits *type_traits,
                                             const struct kefir_ast_type *origin) {
    kefir_id_t complex_float_type_id;
    struct kefir_ir_type *complex_float_type = kefir_ir_module_new_type(mem, module, 1, &complex_float_type_id);
    REQUIRE(complex_float_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));

    REQUIRE_OK(kefir_ir_type_append(complex_float_type, KEFIR_IR_TYPE_COMPLEX_FLOAT64, 0, 0));

    kefir_ast_type_data_model_classification_t classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &classification));
    switch (classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_REAL, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_FLOAT64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_FLOAT64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            // Intentionally left blank
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_REAL, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
            break;

        default:
            if (KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(origin)) {
                REQUIRE_OK(cast_to_float64(mem, module, builder, type_traits, origin->imaginary.real_type));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, 0.0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
            } else {
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to complex float"));
                REQUIRE_OK(cast_to_float64(mem, module, builder, type_traits, origin));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, 0.0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_complex_long_double(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                  struct kefir_irbuilder_block *builder,
                                                  const struct kefir_ast_type_traits *type_traits,
                                                  const struct kefir_ast_type *origin) {
    kefir_id_t complex_float_type_id;
    struct kefir_ir_type *complex_float_type = kefir_ir_module_new_type(mem, module, 1, &complex_float_type_id);
    REQUIRE(complex_float_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));

    REQUIRE_OK(kefir_ir_type_append(complex_float_type, KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE, 0, 0));

    kefir_ast_type_data_model_classification_t classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &classification));
    switch (classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_REAL, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_LONG_DOUBLE, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_LONG_DOUBLE, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_REAL, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_LONG_DOUBLE, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_LONG_DOUBLE, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            // Intentionally left blank
            break;

        default:
            if (KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(origin)) {
                REQUIRE_OK(cast_to_long_double(mem, module, builder, type_traits, origin->imaginary.real_type));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST, 0.0L));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
            } else {
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to complex float"));
                REQUIRE_OK(cast_to_long_double(mem, module, builder, type_traits, origin));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST, 0.0L));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_imaginary_float(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            struct kefir_irbuilder_block *builder,
                                            const struct kefir_ast_type_traits *type_traits,
                                            const struct kefir_ast_type *origin) {
    kefir_ast_type_data_model_classification_t classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &classification));
    switch (classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_IMAGINARY, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_FLOAT32, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT32, 0));
            break;

        default:
            if (KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(origin)) {
                REQUIRE_OK(cast_to_float32(mem, module, builder, type_traits, origin->imaginary.real_type));
            } else {
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to imaginary float"));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, 0.0f, 0.0f));
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_imaginary_double(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            struct kefir_irbuilder_block *builder,
                                            const struct kefir_ast_type_traits *type_traits,
                                            const struct kefir_ast_type *origin) {
    kefir_ast_type_data_model_classification_t classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &classification));
    switch (classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_FLOAT64, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_IMAGINARY, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT64, 0));
            break;

        default:
            if (KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(origin)) {
                REQUIRE_OK(cast_to_float64(mem, module, builder, type_traits, origin->imaginary.real_type));
            } else {
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to imaginary double"));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, 0.0));
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_imaginary_long_double(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            struct kefir_irbuilder_block *builder,
                                            const struct kefir_ast_type_traits *type_traits,
                                            const struct kefir_ast_type *origin) {
    kefir_ast_type_data_model_classification_t classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &classification));
    switch (classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_LONG_DOUBLE, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_IMAGINARY, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_LONG_DOUBLE, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_IMAGINARY, 0));
            break;

        default:
            if (KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(origin)) {
                REQUIRE_OK(cast_to_long_double(mem, module, builder, type_traits, origin->imaginary.real_type));
            } else {
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to imaginary long double"));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST, 0.0));
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_integer(const struct kefir_ast_type_traits *type_traits,
                                      struct kefir_irbuilder_block *builder, const struct kefir_ast_type *origin,
                                      const struct kefir_ast_type *target) {
    kefir_bool_t target_sign;
    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, target, &target_sign));

    REQUIRE_OK(obtain_real_part(type_traits, builder, &origin));
    if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32) {
        if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(target)) {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT_TO_BITINT_SIGNED,
                                                           target->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT_TO_BITINT_UNSIGNED,
                                                           target->bitprecise.width));
            }
        } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(target)) {
            switch (target->tag) {
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_FROM_FLOAT32, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_FROM_FLOAT32, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
            }
        } else if (target_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_INT, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_TO_UINT, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32) {
        if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(target)) {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DOUBLE_TO_BITINT_SIGNED,
                                                           target->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DOUBLE_TO_BITINT_UNSIGNED,
                                                           target->bitprecise.width));
            }
        } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(target)) {
            switch (target->tag) {
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_FROM_FLOAT64, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_FROM_FLOAT64, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
            }
        } else if (target_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_INT, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_TO_UINT, 0));
        }
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin) || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64) {
        if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(target)) {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED,
                                                           target->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED,
                                                           target->bitprecise.width));
            }
        } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(target)) {
            switch (target->tag) {
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_FROM_LONG_DOUBLE, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_FROM_LONG_DOUBLE, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
            }
        } else if (target_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_INT, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_UINT, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32) {
        if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(target)) {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_BITINT_SIGNED,
                                                           target->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_BITINT_UNSIGNED,
                                                           target->bitprecise.width));
            }
        } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(target)) {
            switch (target->tag) {
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_FROM_DECIMAL32, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_FROM_DECIMAL32, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
            }
        } else {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_INT, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL32_TO_UINT, 0));
            }
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64) {
        if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(target)) {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_BITINT_SIGNED,
                                                           target->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_BITINT_UNSIGNED,
                                                           target->bitprecise.width));
            }
        } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(target)) {
            switch (target->tag) {
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_FROM_DECIMAL64, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_FROM_DECIMAL64, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
            }
        } else {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_INT, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL64_TO_UINT, 0));
            }
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64) {
        if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(target)) {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_BITINT_SIGNED,
                                                           target->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_BITINT_UNSIGNED,
                                                           target->bitprecise.width));
            }
        } else if (KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(target)) {
            switch (target->tag) {
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_FROM_DECIMAL128, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_FROM_DECIMAL128, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
            }
        } else {
            if (target_sign) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_INT, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_DECIMAL128_TO_UINT, 0));
            }
        }
    }

    kefir_bool_t origin_type_signedness;
    kefir_ast_type_data_model_classification_t target_type_data_model, origin_type_data_model;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, target, &target_type_data_model));
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &origin_type_data_model));
    switch (target_type_data_model) {
        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            switch (origin_type_data_model) {
                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GET_SIGNED,
                                                                   origin->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GET_UNSIGNED,
                                                                   origin->bitprecise.width));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_TRUNCATE_INT64, 0));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            switch (origin_type_data_model) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_8BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GET_SIGNED,
                                                                   origin->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GET_UNSIGNED,
                                                                   origin->bitprecise.width));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_TRUNCATE_INT64, 0));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            switch (origin_type_data_model) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_8BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_16BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GET_SIGNED,
                                                                   origin->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GET_UNSIGNED,
                                                                   origin->bitprecise.width));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_TRUNCATE_INT64, 0));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            switch (origin_type_data_model) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_8BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_16BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_32BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffffffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GET_SIGNED,
                                                                   origin->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GET_UNSIGNED,
                                                                   origin->bitprecise.width));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT128:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_TRUNCATE_INT64, 0));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;
                
        case KEFIR_AST_TYPE_DATA_MODEL_INT128:
            switch (origin_type_data_model) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_8BITS, 0));
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGN_EXTEND_64BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_ZERO_EXTEND_64BITS, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_16BITS, 0));
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGN_EXTEND_64BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_ZERO_EXTEND_64BITS, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_32BITS, 0));
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGN_EXTEND_64BITS, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffffffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_ZERO_EXTEND_64BITS, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_SIGN_EXTEND_64BITS, 0));
                    } else {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_ZERO_EXTEND_64BITS, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_FROM_BITINT_SIGNED, origin->bitprecise.width));
                    } else {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_FROM_BITINT_UNSIGNED, origin->bitprecise.width));
                    }
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            switch (origin_type_data_model) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_8BITS, 0));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_FROM_SIGNED,
                                                                   target->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_FROM_UNSIGNED,
                                                                   target->bitprecise.width));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_16BITS, 0));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_FROM_SIGNED,
                                                                   target->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_FROM_UNSIGNED,
                                                                   target->bitprecise.width));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_32BITS, 0));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_FROM_SIGNED,
                                                                   target->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffffffULL));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_FROM_UNSIGNED,
                                                                   target->bitprecise.width));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_FROM_SIGNED,
                                                                   target->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_FROM_UNSIGNED,
                                                                   target->bitprecise.width));
                    }
                    break;
 
                case KEFIR_AST_TYPE_DATA_MODEL_INT128:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT128_SIGNED_TO_BITINT,
                                                                   target->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT128_UNSIGNED_TO_BITINT,
                                                                   target->bitprecise.width));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_type_signedness));
                    if (origin_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_BITINT_CAST_SIGNED,
                                                                   target->bitprecise.width, origin->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_BITINT_CAST_UNSIGNED,
                                                                   target->bitprecise.width, origin->bitprecise.width));
                    }
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected target integral type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_typeconv_to_bool(const struct kefir_ast_type_traits *type_traits,
                                                    struct kefir_irbuilder_block *builder,
                                                    const struct kefir_ast_type *origin) {
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type traits"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid origin AST type"));

    origin = kefir_ast_translator_normalize_type(origin);

    if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin) || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST, 0.0L));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_EQUALS, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, 0.0));
        REQUIRE_OK(
            KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE, KEFIR_IR_COMPARE_FLOAT64_EQUALS));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || origin->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, 0.0f, 0.0f));
        REQUIRE_OK(
            KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE, KEFIR_IR_COMPARE_FLOAT32_EQUALS));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_COMPLEX_FLOATING_POINT) {
        kefir_ast_type_data_model_classification_t classification;
        REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &classification));
        switch (classification) {
            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_TRUNCATE_1BIT, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_TRUNCATE_1BIT, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT, 0));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected complex floating-point real type");
        }
    } else if (origin->tag == KEFIR_AST_TYPE_IMAGINARY_FLOATING_POINT) {
        kefir_ast_type_data_model_classification_t classification;
        REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &classification));
        switch (classification) {
            case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, 0.0f, 0.0f));
                REQUIRE_OK(
                    KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE, KEFIR_IR_COMPARE_FLOAT32_EQUALS));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, 0.0));
                REQUIRE_OK(
                    KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE, KEFIR_IR_COMPARE_FLOAT64_EQUALS));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST, 0.0));
                REQUIRE_OK(
                    KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_EQUALS, 0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected imaginary floating-point real type");
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32) {
        REQUIRE_OK(kefir_dfp_require_supported(NULL));
        kefir_dfp_decimal32_t dec32 = kefir_dfp_decimal32_from_int64(0);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_DECIMAL32_CONST, dec32.uvalue, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL32_EQUAL, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_BOOL_NOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64) {
        REQUIRE_OK(kefir_dfp_require_supported(NULL));
        kefir_dfp_decimal64_t dec64 = kefir_dfp_decimal64_from_int64(0);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL64_CONST, dec64.uvalue));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL64_EQUAL, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_BOOL_NOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || origin->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64) {
        REQUIRE_OK(kefir_dfp_require_supported(NULL));
        kefir_dfp_decimal128_t dec128 = kefir_dfp_decimal128_from_int64(0);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_DECIMAL128_CONST, dec128.uvalue[0],
                                                     dec128.uvalue[1]));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_DECIMAL128_EQUAL, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_BOOL_NOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, 0));
    } else {
        kefir_ast_type_data_model_classification_t origin_type_data_model;
        REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, origin, &origin_type_data_model));
        switch (origin_type_data_model) {
            case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_TO_BOOL, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_TO_BOOL, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_TO_BOOL, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_TO_BOOL, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT128:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT128_TO_BOOL, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                REQUIRE_OK(
                    KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_TO_BOOL, origin->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to cast non-scalar type to bool");
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_typeconv(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            struct kefir_irbuilder_block *builder,
                                            const struct kefir_ast_type_traits *type_traits,
                                            const struct kefir_ast_type *origin,
                                            const struct kefir_ast_type *destination) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid origin AST type traits"));
    REQUIRE(origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid origin AST type"));
    REQUIRE(destination != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST type"));

    const struct kefir_ast_type *normalized_origin = kefir_ast_translator_normalize_type(origin);
    const struct kefir_ast_type *normalized_destination = kefir_ast_translator_normalize_type(destination);

    REQUIRE(KEFIR_AST_TYPE_IS_SCALAR_TYPE(normalized_origin) || normalized_destination->tag == KEFIR_AST_TYPE_VOID,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected origin AST type to be scalar"));
    REQUIRE(KEFIR_AST_TYPE_IS_SCALAR_TYPE(normalized_destination) || normalized_destination->tag == KEFIR_AST_TYPE_VOID,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected destination AST type to be scalar or void"));

    if (normalized_origin->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER &&
        normalized_destination->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, 0));
        return KEFIR_OK;
    }

    REQUIRE(!KEFIR_AST_TYPE_SAME(normalized_origin, normalized_destination), KEFIR_OK);

    switch (normalized_destination->tag) {
        case KEFIR_AST_TYPE_VOID:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_POINTER:
            if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(normalized_origin)) {
                REQUIRE_OK(cast_to_integer(type_traits, builder, normalized_origin, type_traits->uintptr_type));
            } else if (normalized_origin->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, 0));
            } else {
                REQUIRE(normalized_origin->tag == KEFIR_AST_TYPE_SCALAR_POINTER,
                        KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected origin type to be integral or pointer"));
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_NULL_POINTER:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
            REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(type_traits, builder, normalized_origin));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
        case KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32:
            REQUIRE_OK(cast_to_float32(mem, module, builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
        case KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64:
        case KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32:
            REQUIRE_OK(cast_to_float64(mem, module, builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
        case KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80:
        case KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64:
            REQUIRE_OK(cast_to_long_double(mem, module, builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOATING_POINT: {
            kefir_ast_type_data_model_classification_t classification;
            REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, normalized_destination, &classification));
            switch (classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
                    REQUIRE_OK(cast_to_complex_float(mem, module, builder, type_traits, normalized_origin));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
                    REQUIRE_OK(cast_to_complex_double(mem, module, builder, type_traits, normalized_origin));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
                    REQUIRE_OK(cast_to_complex_long_double(mem, module, builder, type_traits, normalized_origin));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected complex floating-point real type");
            }
        } break;

        case KEFIR_AST_TYPE_IMAGINARY_FLOATING_POINT: {
            kefir_ast_type_data_model_classification_t classification;
            REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, normalized_destination, &classification));
            switch (classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
                    REQUIRE_OK(cast_to_imaginary_float(mem, module, builder, type_traits, normalized_origin));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
                    REQUIRE_OK(cast_to_imaginary_double(mem, module, builder, type_traits, normalized_origin));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
                    REQUIRE_OK(cast_to_imaginary_long_double(mem, module, builder, type_traits, normalized_origin));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected complex floating-point real type");
            }
        } break;

        case KEFIR_AST_TYPE_SCALAR_DECIMAL32:
            REQUIRE_OK(cast_to_decimal32(mem, module, builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_SCALAR_DECIMAL64:
            REQUIRE_OK(cast_to_decimal64(mem, module, builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_SCALAR_DECIMAL128:
        case KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64:
            REQUIRE_OK(cast_to_decimal128(mem, module, builder, type_traits, normalized_origin));
            break;

        default:
            REQUIRE_OK(cast_to_integer(type_traits, builder, normalized_origin, normalized_destination));
            break;
    }
    return KEFIR_OK;
}
