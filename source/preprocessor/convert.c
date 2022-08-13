#include "kefir/preprocessor/preprocessor.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/core/string_buffer.h"
#include <string.h>

kefir_result_t kefir_preprocessor_token_convert(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                                struct kefir_token *dst, const struct kefir_token *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(preprocessor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to destination token"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token"));

    switch (src->klass) {
        case KEFIR_TOKEN_PP_NUMBER: {
            struct kefir_lexer_source_cursor_state state;
            struct kefir_lexer_source_cursor cursor;
            REQUIRE_OK(kefir_lexer_source_cursor_init(&cursor, src->identifier, strlen(src->identifier), ""));
            cursor.location = src->source_location;
            REQUIRE_OK(kefir_lexer_source_cursor_save(&cursor, &state));

            kefir_result_t res = kefir_lexer_scan_floating_point_constant(mem, &cursor, dst);
            if (res == KEFIR_NO_MATCH) {
                REQUIRE_OK(kefir_lexer_source_cursor_restore(&cursor, &state));
                res = kefir_lexer_scan_integral_constant(&cursor, preprocessor->lexer.context, dst);
            }
            REQUIRE_OK(res);
            REQUIRE_ELSE(cursor.index == cursor.length, {
                kefir_token_free(mem, dst);
                return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &cursor.location,
                                              "Unexpected character in numeric token");
            });
            dst->source_location = src->source_location;
        } break;

        case KEFIR_TOKEN_IDENTIFIER: {
            struct kefir_lexer_source_cursor cursor;
            REQUIRE_OK(kefir_lexer_source_cursor_init(&cursor, src->identifier, strlen(src->identifier), ""));
            cursor.location = src->source_location;

            REQUIRE_OK(kefir_lexer_scan_identifier_or_keyword(mem, &cursor, preprocessor->lexer.symbols,
                                                              &preprocessor->lexer.keywords, dst));
            REQUIRE_ELSE(cursor.index == cursor.length, {
                kefir_token_free(mem, dst);
                return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &cursor.location,
                                              "Unexpected character in identifier/keyword token");
            });
            dst->source_location = src->source_location;
        } break;

        case KEFIR_TOKEN_SENTINEL:
        case KEFIR_TOKEN_KEYWORD:
        case KEFIR_TOKEN_CONSTANT:
        case KEFIR_TOKEN_STRING_LITERAL:
        case KEFIR_TOKEN_PUNCTUATOR:
        case KEFIR_TOKEN_EXTENSION:
            REQUIRE_OK(kefir_token_copy(mem, dst, src));
            break;

        case KEFIR_TOKEN_PP_WHITESPACE:
        case KEFIR_TOKEN_PP_HEADER_NAME:
        case KEFIR_TOKEN_PP_PLACEMAKER:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unable to convert preprocessor token to a lexer token");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_token_convert_buffer(struct kefir_mem *mem, struct kefir_preprocessor *preprocessor,
                                                       struct kefir_token_buffer *dst,
                                                       const struct kefir_token_buffer *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(preprocessor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid preprocessor"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token buffer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token buffer"));

    for (kefir_size_t i = 0; i < src->length;) {
        switch (src->tokens[i].klass) {
            case KEFIR_TOKEN_PP_NUMBER:
            case KEFIR_TOKEN_IDENTIFIER:
            case KEFIR_TOKEN_SENTINEL:
            case KEFIR_TOKEN_KEYWORD:
            case KEFIR_TOKEN_CONSTANT:
            case KEFIR_TOKEN_PUNCTUATOR:
            case KEFIR_TOKEN_EXTENSION: {
                struct kefir_token token;
                REQUIRE_OK(kefir_preprocessor_token_convert(mem, preprocessor, &token, &src->tokens[i]));
                kefir_result_t res = kefir_token_buffer_emplace(mem, dst, &token);
                REQUIRE_ELSE(res == KEFIR_OK, {
                    kefir_token_free(mem, &token);
                    return res;
                });
                i++;
            } break;

            case KEFIR_TOKEN_STRING_LITERAL: {
                struct kefir_token token;
                struct kefir_list literals;
                REQUIRE_OK(kefir_list_init(&literals));
                kefir_result_t res = KEFIR_OK;
                kefir_string_literal_token_type_t type = KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE;
                while (res == KEFIR_OK && i < src->length &&
                       (src->tokens[i].klass == KEFIR_TOKEN_STRING_LITERAL ||
                        src->tokens[i].klass == KEFIR_TOKEN_PP_WHITESPACE)) {
                    if (src->tokens[i].klass == KEFIR_TOKEN_STRING_LITERAL) {
                        REQUIRE(src->tokens[i].string_literal.raw_literal,
                                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected raw string literal"));
                        if (!kefir_token_string_literal_type_concat(type, src->tokens[i].string_literal.type, &type)) {
                            break;
                        }
                        res = kefir_list_insert_after(mem, &literals, kefir_list_tail(&literals),
                                                      (void *) &src->tokens[i++]);
                    } else {
                        i++;
                    }
                }
                REQUIRE_CHAIN(&res, kefir_lexer_merge_raw_string_literals(mem, &literals, &token));
                REQUIRE_ELSE(res == KEFIR_OK, {
                    kefir_list_free(mem, &literals);
                    return res;
                });
                res = kefir_list_free(mem, &literals);
                REQUIRE_CHAIN(&res, kefir_token_buffer_emplace(mem, dst, &token));
                REQUIRE_ELSE(res == KEFIR_OK, {
                    kefir_token_free(mem, &token);
                    return res;
                });
            } break;

            case KEFIR_TOKEN_PP_WHITESPACE:
                // Skip whitespaces
                i++;
                break;

            case KEFIR_TOKEN_PP_HEADER_NAME:
            case KEFIR_TOKEN_PP_PLACEMAKER:
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                       "Encountered unexpected preprocessor token during conversion");
        }
    }
    return KEFIR_OK;
}
