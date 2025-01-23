#include "kefir/lexer/lexer.h"
#include "kefir/lexer/base.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/core/util.h"
#include "kefir/core/string_buffer.h"
#include "kefir/util/char32.h"

static kefir_result_t merge_literals(struct kefir_mem *mem, const struct kefir_list *literals,
                                     struct kefir_string_buffer *strbuf) {
    for (const struct kefir_list_entry *iter = kefir_list_head(literals); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_token *, token, iter->value);

        const kefir_char32_t *content = token->string_literal.literal;
        kefir_size_t length = token->string_literal.length;
        REQUIRE(length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected non-empty raw string literal"));
        length--;  // Drop trailing null character
        for (kefir_size_t j = 0; j < length;) {
            kefir_char32_t chr = content[j];
            if (chr == U'\\') {
                REQUIRE(j + 1 < length, KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                               "Expected escape sequence after '\\'"));
                chr = content[++j];
                switch (chr) {
                    case U'\'':
                    case U'\"':
                    case U'\?':
                    case U'\\':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
                        j++;
                        break;

                    case U'a':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\a'));
                        j++;
                        break;

                    case U'b':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\b'));
                        j++;
                        break;

                    case U'f':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\f'));
                        j++;
                        break;

                    case U'n':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\n'));
                        j++;
                        break;

                    case U'r':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\r'));
                        j++;
                        break;

                    case U't':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\t'));
                        j++;
                        break;

                    case U'v':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\v'));
                        j++;
                        break;

                    case U'e':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\x1B'));
                        j++;
                        break;

                    case U'x': {
                        REQUIRE(j + 1 < length && kefir_ishexdigit32(content[j + 1]),
                                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                       "Expected hexadecimal escape sequence"));
                        kefir_int64_t literal = kefir_hex32todec(content[++j]);
                        j++;
                        while (j < length && kefir_ishexdigit32(content[j])) {
                            literal <<= 4;
                            literal |= kefir_hex32todec(content[j++]);
                        }
                        REQUIRE_OK(kefir_string_buffer_append_literal(mem, strbuf, literal));
                    } break;

                    case U'u':
                        REQUIRE(j + 4 < length && kefir_ishexdigit32(content[j + 1]) &&
                                    kefir_ishexdigit32(content[j + 2]) && kefir_ishexdigit32(content[j + 3]) &&
                                    kefir_ishexdigit32(content[j + 4]),
                                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                       "Expected universal character"));
                        chr = (kefir_hex32todec(content[j + 1]) << 12) | (kefir_hex32todec(content[j + 2]) << 8) |
                              (kefir_hex32todec(content[j + 3]) << 4) | kefir_hex32todec(content[j + 4]);
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
                        j += 5;
                        break;

                    case U'U':
                        REQUIRE(j + 8 < length && kefir_ishexdigit32(content[j + 1]) &&
                                    kefir_ishexdigit32(content[j + 2]) && kefir_ishexdigit32(content[j + 3]) &&
                                    kefir_ishexdigit32(content[j + 4]) && kefir_ishexdigit32(content[j + 5]) &&
                                    kefir_ishexdigit32(content[j + 6]) && kefir_ishexdigit32(content[j + 7]) &&
                                    kefir_ishexdigit32(content[j + 8]),
                                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                       "Expected universal character"));
                        chr = (kefir_hex32todec(content[j + 1]) << 28) | (kefir_hex32todec(content[j + 2]) << 24) |
                              (kefir_hex32todec(content[j + 3]) << 20) | (kefir_hex32todec(content[j + 4]) << 16) |
                              (kefir_hex32todec(content[j + 5]) << 12) | (kefir_hex32todec(content[j + 6]) << 8) |
                              (kefir_hex32todec(content[j + 7]) << 4) | kefir_hex32todec(content[j + 8]);
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
                        j += 9;
                        break;

                    default: {
                        REQUIRE(kefir_isoctdigit32(chr),
                                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                       "Unexpected escape sequence"));
                        kefir_int64_t literal = chr - U'0';
                        j++;
                        if (j < length && kefir_isoctdigit32(content[j])) {
                            literal <<= 3;
                            literal += content[j++] - U'0';
                            if (j < length && kefir_isoctdigit32(content[j])) {
                                literal <<= 3;
                                literal |= content[j++] - U'0';
                            }
                        }
                        REQUIRE_OK(kefir_string_buffer_append_literal(mem, strbuf, literal));
                    } break;
                }
            } else {
                REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
                j++;
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_merge_raw_string_literals(struct kefir_mem *mem, const struct kefir_list *literals,
                                                     struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(literals != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token list"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));

    REQUIRE(kefir_list_length(literals) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-empty token list containing raw string literals"));
    kefir_string_literal_token_type_t type = KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE;
    const struct kefir_source_location *source_location = NULL;
    for (const struct kefir_list_entry *iter = kefir_list_head(literals); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_token *, token, iter->value);
        REQUIRE(
            token->klass == KEFIR_TOKEN_STRING_LITERAL && token->string_literal.raw_literal,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-empty token buffer containing raw string literals"));

        if (source_location == NULL) {
            source_location = &token->source_location;
        }

        REQUIRE(kefir_token_string_literal_type_concat(type, token->string_literal.type, &type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                       "Unable to concatenate differently-prefixed string literals"));
    }

    kefir_string_buffer_mode_t bufmode = KEFIR_STRING_BUFFER_MULTIBYTE;
    switch (type) {
        case KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE:
            bufmode = KEFIR_STRING_BUFFER_MULTIBYTE;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE8:
            bufmode = KEFIR_STRING_BUFFER_UNICODE8;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE16:
            bufmode = KEFIR_STRING_BUFFER_UNICODE16;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE32:
            bufmode = KEFIR_STRING_BUFFER_UNICODE32;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_WIDE:
            bufmode = KEFIR_STRING_BUFFER_WIDE;
            break;
    }

    struct kefir_string_buffer strbuf;
    REQUIRE_OK(kefir_string_buffer_init(mem, &strbuf, bufmode));

    kefir_result_t res = merge_literals(mem, literals, &strbuf);
    kefir_size_t length;
    const void *value = kefir_string_buffer_value(&strbuf, &length);
    switch (type) {
        case KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE:
            REQUIRE_CHAIN(&res, kefir_token_new_string_literal_multibyte(mem, value, length, token));
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE8:
            REQUIRE_CHAIN(&res, kefir_token_new_string_literal_unicode8(mem, value, length, token));
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE16:
            REQUIRE_CHAIN(&res, kefir_token_new_string_literal_unicode16(mem, value, length, token));
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE32:
            REQUIRE_CHAIN(&res, kefir_token_new_string_literal_unicode32(mem, value, length, token));
            break;

        case KEFIR_STRING_LITERAL_TOKEN_WIDE:
            REQUIRE_CHAIN(&res, kefir_token_new_string_literal_wide(mem, value, length, token));
            break;
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_buffer_free(mem, &strbuf);
        return res;
    });

    res = kefir_string_buffer_free(mem, &strbuf);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, token);
        return res;
    });

    token->source_location = *source_location;
    return KEFIR_OK;
}
