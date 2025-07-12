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

#include "kefir/preprocessor/predefined_macro.h"
#include "kefir/preprocessor/preprocessor.h"
#include "kefir/preprocessor/util.h"
#include "kefir/core/version.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/core/os_error.h"
#include <float.h>

struct predefined_macro_payload {
    struct kefir_preprocessor_predefined_macro_scope *scope;
    kefir_size_t argc;
    kefir_size_t vararg;
};

static kefir_result_t make_pp_number(struct kefir_mem *mem, struct kefir_token_allocator *token_allocator,
                                     struct kefir_token_buffer *buffer, const char *buf,
                                     const struct kefir_source_location *source_location) {
    struct kefir_token *allocated_token;
    if (buf[0] == '-') {
        buf++;
        REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_MINUS, allocated_token));
        allocated_token->source_location = *source_location;
        REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
    }

    REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
    REQUIRE_OK(kefir_token_new_pp_number(mem, buf, strlen(buf), allocated_token));
    allocated_token->source_location = *source_location;
    REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
    return KEFIR_OK;
}

static kefir_result_t predefined_macro_argc(const struct kefir_preprocessor_macro *macro, kefir_size_t *argc_ptr,
                                            kefir_bool_t *vararg_ptr) {
    UNUSED(macro);
    UNUSED(argc_ptr);
    UNUSED(vararg_ptr);
    return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to retrieve argument count of predefined object macro");
}

static kefir_result_t predefined_function_macro_argc(const struct kefir_preprocessor_macro *macro,
                                                     kefir_size_t *argc_ptr, kefir_bool_t *vararg_ptr) {
    REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor macro"));
    REQUIRE(argc_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to argument count"));
    REQUIRE(vararg_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to vararg flag"));
    ASSIGN_DECL_CAST(struct predefined_macro_payload *, payload, macro->payload);

    REQUIRE(macro->type == KEFIR_PREPROCESSOR_MACRO_FUNCTION,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to retrieve argument count of object macro"));
    *argc_ptr = payload->argc;
    *vararg_ptr = payload->vararg;
    return KEFIR_OK;
}

#define MACRO(_name)                                                                                                  \
    static kefir_result_t macro_##_name##_apply(                                                                      \
        struct kefir_mem *mem, struct kefir_preprocessor *preprocessor, const struct kefir_preprocessor_macro *macro, \
        struct kefir_string_pool *symbols, const struct kefir_list *args,                                             \
        struct kefir_token_allocator *token_allocator, struct kefir_token_buffer *buffer,                             \
        const struct kefir_source_location *source_location) {                                                        \
        UNUSED(symbols);                                                                                              \
        UNUSED(preprocessor);                                                                                         \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));            \
        REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor macro"));        \
        REQUIRE(args == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected empty macro argument list"));        \
        REQUIRE(token_allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token allocator")); \
        REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));             \
        REQUIRE(source_location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location")); \
        ASSIGN_DECL_CAST(struct predefined_macro_payload *, macro_payload, macro->payload);                           \
        UNUSED(macro_payload);                                                                                        \
                                                                                                                      \
        do

#define FUNCTION_MACRO(_name)                                                                                         \
    static kefir_result_t macro_##_name##_apply(                                                                      \
        struct kefir_mem *mem, struct kefir_preprocessor *preprocessor, const struct kefir_preprocessor_macro *macro, \
        struct kefir_string_pool *symbols, const struct kefir_list *args,                                             \
        struct kefir_token_allocator *token_allocator, struct kefir_token_buffer *buffer,                             \
        const struct kefir_source_location *source_location) {                                                        \
        UNUSED(symbols);                                                                                              \
        UNUSED(preprocessor);                                                                                         \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));            \
        REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor macro"));        \
        REQUIRE(token_allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token allocator")); \
        REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));             \
        REQUIRE(source_location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location")); \
        ASSIGN_DECL_CAST(struct predefined_macro_payload *, macro_payload, macro->payload);                           \
        UNUSED(macro_payload);                                                                                        \
                                                                                                                      \
        do

#define MACRO_END    \
    while (0)        \
        ;            \
    return KEFIR_OK; \
    }

