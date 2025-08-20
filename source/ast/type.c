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

#include <string.h>
#include "kefir/ast/type.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#define __KEFIR_IMPL_TYPECLASS_HEADER__
#include "kefir/runtime/common/typeclass_kefir_impl.h"
#undef __KEFIR_IMPL_TYPECLASS_HEADER__

const struct kefir_ast_bitfield_properties KEFIR_AST_BITFIELD_PROPERTIES_NONE = {.bitfield = false};

static kefir_result_t default_integral_type_rank(const struct kefir_ast_type_traits *type_traits,
                                                 const struct kefir_ast_type *type, kefir_size_t *rank_ptr) {
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type traits"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid integral AST type"));
    REQUIRE(KEFIR_AST_TYPE_IS_NONENUM_INTEGRAL_TYPE(type),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid integral AST type"));
    REQUIRE(rank_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST type rank"));

    ASSIGN_DECL_CAST(const struct kefir_data_model_descriptor *, data_model, type_traits->payload);
    switch (type->tag) {
#define MAKE_RANK(_base, _width, _bit_precise) \
    ((((kefir_uint64_t) (_base)) << 32) | ((kefir_uint32_t) ((_width) << 1)) | ((_bit_precise) ? 0 : 1))
        case KEFIR_AST_TYPE_SCALAR_BOOL:
            *rank_ptr = MAKE_RANK(0, data_model->scalar_width.bool_bits, false);
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            *rank_ptr = MAKE_RANK(1, data_model->scalar_width.char_bits, false);
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            *rank_ptr = MAKE_RANK(2, data_model->scalar_width.short_bits, false);
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            *rank_ptr = MAKE_RANK(3, data_model->scalar_width.int_bits, false);
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            *rank_ptr = MAKE_RANK(4, data_model->scalar_width.long_bits, false);
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            *rank_ptr = MAKE_RANK(5, data_model->scalar_width.long_long_bits, false);
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE:
            if (type->bitprecise.width <= data_model->scalar_width.char_bits) {
                *rank_ptr = MAKE_RANK(1, type->bitprecise.width, true);
            } else if (type->bitprecise.width <= data_model->scalar_width.short_bits) {
                *rank_ptr = MAKE_RANK(2, type->bitprecise.width, true);
            } else if (type->bitprecise.width <= data_model->scalar_width.int_bits) {
                *rank_ptr = MAKE_RANK(3, type->bitprecise.width, true);
            } else if (type->bitprecise.width <= data_model->scalar_width.long_bits) {
                *rank_ptr = MAKE_RANK(4, type->bitprecise.width, true);
            } else if (type->bitprecise.width <= data_model->scalar_width.long_long_bits) {
                *rank_ptr = MAKE_RANK(5, type->bitprecise.width, true);
            } else {
                *rank_ptr = MAKE_RANK(6, type->bitprecise.width, true);
            }
            break;
#undef MAKE_RANK

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected integral AST type");
    }
    return KEFIR_OK;
}

static kefir_result_t default_integral_type_width(const struct kefir_ast_type_traits *type_traits,
                                                  const struct kefir_ast_type *type, kefir_size_t *width_ptr) {
    ASSIGN_DECL_CAST(const struct kefir_data_model_descriptor *, data_model, type_traits->payload);
    switch (type->tag) {
        case KEFIR_AST_TYPE_SCALAR_BOOL:
            *width_ptr = data_model->scalar_width.bool_bits;
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            *width_ptr = data_model->scalar_width.char_bits;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            *width_ptr = data_model->scalar_width.short_bits;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            *width_ptr = data_model->scalar_width.int_bits;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            *width_ptr = data_model->scalar_width.long_bits;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            *width_ptr = data_model->scalar_width.long_long_bits;
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE:
            *width_ptr = type->bitprecise.width;
            break;
#undef MAKE_RANK

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected integral AST type");
    }
    return KEFIR_OK;
}

static kefir_result_t default_integral_type_fits(const struct kefir_ast_type_traits *type_traits,
                                                 const struct kefir_ast_type *source, const struct kefir_ast_type *dest,
                                                 kefir_bool_t *result) {
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type traits"));
    REQUIRE(source != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST type"));
    REQUIRE(dest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST type"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid result pointer"));
    REQUIRE((KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(source) || source->tag == KEFIR_AST_TYPE_SCALAR_BOOL) &&
                (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(dest) || dest->tag == KEFIR_AST_TYPE_SCALAR_BOOL),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected both source and destination to be basic types"));

    kefir_size_t source_width = 0, destination_width = 0;
    REQUIRE_OK(default_integral_type_width(type_traits, source, &source_width));
    REQUIRE_OK(default_integral_type_width(type_traits, dest, &destination_width));

    kefir_bool_t src_sign, dst_sign;
    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, source, &src_sign));
    REQUIRE_OK(kefir_ast_type_is_signed(type_traits, dest, &dst_sign));

    if (src_sign == dst_sign || (src_sign && !dst_sign)) {
        *result = source_width <= destination_width;
    } else if (!src_sign && dst_sign) {
        *result = source_width < destination_width;
    } else {
        *result = false;
    }
    return KEFIR_OK;
}

