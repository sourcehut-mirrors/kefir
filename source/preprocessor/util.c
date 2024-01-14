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

#include "kefir/preprocessor/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/preprocessor/format.h"

kefir_result_t kefir_token_new_string_literal_raw_from_escaped_multibyte(struct kefir_mem *mem,
                                                                         kefir_string_literal_token_type_t type,
                                                                         const char *content, kefir_size_t length,
                                                                         struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));

    struct kefir_string_buffer strbuf;
    REQUIRE_OK(kefir_string_buffer_init(mem, &strbuf, KEFIR_STRING_BUFFER_UNICODE32));
    kefir_result_t res = kefir_preprocessor_escape_string(mem, &strbuf, content, length);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_buffer_free(mem, &strbuf);
        return res;
    });

    kefir_size_t buflen;
    const kefir_char32_t *value = kefir_string_buffer_value(&strbuf, &buflen);
    res = kefir_token_new_string_literal_raw(mem, type, value, buflen, token);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_buffer_free(mem, &strbuf);
        return res;
    });

    res = kefir_string_buffer_free(mem, &strbuf);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, token);
        return res;
    });

    return KEFIR_OK;
}
