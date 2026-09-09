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

#include "kefir/lexer/lexer.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/core/util.h"
#include "kefir/core/string_buffer.h"
#include "kefir/lexer/util.h"
#include "kefir/util/uchar.h"

static kefir_result_t merge_literals(struct kefir_mem *mem, const struct kefir_list *literals,
                                     struct kefir_string_buffer *strbuf) {
    for (const struct kefir_list_entry *iter = kefir_list_head(literals); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_token *, token, iter->value);

        const char *content = (char *) token->string_literal->literal;
        kefir_size_t length = token->string_literal->length;
        REQUIRE(length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected non-empty raw string literal"));
        length--;  // Drop trailing null character
        for (kefir_size_t j = 0; j < length;) {
            char chr = content[j];
            if (chr == '\\') {
                REQUIRE(j + 1 < length, KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                               "Expected escape sequence after '\\'"));
                chr = content[++j];
                switch (chr) {
                    case '\'':
                    case '\"':
                    case '\?':
                    case '\\':
                        REQUIRE_OK(kefir_string_buffer_append_literal(mem, strbuf, chr));
                        j++;
                        break;

                    case 'a':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\a'));
                        j++;
                        break;

                    case 'b':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\b'));
                        j++;
                        break;

                    case 'f':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\f'));
                        j++;
                        break;

                    case 'n':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\n'));
                        j++;
                        break;

                    case 'r':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\r'));
                        j++;
                        break;

                    case 't':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\t'));
                        j++;
                        break;

                    case 'v':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\v'));
                        j++;
                        break;

                    case 'e':
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, U'\x1B'));
                        j++;
                        break;

                    case 'x': {
                        REQUIRE(j + 1 < length && kefir_lexer_char_ishexdigit(content[j + 1]),
                                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                       "Expected hexadecimal escape sequence"));
                        kefir_int64_t literal = kefir_lexer_char_hex2dec(content[++j]);
                        j++;
                        while (j < length && kefir_lexer_char_ishexdigit(content[j])) {
                            literal <<= 4;
                            literal |= kefir_lexer_char_hex2dec(content[j++]);
                        }
                        REQUIRE_OK(kefir_string_buffer_append_literal(mem, strbuf, literal));
                    } break;

                    case 'u': {
                        REQUIRE(j + 4 < length && kefir_lexer_char_ishexdigit(content[j + 1]) &&
                                    kefir_lexer_char_ishexdigit(content[j + 2]) && kefir_lexer_char_ishexdigit(content[j + 3]) &&
                                    kefir_lexer_char_ishexdigit(content[j + 4]),
                                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                       "Expected universal character"));
                        kefir_char32_t chr = (kefir_lexer_char_hex2dec(content[j + 1]) << 12) | (kefir_lexer_char_hex2dec(content[j + 2]) << 8) |
                              (kefir_lexer_char_hex2dec(content[j + 3]) << 4) | kefir_lexer_char_hex2dec(content[j + 4]);
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
                        j += 5;
                    } break;

                    case 'U': {
                        REQUIRE(j + 8 < length && kefir_lexer_char_ishexdigit(content[j + 1]) &&
                                    kefir_lexer_char_ishexdigit(content[j + 2]) && kefir_lexer_char_ishexdigit(content[j + 3]) &&
                                    kefir_lexer_char_ishexdigit(content[j + 4]) && kefir_lexer_char_ishexdigit(content[j + 5]) &&
                                    kefir_lexer_char_ishexdigit(content[j + 6]) && kefir_lexer_char_ishexdigit(content[j + 7]) &&
                                    kefir_lexer_char_ishexdigit(content[j + 8]),
                                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                       "Expected universal character"));
                        kefir_char32_t chr = (kefir_lexer_char_hex2dec(content[j + 1]) << 28) | (kefir_lexer_char_hex2dec(content[j + 2]) << 24) |
                              (kefir_lexer_char_hex2dec(content[j + 3]) << 20) | (kefir_lexer_char_hex2dec(content[j + 4]) << 16) |
                              (kefir_lexer_char_hex2dec(content[j + 5]) << 12) | (kefir_lexer_char_hex2dec(content[j + 6]) << 8) |
                              (kefir_lexer_char_hex2dec(content[j + 7]) << 4) | kefir_lexer_char_hex2dec(content[j + 8]);
                        REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
                        j += 9;
                    } break;

                    default: {
                        REQUIRE(kefir_lexer_char_isoctdigit(chr),
                                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &token->source_location,
                                                       "Unexpected escape sequence"));
                        kefir_int64_t literal = chr - '0';
                        j++;
                        if (j < length && kefir_lexer_char_isoctdigit(content[j])) {
                            literal <<= 3;
                            literal += content[j++] - '0';
                            if (j < length && kefir_lexer_char_isoctdigit(content[j])) {
                                literal <<= 3;
                                literal |= content[j++] - '0';
                            }
                        }
                        REQUIRE_OK(kefir_string_buffer_append_literal(mem, strbuf, literal));
                    } break;
                }
            } else {
                mbstate_t mbstate = {0};
                char32_t c32;
                kefir_size_t index = j;
                kefir_bool_t terminate = false;
                for (; !terminate && index < length;) {
                    size_t rc = mbrtoc32(&c32, content + index, length - index, &mbstate);
                    switch (rc) {
                        case (size_t) -3:
                            REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, c32));
                            break;

                        case (size_t) -2:
                            REQUIRE_OK(kefir_string_buffer_append_literal(mem, strbuf, content[index]));
                            index++;
                            terminate = true;
                            break;

                        case (size_t) -1:
                            terminate = true;
                            break;

                        case 0:
                            REQUIRE_OK(kefir_string_buffer_append_literal(mem, strbuf, '\0'));
                            index++;
                            terminate = true;
                            break;

                        default:
                            REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, c32));
                            index += rc;
                            terminate = true;
                            break;
                    }
                }

                if (index == j) {
                    REQUIRE_OK(kefir_string_buffer_append_literal(mem, strbuf, chr));
                    j++;
                } else {
                    j = index;
                }
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
            token->klass == KEFIR_TOKEN_STRING_LITERAL && token->string_literal->raw_literal,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-empty token buffer containing raw string literals"));

        if (source_location == NULL) {
            source_location = &token->source_location;
        }

        REQUIRE(kefir_token_string_literal_type_concat(type, token->string_literal->type, &type),
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
