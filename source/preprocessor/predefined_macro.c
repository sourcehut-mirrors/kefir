/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "kefir/preprocessor/predefined_macro.h"
#include "kefir/preprocessor/preprocessor.h"
#include "kefir/preprocessor/util.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <float.h>

static kefir_result_t make_pp_number(struct kefir_mem *mem, struct kefir_token_buffer *buffer, const char *buf,
                                     const struct kefir_source_location *source_location) {
    struct kefir_token token;
    if (buf[0] == '-') {
        buf++;
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_MINUS, &token));
        token.source_location = *source_location;
        kefir_result_t res = kefir_token_buffer_emplace(mem, buffer, &token);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_token_free(mem, &token);
            return res;
        });
    }

    REQUIRE_OK(kefir_token_new_pp_number(mem, buf, strlen(buf), &token));
    token.source_location = *source_location;
    kefir_result_t res = kefir_token_buffer_emplace(mem, buffer, &token);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, &token);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t predefined_macro_argc(const struct kefir_preprocessor_macro *macro, kefir_size_t *argc_ptr,
                                            kefir_bool_t *vararg_ptr) {
    UNUSED(macro);
    UNUSED(argc_ptr);
    UNUSED(vararg_ptr);
    return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to retrieve argument count of predefined object macro");
}

#define MACRO(_name)                                                                                                  \
    static kefir_result_t macro_##_name##_apply(                                                                      \
        struct kefir_mem *mem, struct kefir_preprocessor *preprocessor, const struct kefir_preprocessor_macro *macro, \
        struct kefir_symbol_table *symbols, const struct kefir_list *args, struct kefir_token_buffer *buffer,         \
        const struct kefir_source_location *source_location) {                                                        \
        UNUSED(symbols);                                                                                              \
        UNUSED(preprocessor);                                                                                         \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));            \
        REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor macro"));        \
        REQUIRE(args == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected empty macro argument list"));        \
        REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));             \
        REQUIRE(source_location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location")); \
        ASSIGN_DECL_CAST(struct kefir_preprocessor_predefined_macro_scope *, predefined_scope, macro->payload);       \
        UNUSED(predefined_scope);                                                                                     \
                                                                                                                      \
        do

#define MACRO_END    \
    while (0)        \
        ;            \
    return KEFIR_OK; \
    }

MACRO(file) {
    const char *file = source_location->source;
    struct kefir_token token;
    REQUIRE_OK(kefir_token_new_string_literal_raw_from_escaped_multibyte(mem, KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
                                                                         file, strlen(file), &token));
    token.source_location = *source_location;
    kefir_result_t res = kefir_token_buffer_emplace(mem, buffer, &token);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, &token);
        return res;
    });
}
MACRO_END

MACRO(line) {
    kefir_source_location_line_t line = source_location->line;
    char strbuf[64] = {0};
    snprintf(strbuf, sizeof(strbuf), KEFIR_UINT_FMT, line);
    REQUIRE_OK(make_pp_number(mem, buffer, strbuf, source_location));
}
MACRO_END

MACRO(date) {
    struct tm *tm = localtime(&predefined_scope->preprocessor->context->environment.timestamp);
    char strbuf[256] = {0};
    size_t count = strftime(strbuf, sizeof(strbuf), "%b %e %Y", tm);
    REQUIRE(count != 0, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Failed to format current date"));

    struct kefir_token token;
    REQUIRE_OK(kefir_token_new_string_literal_raw_from_escaped_multibyte(mem, KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
                                                                         strbuf, count, &token));
    token.source_location = *source_location;
    kefir_result_t res = kefir_token_buffer_emplace(mem, buffer, &token);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, &token);
        return res;
    });
}
MACRO_END

MACRO(time) {
    struct tm *tm = localtime(&predefined_scope->preprocessor->context->environment.timestamp);
    char strbuf[256] = {0};
    size_t count = strftime(strbuf, sizeof(strbuf), "%H:%M:%S", tm);
    REQUIRE(count != 0, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Failed to format current time"));

    struct kefir_token token;
    REQUIRE_OK(kefir_token_new_string_literal_raw_from_escaped_multibyte(mem, KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
                                                                         strbuf, count, &token));
    token.source_location = *source_location;
    kefir_result_t res = kefir_token_buffer_emplace(mem, buffer, &token);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, &token);
        return res;
    });
}
MACRO_END