MACRO(file) {
    const char *file = source_location->source;
    struct kefir_token *allocated_token;
    REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
    REQUIRE_OK(kefir_token_new_string_literal_raw_from_escaped_multibyte(mem, KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
                                                                         file, strlen(file), allocated_token));
    allocated_token->source_location = *source_location;
    REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
}
MACRO_END

MACRO(line) {
    kefir_source_location_line_t line = source_location->line;
    char strbuf[64] = {0};
    snprintf(strbuf, sizeof(strbuf), "%" KEFIR_UINT_FMT, line);
    REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, strbuf, source_location));
}
MACRO_END

MACRO(date) {
    struct tm *tm = localtime(&macro_payload->scope->preprocessor->context->environment.timestamp);
    char strbuf[256] = {0};
    size_t count = strftime(strbuf, sizeof(strbuf), "%b %e %Y", tm);
    REQUIRE(count != 0, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Failed to format current date"));

    struct kefir_token *allocated_token;
    REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
    REQUIRE_OK(kefir_token_new_string_literal_raw_from_escaped_multibyte(mem, KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
                                                                         strbuf, count, allocated_token));
    allocated_token->source_location = *source_location;
    REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
}
MACRO_END

MACRO(time) {
    struct tm *tm = localtime(&macro_payload->scope->preprocessor->context->environment.timestamp);
    char strbuf[256] = {0};
    size_t count = strftime(strbuf, sizeof(strbuf), "%H:%M:%S", tm);
    REQUIRE(count != 0, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Failed to format current time"));

    struct kefir_token *allocated_token;
    REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
    REQUIRE_OK(kefir_token_new_string_literal_raw_from_escaped_multibyte(mem, KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
                                                                         strbuf, count, allocated_token));
    allocated_token->source_location = *source_location;
    REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
}
MACRO_END

MACRO(kefircc_version) {
    char version_buf[256];
    int len = snprintf(version_buf, sizeof(version_buf) - 1, "%s", KEFIR_VERSION_SHORT);
    REQUIRE(len > 0, KEFIR_SET_OS_ERROR("Failed to format compiler version"));

    struct kefir_token *allocated_token;
    REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
    REQUIRE_OK(kefir_token_new_string_literal_raw_from_escaped_multibyte(mem, KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
                                                                         version_buf, len, allocated_token));
    allocated_token->source_location = *source_location;
    REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
}
MACRO_END

MACRO(kefircc_full_version) {
    char version_buf[256];
    int len = snprintf(version_buf, sizeof(version_buf) - 1, "%s", KEFIR_VERSION_FULL);
    REQUIRE(len > 0, KEFIR_SET_OS_ERROR("Failed to format compiler version"));

    struct kefir_token *allocated_token;
    REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
    REQUIRE_OK(kefir_token_new_string_literal_raw_from_escaped_multibyte(mem, KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
                                                                         version_buf, len, allocated_token));
    allocated_token->source_location = *source_location;
    REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
}
MACRO_END

#define MACRO_PP_NUMBER_FMT(_name, _buflen, _format, ...)                                  \
    MACRO(_name) {                                                                         \
        char strbuf[_buflen + 1] = {0};                                                    \
        snprintf(strbuf, sizeof(strbuf) - 1, _format, __VA_ARGS__);                        \
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, strbuf, source_location)); \
    }                                                                                      \
    MACRO_END

MACRO_PP_NUMBER_FMT(stdc_hosted, 64, "%d", macro_payload->scope->preprocessor->context->environment.hosted ? 1 : 0)
MACRO_PP_NUMBER_FMT(stdc_version, 64, "%" KEFIR_ULONG_FMT "L",
                    macro_payload->scope->preprocessor->context->environment.version)
