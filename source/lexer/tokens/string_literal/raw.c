#include "kefir/lexer/lexer.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/core/util.h"
#include "kefir/core/string_buffer.h"

struct params {
    struct kefir_token *token;
};

static kefir_result_t scan_string(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                  struct kefir_string_buffer *strbuf) {
    kefir_char32_t chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
    for (; chr != KEFIR_LEXER_SOURCE_CURSOR_EOF && chr != U'\"';) {

        if (chr == U'\\') {
            REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
            REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
            chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
            if (chr != KEFIR_LEXER_SOURCE_CURSOR_EOF) {
                REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
                REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
                chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
            }
        } else {
            REQUIRE(chr != U'\n', KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location,
                                                         "Unexpected newline character"));
            REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
            REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
            chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
        }
    }

    REQUIRE(chr == U'\"', KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location,
                                                 "Expected string terminating double quote"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
    return KEFIR_OK;
}

static kefir_result_t match_impl(struct kefir_mem *mem, struct kefir_lexer *lexer, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct params *, params, payload);

    kefir_string_literal_token_type_t type;
    kefir_result_t res = kefir_lexer_cursor_match_string(lexer->cursor, U"\"");
    if (res != KEFIR_NO_MATCH) {
        REQUIRE_OK(res);
        type = KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE;
        REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
    }

    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_cursor_match_string(lexer->cursor, U"u8\"");
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            type = KEFIR_STRING_LITERAL_TOKEN_UNICODE8;
            REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 3));
        }
    }

    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_cursor_match_string(lexer->cursor, U"u\"");
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            type = KEFIR_STRING_LITERAL_TOKEN_UNICODE16;
            REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 2));
        }
    }

    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_cursor_match_string(lexer->cursor, U"U\"");
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            type = KEFIR_STRING_LITERAL_TOKEN_UNICODE32;
            REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 2));
        }
    }

    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_cursor_match_string(lexer->cursor, U"L\"");
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            type = KEFIR_STRING_LITERAL_TOKEN_WIDE;
            REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 2));
        }
    }

    if (res == KEFIR_NO_MATCH) {
        return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match raw string literal");
    }

    struct kefir_string_buffer strbuf;
    REQUIRE_OK(kefir_string_buffer_init(mem, &strbuf, KEFIR_STRING_BUFFER_UNICODE32));

    res = scan_string(mem, lexer, &strbuf);
    if (res == KEFIR_OK) {
        kefir_size_t length = 0;
        const kefir_char32_t *content = kefir_string_buffer_value(&strbuf, &length);
        res = kefir_token_new_string_literal_raw(mem, type, content, length, params->token);
    }
    REQUIRE_ELSE(res == KEFIR_OK, { kefir_string_buffer_free(mem, &strbuf); });

    res = kefir_string_buffer_free(mem, &strbuf);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, params->token);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_match_raw_string_literal(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                    struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    struct params params = {.token = token};
    REQUIRE_OK(kefir_lexer_apply(mem, lexer, match_impl, &params));
    return KEFIR_OK;
}