#define MACRO_PP_NUMBER_FMT(_name, _buflen, _format, ...)                 \
    MACRO(_name) {                                                        \
        char strbuf[_buflen + 1] = {0};                                   \
        snprintf(strbuf, sizeof(strbuf) - 1, _format, __VA_ARGS__);       \
        REQUIRE_OK(make_pp_number(mem, buffer, strbuf, source_location)); \
    }                                                                     \
    MACRO_END

MACRO_PP_NUMBER_FMT(stdc_hosted, 64, "%d", predefined_scope->preprocessor->context->environment.hosted ? 1 : 0)
MACRO_PP_NUMBER_FMT(stdc_version, 64, KEFIR_ULONG_FMT "L", predefined_scope->preprocessor->context->environment.version)
MACRO_PP_NUMBER_FMT(stdc_iso_10646, 64, KEFIR_ULONG_FMT "L",
                    predefined_scope->preprocessor->context->environment.stdc_iso10646)
MACRO_PP_NUMBER_FMT(stdc_lib_ext1, 64, KEFIR_ULONG_FMT "L",
                    predefined_scope->preprocessor->context->environment.stdc_lib_ext1)
MACRO_PP_NUMBER_FMT(produce_one, 64, KEFIR_INT_FMT, 1)
MACRO_PP_NUMBER_FMT(big_endian, 64, KEFIR_INT_FMT, 1234)
MACRO_PP_NUMBER_FMT(little_endian, 64, KEFIR_INT_FMT, 4321)
MACRO_PP_NUMBER_FMT(pdp_endian, 64, KEFIR_INT_FMT, 3412)
MACRO_PP_NUMBER_FMT(char_bit, 64, KEFIR_UINT64_FMT, preprocessor->context->environment.data_model->char_bit)
MACRO_PP_NUMBER_FMT(schar_max, 64, KEFIR_UINT64_FMT,
                    (1ul << (preprocessor->context->environment.data_model->char_bit - 1)) - 1)
MACRO_PP_NUMBER_FMT(shrt_max, 64, KEFIR_UINT64_FMT,
                    (1ul << (preprocessor->context->environment.data_model->int_width.short_int - 1)) - 1)
MACRO_PP_NUMBER_FMT(int_max, 64, KEFIR_UINT64_FMT,
                    (1ul << (preprocessor->context->environment.data_model->int_width.integer - 1)) - 1)
MACRO_PP_NUMBER_FMT(long_max, 64, KEFIR_UINT64_FMT,
                    (1ul << (preprocessor->context->environment.data_model->int_width.long_int - 1)) - 1)
MACRO_PP_NUMBER_FMT(long_long_max, 64, KEFIR_UINT64_FMT,
                    (1ul << (preprocessor->context->environment.data_model->int_width.long_long_int - 1)) - 1)
MACRO_PP_NUMBER_FMT(shrt_width, 64, KEFIR_UINT64_FMT,
                    preprocessor->context->environment.data_model->int_width.short_int)
MACRO_PP_NUMBER_FMT(int_width, 64, KEFIR_UINT64_FMT, preprocessor->context->environment.data_model->int_width.integer)
MACRO_PP_NUMBER_FMT(long_width, 64, KEFIR_UINT64_FMT, preprocessor->context->environment.data_model->int_width.long_int)
MACRO_PP_NUMBER_FMT(long_long_width, 64, KEFIR_UINT64_FMT,
                    preprocessor->context->environment.data_model->int_width.long_long_int)