MACRO_PP_NUMBER_FMT(stdc_iso_10646, 64, "%" KEFIR_ULONG_FMT "L",
                    macro_payload->scope->preprocessor->context->environment.stdc_iso10646)
MACRO_PP_NUMBER_FMT(stdc_lib_ext1, 64, "%" KEFIR_ULONG_FMT "L",
                    macro_payload->scope->preprocessor->context->environment.stdc_lib_ext1)
MACRO_PP_NUMBER_FMT(produce_one, 64, "%" KEFIR_INT_FMT, 1)
MACRO_PP_NUMBER_FMT(big_endian, 64, "%" KEFIR_INT_FMT, 4321)
MACRO_PP_NUMBER_FMT(little_endian, 64, "%" KEFIR_INT_FMT, 1234)
MACRO_PP_NUMBER_FMT(pdp_endian, 64, "%" KEFIR_INT_FMT, 3412)
MACRO_PP_NUMBER_FMT(char_bit, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.char_bits)
MACRO_PP_NUMBER_FMT(schar_max, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t)
                        kefir_data_model_descriptor_signed_char_max(preprocessor->context->environment.data_model))
MACRO_PP_NUMBER_FMT(shrt_max, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t)
                        kefir_data_model_descriptor_signed_short_max(preprocessor->context->environment.data_model))
MACRO_PP_NUMBER_FMT(int_max, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t)
                        kefir_data_model_descriptor_signed_int_max(preprocessor->context->environment.data_model))
MACRO_PP_NUMBER_FMT(long_max, 64, "%" KEFIR_UINT64_FMT "L",
                    (kefir_uint64_t)
                        kefir_data_model_descriptor_signed_long_max(preprocessor->context->environment.data_model))
MACRO_PP_NUMBER_FMT(long_long_max, 64, "%" KEFIR_UINT64_FMT "L",
                    (kefir_uint64_t)
                        kefir_data_model_descriptor_signed_long_long_max(preprocessor->context->environment.data_model))
MACRO_PP_NUMBER_FMT(schar_width, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.char_bits)
MACRO_PP_NUMBER_FMT(shrt_width, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.short_bits)
MACRO_PP_NUMBER_FMT(int_width, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.int_bits)
MACRO_PP_NUMBER_FMT(long_width, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.long_bits)
MACRO_PP_NUMBER_FMT(long_long_width, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.long_long_bits)
MACRO_PP_NUMBER_FMT(short_sizeof, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.short_bits /
                        preprocessor->context->environment.data_model->scalar_width.char_bits)
MACRO_PP_NUMBER_FMT(int_sizeof, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.int_bits /
                        preprocessor->context->environment.data_model->scalar_width.char_bits)
MACRO_PP_NUMBER_FMT(long_sizeof, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.long_bits /
                        preprocessor->context->environment.data_model->scalar_width.char_bits)
MACRO_PP_NUMBER_FMT(long_long_sizeof, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.long_long_bits /
                        preprocessor->context->environment.data_model->scalar_width.char_bits)
MACRO_PP_NUMBER_FMT(float_sizeof, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.float_bits /
                        preprocessor->context->environment.data_model->scalar_width.char_bits)
MACRO_PP_NUMBER_FMT(double_sizeof, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.double_bits /
                        preprocessor->context->environment.data_model->scalar_width.char_bits)
MACRO_PP_NUMBER_FMT(long_double_sizeof, 64, "%" KEFIR_UINT64_FMT,
                    (kefir_uint64_t) preprocessor->context->environment.data_model->scalar_width.long_double_bits /
                        preprocessor->context->environment.data_model->scalar_width.char_bits)

MACRO(counter) {
    kefir_uint_t counter = macro_payload->scope->preprocessor->context->state.counter++;
    char strbuf[64] = {0};
    snprintf(strbuf, sizeof(strbuf), "%" KEFIR_UINT_FMT, counter);
    REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, strbuf, source_location));
}
MACRO_END