static kefir_result_t default_pointer_type_fits(const struct kefir_ast_type_traits *type_traits,
                                                const struct kefir_ast_type *type, kefir_bool_t *result) {
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type traits"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid result pointer"));
    REQUIRE((KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(type) || type->tag == KEFIR_AST_TYPE_SCALAR_BOOL ||
             type->tag == KEFIR_AST_TYPE_SCALAR_POINTER || type->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected type to be basic or pointer type"));

    ASSIGN_DECL_CAST(const struct kefir_data_model_descriptor *, data_model, type_traits->payload);
    switch (type->tag) {
        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            *result = false;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            *result = data_model->model == KEFIR_DATA_MODEL_SILP64 || data_model->model == KEFIR_DATA_MODEL_ILP64 ||
                      data_model->model == KEFIR_DATA_MODEL_ILP32;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            *result = data_model->model != KEFIR_DATA_MODEL_LLP64;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            *result = true;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE:
            *result = type->bitprecise.width >= data_model->scalar_width.long_long_bits;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected AST type");
    }
    return KEFIR_OK;
}

static const struct kefir_ast_type *default_bitfield_promotion(const struct kefir_ast_type_traits *type_traits,
                                                               const struct kefir_ast_type *type, kefir_size_t width) {
    REQUIRE(type_traits != NULL, NULL);
    REQUIRE(type != NULL, NULL);

    ASSIGN_DECL_CAST(const struct kefir_data_model_descriptor *, data_model, type_traits->payload);
    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(type)) {
        return type;
    } else if (width == data_model->scalar_width.int_bits && KEFIR_INTERNAL_AST_TYPE_IS_SIGNED_INTEGER(type)) {
        return kefir_ast_type_signed_int();
    } else if (width >= data_model->scalar_width.int_bits) {
        return type;
    } else {
        return kefir_ast_type_signed_int();
    }
}