MACRO_PP_NUMBER_FMT(flt_radix, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.float_radix)
MACRO_PP_NUMBER_FMT(flt_mant_dig, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.float_mantissa_digits)
MACRO_PP_NUMBER_FMT(dbl_mant_dig, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.double_mantissa_digits)
MACRO_PP_NUMBER_FMT(ldbl_mant_dig, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.long_double_mantissa_digits)
MACRO_PP_NUMBER_FMT(flt_dig, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.float_digits)
MACRO_PP_NUMBER_FMT(dbl_dig, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.double_digits)
MACRO_PP_NUMBER_FMT(ldbl_dig, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.long_double_digits)
MACRO_PP_NUMBER_FMT(flt_min_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.float_min_exponent)
MACRO_PP_NUMBER_FMT(dbl_min_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.double_min_exponent)
MACRO_PP_NUMBER_FMT(ldbl_min_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.long_double_min_exponent)
MACRO_PP_NUMBER_FMT(flt_min10_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.float_min10_exponent)
MACRO_PP_NUMBER_FMT(dbl_min10_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.double_min10_exponent)
MACRO_PP_NUMBER_FMT(ldbl_min10_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.long_double_min10_exponent)
MACRO_PP_NUMBER_FMT(flt_max_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.float_max_exponent)
MACRO_PP_NUMBER_FMT(dbl_max_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.double_max_exponent)
MACRO_PP_NUMBER_FMT(ldbl_max_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.long_double_max_exponent)
MACRO_PP_NUMBER_FMT(flt_max10_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.float_max10_exponent)
MACRO_PP_NUMBER_FMT(dbl_max10_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.double_max10_exponent)
MACRO_PP_NUMBER_FMT(ldbl_max10_exp, 64, KEFIR_INT64_FMT,
                    preprocessor->context->environment.data_model->floating_point.long_double_max10_exponent)
MACRO_PP_NUMBER_FMT(flt_min, 64, "%s", preprocessor->context->environment.data_model->floating_point.float_min)
MACRO_PP_NUMBER_FMT(dbl_min, 64, "%s", preprocessor->context->environment.data_model->floating_point.double_min)
MACRO_PP_NUMBER_FMT(ldbl_min, 64, "%s", preprocessor->context->environment.data_model->floating_point.long_double_min)
MACRO_PP_NUMBER_FMT(flt_epsilon, 64, "%s", preprocessor->context->environment.data_model->floating_point.float_epsilon)
MACRO_PP_NUMBER_FMT(dbl_epsilon, 64, "%s", preprocessor->context->environment.data_model->floating_point.double_epsilon)
MACRO_PP_NUMBER_FMT(ldbl_epsilon, 64, "%s",
                    preprocessor->context->environment.data_model->floating_point.long_double_epsilon)
MACRO_PP_NUMBER_FMT(flt_max, 64, "%s", preprocessor->context->environment.data_model->floating_point.float_max)
MACRO_PP_NUMBER_FMT(dbl_max, 64, "%s", preprocessor->context->environment.data_model->floating_point.double_max)
MACRO_PP_NUMBER_FMT(ldbl_max, 64, "%s", preprocessor->context->environment.data_model->floating_point.long_double_max)

static kefir_result_t define_predefined_macro(
    struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
    struct kefir_preprocessor_predefined_macro_scope *scope, struct kefir_preprocessor_macro *macro,
    const char *identifier,
    kefir_result_t (*apply)(struct kefir_mem *, struct kefir_preprocessor *, const struct kefir_preprocessor_macro *,
                            struct kefir_symbol_table *, const struct kefir_list *, struct kefir_token_buffer *,
                            const struct kefir_source_location *)) {
    macro->identifier = identifier;
    macro->type = KEFIR_PREPROCESSOR_MACRO_OBJECT;
    macro->payload = scope;
    macro->apply = apply;
    macro->argc = predefined_macro_argc;

    if (!kefir_hashtree_has(&preprocessor->context->undefined_macros, (kefir_hashtree_key_t) identifier)) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &scope->macro_tree, (kefir_hashtree_key_t) identifier,
                                         (kefir_hashtree_value_t) macro));
    }
    return KEFIR_OK;
}