FUNCTION_MACRO(has_attribute) {
    const struct kefir_list_entry *args_iter = kefir_list_head(args);
    REQUIRE(args_iter != NULL && args_iter->next == NULL,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_attribute expects a single identifier argument"));
    ASSIGN_DECL_CAST(const struct kefir_token_buffer *, arg_buffer, args_iter->value);
    REQUIRE(kefir_token_buffer_length(arg_buffer) == 1,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_attribute expects a single identifier argument"));
    const struct kefir_token *arg = kefir_token_buffer_at(arg_buffer, 0);
    REQUIRE(arg->klass == KEFIR_TOKEN_IDENTIFIER,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_attribute expects a single identifier argument"));

    if (kefir_hashtreeset_has(&preprocessor->context->environment.supported_attributes,
                              (kefir_hashtreeset_entry_t) arg->identifier)) {
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, "1", source_location));
    } else {
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, "0", source_location));
    }
    return KEFIR_OK;
}
MACRO_END

FUNCTION_MACRO(has_include) {
    const struct kefir_list_entry *args_iter = kefir_list_head(args);
    REQUIRE(args_iter != NULL && args_iter->next == NULL,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_include expects a single header name argument"));
    ASSIGN_DECL_CAST(struct kefir_token_buffer *, arg_buffer, args_iter->value);

    REQUIRE_OK(kefir_preprocessor_run_substitutions(mem, preprocessor, token_allocator, arg_buffer, NULL,
                                                    KEFIR_PREPROCESSOR_SUBSTITUTION_NORMAL));

    REQUIRE(kefir_token_buffer_length(arg_buffer) > 0,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_include expects a single header name argument"));
    const struct kefir_token *arg = kefir_token_buffer_at(arg_buffer, 0);

    kefir_result_t res;
    struct kefir_preprocessor_source_file source_file;
    if (arg->klass == KEFIR_TOKEN_STRING_LITERAL) {
        REQUIRE(arg->string_literal.type == KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE ||
                    arg->string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE8,
                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                       "Macro __has_include expects a single header name argument"));

        const char *filepath = arg->string_literal.literal;
        if (arg->string_literal.raw_literal) {
            REQUIRE_OK(
                kefir_preprocessor_convert_raw_string_into_multibyte(mem, preprocessor->lexer.symbols, arg, &filepath));
        }

        res = preprocessor->context->source_locator->open(mem, preprocessor->context->source_locator, filepath, false,
                                                          preprocessor->current_file,
                                                          KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NORMAL, &source_file);
    } else if (arg->klass == KEFIR_TOKEN_PP_HEADER_NAME) {
        res = preprocessor->context->source_locator->open(
            mem, preprocessor->context->source_locator, arg->pp_header_name.header_name, arg->pp_header_name.system,
            preprocessor->current_file, KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NORMAL, &source_file);
    } else {
        const char *header_name;
        res = kefir_preprocessor_construct_system_header_name_from_buffer(mem, arg_buffer, preprocessor->lexer.symbols,
                                                                          &header_name);
        REQUIRE_CHAIN(&res, preprocessor->context->source_locator->open(mem, preprocessor->context->source_locator,
                                                                        header_name, true, preprocessor->current_file,
                                                                        KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NORMAL,
                                                                        &source_file));
    }

    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        REQUIRE_OK(source_file.close(mem, &source_file));
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, "1", source_location));
    } else {
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, "0", source_location));
    }
    return KEFIR_OK;
}
MACRO_END