kefir_result_t kefir_ast_type_traits_init(const struct kefir_data_model_descriptor *data_model,
                                          struct kefir_ast_type_traits *type_traits) {
    REQUIRE(data_model != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid data model"));
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST type traits"));

    *type_traits = (struct kefir_ast_type_traits) {.integral_type_rank = default_integral_type_rank,
                                                   .integral_type_fits = default_integral_type_fits,
                                                   .pointer_type_fits = default_pointer_type_fits,
                                                   .bitfield_promotion = default_bitfield_promotion,
                                                   .underlying_enumeration_type = kefir_ast_type_signed_int(),
                                                   .incomplete_type_substitute = kefir_ast_type_char(),
                                                   .character_type_signedness = true,
                                                   .data_model = data_model,
                                                   .payload = (kefir_uptr_t) data_model};

    switch (data_model->model) {
        case KEFIR_DATA_MODEL_LP64:
            type_traits->size_type = kefir_ast_type_unsigned_long();
            type_traits->uintptr_type = kefir_ast_type_unsigned_long();
            type_traits->ptrdiff_type = kefir_ast_type_signed_long();
            type_traits->unicode8_char_type = kefir_ast_type_char();
            type_traits->unicode16_char_type = kefir_ast_type_unsigned_short();
            type_traits->unicode32_char_type = kefir_ast_type_unsigned_int();
            type_traits->wide_char_type = kefir_ast_type_signed_int();
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Data models other than LP64 are not supported at the moment");
    }
    return KEFIR_OK;
}

kefir_bool_t kefir_ast_type_is_complete(const struct kefir_ast_type *type) {
    switch (type->tag) {
        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION:
            return type->structure_type.complete;

        case KEFIR_AST_TYPE_ENUMERATION:
            return type->enumeration_type.complete;

        default:
            return true;
    }
}

kefir_result_t kefir_ast_type_list_variable_modificators(const struct kefir_ast_type *type,
                                                         kefir_result_t (*callback)(const struct kefir_ast_node_base *,
                                                                                    void *),
                                                         void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(callback != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node callback"));

    switch (type->tag) {
        case KEFIR_AST_TYPE_ARRAY:
            if (KEFIR_AST_TYPE_IS_VL_ARRAY(type)) {
                kefir_result_t res = callback(type->array_type.vla_length, payload);
                if (res != KEFIR_YIELD) {
                    REQUIRE_OK(res);
                    REQUIRE_OK(
                        kefir_ast_type_list_variable_modificators(type->array_type.element_type, callback, payload));
                }
            } else {
                REQUIRE_OK(kefir_ast_type_list_variable_modificators(type->array_type.element_type, callback, payload));
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE_OK(kefir_ast_type_list_variable_modificators(type->referenced_type, callback, payload));
            break;

        case KEFIR_AST_TYPE_QUALIFIED:
            REQUIRE_OK(kefir_ast_type_list_variable_modificators(type->qualified_type.type, callback, payload));
            break;

        default:
            // All other types cannot be variably-modified
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t list_vl_modifiers(const struct kefir_ast_node_base *node, void *payload) {
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(const struct kefir_ast_node_base **, found, payload);
    if (node != NULL) {
        *found = node;
        return KEFIR_YIELD;
    } else {
        return KEFIR_OK;
    }
}

const struct kefir_ast_node_base *kefir_ast_type_get_top_variable_modificator(const struct kefir_ast_type *type) {
    REQUIRE(type != NULL, NULL);
    const struct kefir_ast_node_base *node = NULL;
    kefir_ast_type_list_variable_modificators(type, list_vl_modifiers, &node);
    kefir_clear_error();
    return node;
}

kefir_bool_t kefir_ast_type_is_variably_modified(const struct kefir_ast_type *type) {
    return kefir_ast_type_get_top_variable_modificator(type) != NULL;
}

static kefir_result_t free_type_bundle(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                       void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid list entry"));
    REQUIRE_OK(KEFIR_AST_TYPE_FREE(mem, (struct kefir_ast_type *) entry->value));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_bundle_init(struct kefir_ast_type_bundle *type_bundle,
                                          struct kefir_string_pool *symbols) {
    REQUIRE(type_bundle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type type_bundlesitory"));
    type_bundle->symbols = symbols;
    REQUIRE_OK(kefir_list_init(&type_bundle->types));
    REQUIRE_OK(kefir_list_on_remove(&type_bundle->types, free_type_bundle, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_bundle_free(struct kefir_mem *mem, struct kefir_ast_type_bundle *type_bundle) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type_bundle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type type_bundlesitory"));
    REQUIRE_OK(kefir_list_free(mem, &type_bundle->types));
    return KEFIR_OK;
}

kefir_ast_function_specifier_t kefir_ast_context_merge_function_specifiers(kefir_ast_function_specifier_t s1,
                                                                           kefir_ast_function_specifier_t s2) {
    _Static_assert(KEFIR_AST_FUNCTION_SPECIFIER_NONE < 4,
                   "AST function specifier is expected to fit into conversion matrix");
    _Static_assert(KEFIR_AST_FUNCTION_SPECIFIER_INLINE < 4,
                   "AST function specifier is expected to fit into conversion matrix");
    _Static_assert(KEFIR_AST_FUNCTION_SPECIFIER_NORETURN < 4,
                   "AST function specifier is expected to fit into conversion matrix");
    _Static_assert(KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN < 4,
                   "AST function specifier is expected to fit into conversion matrix");
    kefir_ast_function_specifier_t MATRIX[4][4] = {
        [KEFIR_AST_FUNCTION_SPECIFIER_NONE] =
            {[KEFIR_AST_FUNCTION_SPECIFIER_NONE] = KEFIR_AST_FUNCTION_SPECIFIER_NONE,
             [KEFIR_AST_FUNCTION_SPECIFIER_INLINE] = KEFIR_AST_FUNCTION_SPECIFIER_INLINE,
             [KEFIR_AST_FUNCTION_SPECIFIER_NORETURN] = KEFIR_AST_FUNCTION_SPECIFIER_NORETURN,
             [KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN] = KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN},
        [KEFIR_AST_FUNCTION_SPECIFIER_INLINE] =
            {[KEFIR_AST_FUNCTION_SPECIFIER_NONE] = KEFIR_AST_FUNCTION_SPECIFIER_NONE,
             [KEFIR_AST_FUNCTION_SPECIFIER_INLINE] = KEFIR_AST_FUNCTION_SPECIFIER_INLINE,
             [KEFIR_AST_FUNCTION_SPECIFIER_NORETURN] = KEFIR_AST_FUNCTION_SPECIFIER_NORETURN,
             [KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN] = KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN},
        [KEFIR_AST_FUNCTION_SPECIFIER_NORETURN] =
            {[KEFIR_AST_FUNCTION_SPECIFIER_NONE] = KEFIR_AST_FUNCTION_SPECIFIER_NORETURN,
             [KEFIR_AST_FUNCTION_SPECIFIER_INLINE] = KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN,
             [KEFIR_AST_FUNCTION_SPECIFIER_NORETURN] = KEFIR_AST_FUNCTION_SPECIFIER_NORETURN,
             [KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN] = KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN},
        [KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN] = {
            [KEFIR_AST_FUNCTION_SPECIFIER_NONE] = KEFIR_AST_FUNCTION_SPECIFIER_NORETURN,
            [KEFIR_AST_FUNCTION_SPECIFIER_INLINE] = KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN,
            [KEFIR_AST_FUNCTION_SPECIFIER_NORETURN] = KEFIR_AST_FUNCTION_SPECIFIER_NORETURN,
            [KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN] = KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN}};
    return MATRIX[s1][s2];
}

kefir_bool_t kefir_ast_function_specifier_is_inline(kefir_ast_function_specifier_t specifier) {
    return specifier == KEFIR_AST_FUNCTION_SPECIFIER_INLINE ||
           specifier == KEFIR_AST_FUNCTION_SPECIFIER_INLINE_NORETURN;
}

kefir_result_t kefir_ast_type_classify(const struct kefir_ast_type *type, kefir_int_t *klass_ptr) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(klass_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to type"));

    type = kefir_ast_unqualified_type(type);
    if (type->tag == KEFIR_AST_TYPE_VOID) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_VOID_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_SCALAR_CHAR || type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR ||
               type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_CHAR_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT || type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT ||
               type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_INT || type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT ||
               type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG || type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG ||
               type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG ||
               type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG ||
               type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE ||
               type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_INTEGER_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_ENUMERATION) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_ENUMERAL_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_SCALAR_BOOL) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_BOOLEAN_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_SCALAR_POINTER || type->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_POINTER_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || type->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE ||
               type->tag == KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_REAL_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_FUNCTION) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_FUNCTION_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_STRUCTURE) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_RECORD_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_UNION) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_UNION_TYPE_CLASS;
    } else if (type->tag == KEFIR_AST_TYPE_ARRAY) {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_ARRAY_TYPE_CLASS;
    } else {
        *klass_ptr = __KEFIR_IMPL_TYPECLASS_NO_TYPE_CLASS;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_data_model_classify(const struct kefir_ast_type_traits *type_traits,
                                                  const struct kefir_ast_type *type,
                                                  kefir_ast_type_data_model_classification_t *classification_ptr) {
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type traits"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(classification_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST type data model classification"));

    ASSIGN_DECL_CAST(const struct kefir_data_model_descriptor *, data_model, type_traits->payload);
#define MATCH_BITS(_bits, _classification)                                                        \
    do {                                                                                          \
        switch ((_bits)) {                                                                        \
            case 8:                                                                               \
                *(_classification) = KEFIR_AST_TYPE_DATA_MODEL_INT8;                              \
                break;                                                                            \
            case 16:                                                                              \
                *(_classification) = KEFIR_AST_TYPE_DATA_MODEL_INT16;                             \
                break;                                                                            \
            case 32:                                                                              \
                *(_classification) = KEFIR_AST_TYPE_DATA_MODEL_INT32;                             \
                break;                                                                            \
            case 64:                                                                              \
                *(_classification) = KEFIR_AST_TYPE_DATA_MODEL_INT64;                             \
                break;                                                                            \
            default:                                                                              \
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected integer type bit width"); \
        }                                                                                         \
    } while (0)

    switch (type->tag) {
        case KEFIR_AST_TYPE_VOID:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_VOID;
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
            MATCH_BITS(data_model->scalar_width.bool_bits, classification_ptr);
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            MATCH_BITS(data_model->scalar_width.char_bits, classification_ptr);
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            MATCH_BITS(data_model->scalar_width.short_bits, classification_ptr);
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            MATCH_BITS(data_model->scalar_width.int_bits, classification_ptr);
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            MATCH_BITS(data_model->scalar_width.long_bits, classification_ptr);
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            MATCH_BITS(data_model->scalar_width.long_long_bits, classification_ptr);
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_FLOAT;
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_DOUBLE;
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE;
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT;
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE;
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE;
            break;

        case KEFIR_AST_TYPE_SCALAR_POINTER:
        case KEFIR_AST_TYPE_SCALAR_NULL_POINTER:
            REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, type_traits->uintptr_type, classification_ptr));
            break;

        case KEFIR_AST_TYPE_ENUMERATION:
            REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, type->enumeration_type.underlying_type,
                                                          classification_ptr));
            break;

        case KEFIR_AST_TYPE_QUALIFIED:
            REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, type->qualified_type.type, classification_ptr));
            break;

        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION:
        case KEFIR_AST_TYPE_ARRAY:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_AGGREGATE;
            break;

        case KEFIR_AST_TYPE_FUNCTION:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_FUNCTION;
            break;

        case KEFIR_AST_TYPE_AUTO:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_AUTO;
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE:
            *classification_ptr = KEFIR_AST_TYPE_DATA_MODEL_BITINT;
            break;
    }

