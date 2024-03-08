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

#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ir/module.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t obtain_real_part(struct kefir_irbuilder_block *builder, const struct kefir_ast_type **origin) {
    if ((*origin)->tag == KEFIR_AST_TYPE_COMPLEX_FLOAT) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF32R, 0));
        *origin = kefir_ast_type_corresponding_real_type(*origin);
        REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
    } else if ((*origin)->tag == KEFIR_AST_TYPE_COMPLEX_DOUBLE) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LOAD64, 0));
        *origin = kefir_ast_type_corresponding_real_type(*origin);
        REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
    } else if ((*origin)->tag == KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LOADLD, 0));
        *origin = kefir_ast_type_corresponding_real_type(*origin);
        REQUIRE(*origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain corresponding real type"));
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_float32(struct kefir_irbuilder_block *builder,
                                      const struct kefir_ast_type_traits *type_traits,
                                      const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(builder, &origin));

    if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, kefir_ast_translator_normalize_type(origin), &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_INTCF32, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UINTCF32, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT) {
        // Intentionally left blank
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64CF32, 0));
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin)) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDCF32, 0));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to floating-point values");
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_float64(struct kefir_irbuilder_block *builder,
                                      const struct kefir_ast_type_traits *type_traits,
                                      const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(builder, &origin));

    if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_INTCF64, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UINTCF64, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32CF64, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE) {
        // Intentionally left blank
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin)) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDCF64, 0));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to floating-point values");
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_long_double(struct kefir_irbuilder_block *builder,
                                          const struct kefir_ast_type_traits *type_traits,
                                          const struct kefir_ast_type *origin) {
    REQUIRE_OK(obtain_real_part(builder, &origin));
    if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin)) {
        kefir_bool_t origin_sign;
        REQUIRE_OK(kefir_ast_type_is_signed(type_traits, origin, &origin_sign));

        if (origin_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_INTCLD, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UINTCLD, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32CLD, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64CLD, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE) {
        // Intentionally left blank
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast pointers to floating-point values");
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

    switch (origin->tag) {
        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            // Intentionally left blank
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF64R, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F64CF32, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF64I, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F64CF32, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF32, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPLDR, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LDCF32, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPLDI, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LDCF32, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF32, 0));
            break;

        default:
            REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                    KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to complex float"));
            REQUIRE_OK(cast_to_float32(builder, type_traits, origin));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IROPCODE_PUSHF32, 0.0f, 0.0f));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF32, 0));
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

    switch (origin->tag) {
        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF32R, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F32CF64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF32I, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F32CF64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF64, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            // Intentionally left blank
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPLDR, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LDCF64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPLDI, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LDCF64, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF64, 0));
            break;

        default:
            REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                    KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to complex float"));
            REQUIRE_OK(cast_to_float64(builder, type_traits, origin));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IROPCODE_PUSHF64, 0.0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF64, 0));
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

    switch (origin->tag) {
        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF32R, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F32CLD, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF32I, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F32CLD, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPLD, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF64R, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F64CLD, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPF64I, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F64CLD, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPLD, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            // Intentionally left blank
            break;

        default:
            REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(origin),
                    KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot cast non-arithmetic types to complex float"));
            REQUIRE_OK(cast_to_long_double(builder, type_traits, origin));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IROPCODE_PUSHLD, 0.0L));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_CMPLD, 0));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t cast_to_integer(const struct kefir_ast_type_traits *type_traits,
                                      struct kefir_irbuilder_block *builder, const struct kefir_ast_type *origin,
                                      const struct kefir_ast_type *target) {
    kefir_bool_t target_sign;
    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, target, &target_sign));

    REQUIRE_OK(obtain_real_part(builder, &origin));
    if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(origin) || origin->tag == KEFIR_AST_TYPE_SCALAR_POINTER ||
        target->tag == KEFIR_AST_TYPE_SCALAR_BOOL) {
        // Do nothing
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT) {
        if (target_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32CINT, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32CUINT, 0));
        }
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE) {
        if (target_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64CINT, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64CUINT, 0));
        }
    } else if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin)) {
        if (target_sign) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDCINT, 0));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDCUINT, 0));
        }
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected type in integral conversion");
    }

    switch (target->tag) {
        case KEFIR_AST_TYPE_SCALAR_CHAR:
            if (type_traits->character_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_EXTEND8, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, 0xff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IAND, 0));
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, 0xffULL));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IAND, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_EXTEND8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, 0xffffULL));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IAND, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_EXTEND16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, 0xffffffffULL));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IAND, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_EXTEND32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            // Intentionally left blank
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected target integral type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_typeconv_to_bool(struct kefir_irbuilder_block *builder,
                                                    const struct kefir_ast_type *origin) {
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid origin AST type"));

    origin = kefir_ast_translator_normalize_type(origin);

    if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(origin)) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IROPCODE_PUSHLD, 0.0L));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LDEQUALS, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_BNOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IROPCODE_PUSHF64, 0.0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F64EQUALS, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_BNOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_SCALAR_FLOAT) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IROPCODE_PUSHF32, 0.0f, 0.0f));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_F32EQUALS, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_BNOT, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_COMPLEX_FLOAT) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF32TRUNC1, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_COMPLEX_DOUBLE) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF64TRUNC1, 0));
    } else if (origin->tag == KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPLDTRUNC1, 0));
    } else {
        REQUIRE(KEFIR_AST_TYPE_IS_SCALAR_TYPE(origin),
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to cast non-scalar type to bool"));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_TRUNCATE1, 0));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_typeconv_normalize(struct kefir_irbuilder_block *builder,
                                                      const struct kefir_ast_type_traits *type_traits,
                                                      const struct kefir_ast_type *origin) {
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid origin AST type traits"));
    REQUIRE(origin != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid origin AST type"));

    const struct kefir_ast_type *normalized_origin = kefir_ast_translator_normalize_type(origin);
    REQUIRE(KEFIR_AST_TYPE_IS_SCALAR_TYPE(normalized_origin),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected origin AST type to be scalar"));

    switch (normalized_origin->tag) {
        case KEFIR_AST_TYPE_SCALAR_POINTER:
        case KEFIR_AST_TYPE_SCALAR_FLOAT:
        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            // Intentionally left blank
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IAND, 0));
            break;

        default:
            REQUIRE_OK(cast_to_integer(type_traits, builder, normalized_origin, normalized_origin));
            break;
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
    REQUIRE(!KEFIR_AST_TYPE_SAME(normalized_origin, normalized_destination), KEFIR_OK);

    switch (normalized_destination->tag) {
        case KEFIR_AST_TYPE_VOID:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_POP, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(normalized_origin) ||
                        normalized_origin->tag == KEFIR_AST_TYPE_SCALAR_POINTER,
                    KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected origin type to be integral or pointer"));
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
            REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(builder, normalized_origin));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(cast_to_float32(builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE_OK(cast_to_float64(builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(cast_to_long_double(builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(cast_to_complex_float(mem, module, builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(cast_to_complex_double(mem, module, builder, type_traits, normalized_origin));
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(cast_to_complex_long_double(mem, module, builder, type_traits, normalized_origin));
            break;

        default:
            REQUIRE_OK(cast_to_integer(type_traits, builder, normalized_origin, normalized_destination));
            break;
    }
    return KEFIR_OK;
}