FUNCTION_MACRO(has_include_next) {
    const struct kefir_list_entry *args_iter = kefir_list_head(args);
    REQUIRE(args_iter != NULL && args_iter->next == NULL,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_include_next expects a single header name argument"));
    ASSIGN_DECL_CAST(struct kefir_token_buffer *, arg_buffer, args_iter->value);

    REQUIRE_OK(kefir_preprocessor_run_substitutions(mem, preprocessor, token_allocator, arg_buffer, NULL,
                                                    KEFIR_PREPROCESSOR_SUBSTITUTION_NORMAL));

    REQUIRE(kefir_token_buffer_length(arg_buffer) > 0,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_include_next expects a single header name argument"));
    const struct kefir_token *arg = kefir_token_buffer_at(arg_buffer, 0);

    kefir_result_t res;
    struct kefir_preprocessor_source_file source_file;
    if (arg->klass == KEFIR_TOKEN_STRING_LITERAL) {
        REQUIRE(arg->string_literal.type == KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE ||
                    arg->string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE8,
                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                       "Macro __has_include_next expects a single header name argument"));

        const char *filepath = arg->string_literal.literal;
        if (arg->string_literal.raw_literal) {
            REQUIRE_OK(
                kefir_preprocessor_convert_raw_string_into_multibyte(mem, preprocessor->lexer.symbols, arg, &filepath));
        }

        res = preprocessor->context->source_locator->open(mem, preprocessor->context->source_locator, filepath, false,
                                                          preprocessor->current_file,
                                                          KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NEXT, &source_file);
    } else if (arg->klass == KEFIR_TOKEN_PP_HEADER_NAME) {
        res = preprocessor->context->source_locator->open(
            mem, preprocessor->context->source_locator, arg->pp_header_name.header_name, arg->pp_header_name.system,
            preprocessor->current_file, KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NEXT, &source_file);
    } else {
        const char *header_name;
        res = kefir_preprocessor_construct_system_header_name_from_buffer(mem, arg_buffer, preprocessor->lexer.symbols,
                                                                          &header_name);
        REQUIRE_CHAIN(&res, preprocessor->context->source_locator->open(
                                mem, preprocessor->context->source_locator, header_name, true,
                                preprocessor->current_file, KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NEXT, &source_file));
    }

    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        REQUIRE_OK(source_file.close(mem, &source_file));
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, "1", source_location));
    } else {
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, "0", source_location));
    }
    return KEFIR_OK;
}
MACRO_END

FUNCTION_MACRO(has_builtin) {
    const struct kefir_list_entry *args_iter = kefir_list_head(args);
    REQUIRE(args_iter != NULL && args_iter->next == NULL,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_builtin expects a single identifier argument"));
    ASSIGN_DECL_CAST(const struct kefir_token_buffer *, arg_buffer, args_iter->value);
    REQUIRE(kefir_token_buffer_length(arg_buffer) == 1,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_builtin expects a single identifier argument"));
    const struct kefir_token *arg = kefir_token_buffer_at(arg_buffer, 0);
    REQUIRE(arg->klass == KEFIR_TOKEN_IDENTIFIER,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __has_builtin expects a single identifier argument"));

    if (kefir_hashtreeset_has(&preprocessor->context->environment.supported_builtins,
                              (kefir_hashtreeset_entry_t) arg->identifier)) {
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, "1", source_location));
    } else {
        REQUIRE_OK(make_pp_number(mem, token_allocator, buffer, "0", source_location));
    }

    return KEFIR_OK;
}
MACRO_END

FUNCTION_MACRO(define_builtin_prefix) {
    const struct kefir_list_entry *args_iter = kefir_list_head(args);
    REQUIRE(args_iter != NULL && args_iter->next == NULL,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __kefir_define_builtin_prefix expects a single identifier argument"));
    ASSIGN_DECL_CAST(const struct kefir_token_buffer *, arg_buffer, args_iter->value);
    REQUIRE(kefir_token_buffer_length(arg_buffer) == 1,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __kefir_define_builtin_prefix expects a single identifier argument"));
    const struct kefir_token *arg = kefir_token_buffer_at(arg_buffer, 0);
    REQUIRE(arg->klass == KEFIR_TOKEN_IDENTIFIER,
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                   "Macro __kefir_define_builtin_prefix expects a single identifier argument"));

    const char *const identifier =
        kefir_string_pool_insert(mem, preprocessor->context->ast_context->symbols, arg->identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into string pool"));
    REQUIRE_OK(
        kefir_hashtreeset_add(mem, &preprocessor->context->builltin_prefixes, (kefir_hashtreeset_entry_t) identifier));
    return KEFIR_OK;
}
MACRO_END

MACRO(supported_builtins) {
    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&preprocessor->context->environment.supported_builtins, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, builtin, iter.entry);
        struct kefir_token *allocated_token;
        REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
        REQUIRE_OK(kefir_token_new_identifier(mem, symbols, builtin, allocated_token));
        allocated_token->source_location = *source_location;
        REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));

        REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
        REQUIRE_OK(kefir_token_new_pp_whitespace(true, allocated_token));
        allocated_token->source_location = *source_location;
        REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
MACRO_END

MACRO(supported_attributes) {
    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&preprocessor->context->environment.supported_attributes, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, builtin, iter.entry);
        struct kefir_token *allocated_token;
        REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
        REQUIRE_OK(kefir_token_new_identifier(mem, symbols, builtin, allocated_token));
        allocated_token->source_location = *source_location;
        REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));

        REQUIRE_OK(kefir_token_allocator_allocate_empty(mem, token_allocator, &allocated_token));
        REQUIRE_OK(kefir_token_new_pp_whitespace(true, allocated_token));
        allocated_token->source_location = *source_location;
        REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, allocated_token));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