static kefir_result_t locate_predefined(const struct kefir_preprocessor_macro_scope *scope, const char *identifier,
                                        const struct kefir_preprocessor_macro **macro) {
    REQUIRE(scope != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid overlay macro scope"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to preprocessor macro"));
    ASSIGN_DECL_CAST(struct kefir_preprocessor_predefined_macro_scope *, predefined_scope, scope->payload);

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&predefined_scope->macro_tree, (kefir_hashtree_key_t) identifier, &node);
    if (res == KEFIR_NOT_FOUND) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested identifier was not found in predefined macro scope");
    } else {
        REQUIRE_OK(res);
        *macro = (void *) node->value;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_predefined_macro_scope_init(struct kefir_mem *mem,
                                                              struct kefir_preprocessor_predefined_macro_scope *scope,
                                                              struct kefir_preprocessor *preprocessor) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scope != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to predefined macro scope"));
    REQUIRE(preprocessor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor"));

    scope->preprocessor = preprocessor;
    scope->scope.payload = scope;
    scope->scope.locate = locate_predefined;
    REQUIRE_OK(kefir_hashtree_init(&scope->macro_tree, &kefir_hashtree_str_ops));

    kefir_result_t res =
        define_predefined_macro(mem, preprocessor, scope, &scope->macros.file, "__FILE__", macro_file_apply);
    REQUIRE_CHAIN(&res,
                  define_predefined_macro(mem, preprocessor, scope, &scope->macros.line, "__LINE__", macro_line_apply));
    REQUIRE_CHAIN(&res,
                  define_predefined_macro(mem, preprocessor, scope, &scope->macros.date, "__DATE__", macro_date_apply));
    REQUIRE_CHAIN(&res,
                  define_predefined_macro(mem, preprocessor, scope, &scope->macros.time, "__TIME__", macro_time_apply));
    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc, "__STDC__",
                                                macro_produce_one_apply));
    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_hosted, "__STDC_HOSTED__",
                                                macro_stdc_hosted_apply));
    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_version,
                                                "__STDC_VERSION__", macro_stdc_version_apply));

    if (preprocessor->context->environment.stdc_iso10646 > 0) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_iso_10646,
                                                    "__STDC_ISO_10646__", macro_stdc_iso_10646_apply));
    }
    if (preprocessor->context->environment.stdc_mb_might_neq_wc) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_mb_might_neq_wc,
                                                    "__STDC_MB_MIGHT_NEQ_WC__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_utf16) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_utf16,
                                                    "__STDC_UTF_16__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_utf32) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_utf32,
                                                    "__STDC_UTF_32__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_analyzable) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_analyzable,
                                                    "__STDC_ANALYZABLE__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_iec559) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_iec559,
                                                    "__STDC_IEC_559__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_iec559_complex) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_iec559_complex,
                                                    "__STDC_IEC_559_COMPLEX__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_lib_ext1 > 0) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_lib_ext1,
                                                    "__STDC_LIB_EXT1__", macro_stdc_lib_ext1_apply));
    }
    if (preprocessor->context->environment.stdc_no_atomics) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_no_atomics,
                                                    "__STDC_NO_ATOMICS__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_no_complex) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_no_complex,
                                                    "__STDC_NO_COMPLEX__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_no_threads) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_no_threads,
                                                    "__STDC_NO_THREADS__", macro_produce_one_apply));
    }
    if (preprocessor->context->environment.stdc_no_vla) {
        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.stdc_no_vla,
                                                    "__STDC_NO_VLA__", macro_produce_one_apply));
    }
    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.kefircc, "__KEFIRCC__",
                                                macro_produce_one_apply));

    if (preprocessor->context->environment.data_model != NULL) {
        switch (preprocessor->context->environment.data_model->model) {
            case KEFIR_DATA_MODEL_UNKNOWN:
                // Intentionally left blank
                break;

            case KEFIR_DATA_MODEL_ILP32:
                REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.data_model,
                                                            "__ILP32__", macro_produce_one_apply));
                break;

            case KEFIR_DATA_MODEL_LLP64:
                REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.data_model,
                                                            "__LLP64__", macro_produce_one_apply));
                break;

            case KEFIR_DATA_MODEL_LP64:
                REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.data_model,
                                                            "__LP64__", macro_produce_one_apply));
                break;

            case KEFIR_DATA_MODEL_ILP64:
                REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.data_model,
                                                            "__ILP64__", macro_produce_one_apply));
                break;

            case KEFIR_DATA_MODEL_SILP64:
                REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.data_model,
                                                            "__SILP64__", macro_produce_one_apply));
                break;
        }

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.byte_order_big_endian,
                                                    "__ORDER_BIG_ENDIAN__", macro_big_endian_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.byte_order_little_endian,
                                                    "__ORDER_LITTLE_ENDIAN__", macro_little_endian_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.byte_order_pdp_endian,
                                                    "__ORDER_PDP_ENDIAN__", macro_pdp_endian_apply));
        switch (preprocessor->context->environment.data_model->byte_order) {
            case KEFIR_BYTE_ORDER_BIG_ENDIAN:
                REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.byte_order,
                                                            "__BYTE_ORDER__", macro_big_endian_apply));
                break;

            case KEFIR_BYTE_ORDER_LITTLE_ENDIAN:
                REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.byte_order,
                                                            "__BYTE_ORDER__", macro_little_endian_apply));
                break;

            case KEFIR_BYTE_ORDER_UNKNOWN:
                // Intentionally left blank
                break;
        }

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.char_bit, "__CHAR_BIT__",
                                                    macro_char_bit_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.schar_max,
                                                    "__SCHAR_MAX__", macro_schar_max_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.shrt_max,
                                                    "__SHRT_MAX__", macro_shrt_max_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.shrt_width,
                                                    "__SHRT_WIDTH__", macro_shrt_width_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.int_max,
                                                    "__INT_MAX__", macro_int_max_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.int_width,
                                                    "__INT_WIDTH__", macro_int_width_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.long_max,
                                                    "__LONG_MAX__", macro_long_max_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.long_width,
                                                    "__LONG_WIDTH__", macro_long_width_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.long_long_max,
                                                    "__LONG_LONG_MAX__", macro_long_long_max_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.long_long_width,
                                                    "__LONG_LONG_WIDTH__", macro_long_long_width_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_radix,
                                                    "__FLT_RADIX__", macro_flt_radix_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_mant_dig,
                                              "__FLT_MANT_DIG__", macro_flt_mant_dig_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_mant_dig,
                                              "__DBL_MANT_DIG__", macro_dbl_mant_dig_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_mant_dig,
                                              "__LDBL_MANT_DIG__", macro_ldbl_mant_dig_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_dig,
                                                    "__FLT_DIG__", macro_flt_dig_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_dig,
                                                    "__DBL_DIG__", macro_dbl_dig_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_dig,
                                                    "__LDBL_DIG__", macro_ldbl_dig_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_min_exp,
                                                    "__FLT_MIN_EXP__", macro_flt_min_exp_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_min_exp,
                                                    "__DBL_MIN_EXP__", macro_dbl_min_exp_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_min_exp,
                                              "__LDBL_MIN_EXP__", macro_ldbl_min_exp_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_min10_exp,
                                              "__FLT_MIN_10_EXP__", macro_flt_min10_exp_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_min10_exp,
                                              "__DBL_MIN_10_EXP__", macro_dbl_min10_exp_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_min10_exp,
                                              "__LDBL_MIN_10_EXP__", macro_ldbl_min10_exp_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_max_exp,
                                                    "__FLT_MAX_EXP__", macro_flt_max_exp_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_max_exp,
                                                    "__DBL_MAX_EXP__", macro_dbl_max_exp_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_max_exp,
                                              "__LDBL_MAX_EXP__", macro_ldbl_max_exp_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_max10_exp,
                                              "__FLT_MAX_10_EXP__", macro_flt_max10_exp_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_max10_exp,
                                              "__DBL_MAX_10_EXP__", macro_dbl_max10_exp_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_max10_exp,
                                              "__LDBL_MAX_10_EXP__", macro_ldbl_max10_exp_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_min,
                                                    "__FLT_MIN__", macro_flt_min_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_min,
                                                    "__DBL_MIN__", macro_dbl_min_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_min,
                                                    "__LDBL_MIN__", macro_ldbl_min_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_epsilon,
                                                    "__FLT_EPSILON__", macro_flt_epsilon_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_epsilon,
                                                    "__DBL_EPSILON__", macro_dbl_epsilon_apply));

        REQUIRE_CHAIN(&res,
                      define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_epsilon,
                                              "__LDBL_EPSILON__", macro_ldbl_epsilon_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.flt_max,
                                                    "__FLT_MAX__", macro_flt_max_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.dbl_max,
                                                    "__DBL_MAX__", macro_dbl_max_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.floating_point.ldbl_max,
                                                    "__LDBL_MAX__", macro_ldbl_max_apply));
    }

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &scope->macro_tree);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_predefined_macro_scope_free(struct kefir_mem *mem,
                                                              struct kefir_preprocessor_predefined_macro_scope *scope) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scope != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to predefined macro scope"));
    REQUIRE_OK(kefir_hashtree_free(mem, &scope->macro_tree));
    return KEFIR_OK;
}