#undef MATCH_BITS

    return KEFIR_OK;
}

kefir_result_t kefir_ast_type_apply_qualification(struct kefir_mem *mem, struct kefir_ast_type_bundle *type_bundle,
                                                  kefir_c_language_standard_version_t standard_version,
                                                  const struct kefir_ast_type *original_type,
                                                  const struct kefir_ast_type_qualification *qualifications,
                                                  const struct kefir_ast_type **type_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type_bundle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type bundle"));
    REQUIRE(original_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(type_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST type"));

    if (qualifications == NULL) {
        *type_ptr = original_type;
        return KEFIR_OK;
    }

    const struct kefir_ast_type *unqualified_original_type = kefir_ast_unqualified_type(original_type);
    struct kefir_ast_type_qualification original_type_qualifications = {0}, merged_type_qualifications = {0};
    REQUIRE_OK(kefir_ast_type_retrieve_qualifications(&original_type_qualifications, original_type));
    REQUIRE_OK(kefir_ast_type_merge_qualifications(&merged_type_qualifications, &original_type_qualifications,
                                                   qualifications));
    if (unqualified_original_type->tag == KEFIR_AST_TYPE_ARRAY) {
        original_type = kefir_ast_type_array_with_element_type(
            mem, type_bundle, &unqualified_original_type->array_type,
            kefir_ast_type_qualified(mem, type_bundle, original_type->array_type.element_type,
                                     merged_type_qualifications));
        REQUIRE(original_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate array type"));
        if (KEFIR_STANDARD_VERSION_AT_LEAST_C23(standard_version)) {
            original_type = kefir_ast_type_qualified(mem, type_bundle, original_type, merged_type_qualifications);
        }
    } else {
        original_type = kefir_ast_type_qualified(mem, type_bundle, original_type, merged_type_qualifications);
    }

    *type_ptr = original_type;
    return KEFIR_OK;
}

const struct kefir_ast_type *kefir_ast_type_traits_quarter_integer_type(const struct kefir_ast_type_traits *type_traits,
                                                                        kefir_bool_t signed_type) {
    REQUIRE(type_traits != NULL, NULL);

#define MATCH_WIDTH(_width)                                                            \
    do {                                                                               \
        if (type_traits->data_model->scalar_width.char_bits == (_width)) {             \
            if (signed_type) {                                                         \
                return kefir_ast_type_signed_char();                                   \
            } else {                                                                   \
                return kefir_ast_type_unsigned_char();                                 \
            }                                                                          \
        } else if (type_traits->data_model->scalar_width.short_bits == (_width)) {     \
            if (signed_type) {                                                         \
                return kefir_ast_type_signed_short();                                  \
            } else {                                                                   \
                return kefir_ast_type_unsigned_short();                                \
            }                                                                          \
        } else if (type_traits->data_model->scalar_width.int_bits == (_width)) {       \
            if (signed_type) {                                                         \
                return kefir_ast_type_signed_int();                                    \
            } else {                                                                   \
                return kefir_ast_type_unsigned_int();                                  \
            }                                                                          \
        } else if (type_traits->data_model->scalar_width.long_bits == (_width)) {      \
            if (signed_type) {                                                         \
                return kefir_ast_type_signed_long();                                   \
            } else {                                                                   \
                return kefir_ast_type_unsigned_long();                                 \
            }                                                                          \
        } else if (type_traits->data_model->scalar_width.long_long_bits == (_width)) { \
            if (signed_type) {                                                         \
                return kefir_ast_type_signed_long_long();                              \
            } else {                                                                   \
                return kefir_ast_type_unsigned_long_long();                            \
            }                                                                          \
        }                                                                              \
    } while (false)

    MATCH_WIDTH(8);
    return NULL;
}

const struct kefir_ast_type *kefir_ast_type_traits_half_integer_type(const struct kefir_ast_type_traits *type_traits,
                                                                     kefir_bool_t signed_type) {
    REQUIRE(type_traits != NULL, NULL);

    MATCH_WIDTH(16);
    return NULL;
}

const struct kefir_ast_type *kefir_ast_type_traits_single_integer_type(const struct kefir_ast_type_traits *type_traits,
                                                                       kefir_bool_t signed_type) {
    REQUIRE(type_traits != NULL, NULL);

    MATCH_WIDTH(32);
    return NULL;
}

const struct kefir_ast_type *kefir_ast_type_traits_double_integer_type(const struct kefir_ast_type_traits *type_traits,
                                                                       kefir_bool_t signed_type) {
    REQUIRE(type_traits != NULL, NULL);

    MATCH_WIDTH(64);
#undef MATCH_WIDTH
    return NULL;
}