MACRO_END

static kefir_result_t define_predefined_macro(
    struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
    struct kefir_preprocessor_predefined_macro_scope *scope, struct kefir_preprocessor_macro *macro,
    const char *identifier,
    kefir_result_t (*apply)(struct kefir_mem *, struct kefir_preprocessor *, const struct kefir_preprocessor_macro *,
                            struct kefir_string_pool *, const struct kefir_list *, struct kefir_token_allocator *,
                            struct kefir_token_buffer *, const struct kefir_source_location *)) {
    struct predefined_macro_payload *payload = KEFIR_MALLOC(mem, sizeof(struct predefined_macro_payload));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate predefined macro payload"));
    payload->scope = scope;
    payload->argc = 0;
    payload->vararg = false;
    macro->identifier = identifier;
    macro->type = KEFIR_PREPROCESSOR_MACRO_OBJECT;
    macro->payload = payload;
    macro->apply = apply;
    macro->argc = predefined_macro_argc;

    if (!kefir_hashtree_has(&preprocessor->context->undefined_macros, (kefir_hashtree_key_t) identifier)) {
        kefir_result_t res = kefir_hashtree_insert(mem, &scope->macro_tree, (kefir_hashtree_key_t) identifier,
                                                   (kefir_hashtree_value_t) macro);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, payload);
            return res;
        });
    } else {
        KEFIR_FREE(mem, payload);
    }
    return KEFIR_OK;
}

static kefir_result_t define_predefined_function_macro(
    struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
    struct kefir_preprocessor_predefined_macro_scope *scope, struct kefir_preprocessor_macro *macro,
    const char *identifier,
    kefir_result_t (*apply)(struct kefir_mem *, struct kefir_preprocessor *, const struct kefir_preprocessor_macro *,
                            struct kefir_string_pool *, const struct kefir_list *, struct kefir_token_allocator *,
                            struct kefir_token_buffer *, const struct kefir_source_location *),
    kefir_size_t argc, kefir_bool_t vararg) {
    struct predefined_macro_payload *payload = KEFIR_MALLOC(mem, sizeof(struct predefined_macro_payload));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate predefined macro payload"));
    payload->scope = scope;
    payload->argc = argc;
    payload->vararg = vararg;
    macro->identifier = identifier;
    macro->type = KEFIR_PREPROCESSOR_MACRO_FUNCTION;
    macro->payload = payload;
    macro->apply = apply;
    macro->argc = predefined_function_macro_argc;

    if (!kefir_hashtree_has(&preprocessor->context->undefined_macros, (kefir_hashtree_key_t) identifier)) {
        kefir_result_t res = kefir_hashtree_insert(mem, &scope->macro_tree, (kefir_hashtree_key_t) identifier,
                                                   (kefir_hashtree_value_t) macro);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, payload);
            return res;
        });
    } else {
        KEFIR_FREE(mem, payload);
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

static kefir_result_t free_macro(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                 kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_preprocessor_macro *, macro, value);
    REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid predefined macro"));

    KEFIR_FREE(mem, macro->payload);
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
    REQUIRE_OK(kefir_hashtree_on_removal(&scope->macro_tree, free_macro, NULL));

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

    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.kefircc_version,
                                                "__KEFIRCC_VERSION__", macro_kefircc_version_apply));

    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.kefircc_full_version,
                                                "__KEFIRCC_FULL_VERSION__", macro_kefircc_full_version_apply));

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

        if (!preprocessor->context->ast_context->type_traits->character_type_signedness) {
            REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.char_unsigned,
                                                        "__CHAR_UNSIGNED__", macro_produce_one_apply));
        }

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.char_bit, "__CHAR_BIT__",
                                                    macro_char_bit_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.schar_max,
                                                    "__SCHAR_MAX__", macro_schar_max_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.limits.schar_width,
                                                    "__SCHAR_WIDTH__", macro_schar_width_apply));

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

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.sizes.short_sizeof,
                                                    "__SIZEOF_SHORT__", macro_short_sizeof_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.sizes.int_sizeof,
                                                    "__SIZEOF_INT__", macro_int_sizeof_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.sizes.long_sizeof,
                                                    "__SIZEOF_LONG__", macro_long_sizeof_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.sizes.long_long_sizeof,
                                                    "__SIZEOF_LONG_LONG__", macro_long_long_sizeof_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.sizes.float_sizeof,
                                                    "__SIZEOF_FLOAT__", macro_float_sizeof_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.sizes.double_sizeof,
                                                    "__SIZEOF_DOUBLE__", macro_double_sizeof_apply));

        REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.sizes.long_double_sizeof,
                                                    "__SIZEOF_LONG_DOUBLE__", macro_long_double_sizeof_apply));
    }

    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.counter, "__COUNTER__",
                                                macro_counter_apply));

    REQUIRE_CHAIN(&res, define_predefined_function_macro(mem, preprocessor, scope, &scope->macros.has_attribute,
                                                         "__has_attribute", macro_has_attribute_apply, 1, false));
    REQUIRE_CHAIN(&res, define_predefined_function_macro(mem, preprocessor, scope, &scope->macros.has_include,
                                                         "__has_include", macro_has_include_apply, 1, false));
    REQUIRE_CHAIN(&res, define_predefined_function_macro(mem, preprocessor, scope, &scope->macros.has_include_next,
                                                         "__has_include_next", macro_has_include_next_apply, 1, false));
    REQUIRE_CHAIN(&res, define_predefined_function_macro(mem, preprocessor, scope, &scope->macros.has_builtin,
                                                         "__has_builtin", macro_has_builtin_apply, 1, false));
    REQUIRE_CHAIN(&res, define_predefined_function_macro(mem, preprocessor, scope, &scope->macros.define_builtin_prefix,
                                                         "__kefir_define_builtin_prefix",
                                                         macro_define_builtin_prefix_apply, 1, false));

    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.supported_builtins,
                                                "__KEFIRCC_SUPPORTED_BUILTINS__", macro_supported_builtins_apply));
    REQUIRE_CHAIN(&res, define_predefined_macro(mem, preprocessor, scope, &scope->macros.supported_attributes,
                                                "__KEFIRCC_SUPPORTED_ATTRIBUTES__", macro_supported_attributes_apply));

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
