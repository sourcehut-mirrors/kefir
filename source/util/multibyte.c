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

#include <assert.h>
#include <errno.h>
#include "kefir/core/basic-types.h"
#include "kefir/core/util.h"
#include "kefir/util/uchar.h"
#ifdef KEFIR_OWN_UCHAR_IMPLEMENTATION

#define RESULT_ENCODING_ERROR ((size_t) -1)
#define RESULT_SURROGATE_WRITTEN ((size_t) -3)

#define MAX_CHAR_WIDTH 4

#define UTF16_SURROGATE_RANGE_START 0x10000
#define UTF16_SURROGATE_BITSHIFT 10
#define UTF16_SURROGATE_MASK 0x3ffu
#define UTF16_SURROGATE_HIGH_BEGIN 0xd800u
#define UTF16_SURROGATE_HIGH_END 0xdc00u
#define UTF16_SURROGATE_LOW_BEGIN 0xdc00u
#define UTF16_SURROGATE_LOW_END 0xe000u

size_t c32rtomb(char *restrict mbstr, char32_t chr32, mbstate_t *restrict state) {
    return wcrtomb(mbstr, chr32, state);
}

size_t mbrtoc32(char32_t *restrict chr32str, const char *restrict mbstr, size_t mblength, mbstate_t *restrict state) {
    assert(state != NULL);
    assert(chr32str != NULL);
    assert(mbstr != NULL);

    wchar_t wide_char;
    size_t result = mbrtowc(&wide_char, mbstr, mblength, state);
    if (result <= MAX_CHAR_WIDTH) {
        *chr32str = (char32_t) wide_char;
    }
    return result;
}

size_t c16rtomb(char *restrict mbstr, char16_t chr16, mbstate_t *restrict state) {
    assert(state != NULL);
    assert(mbstr != NULL);

    _Static_assert(sizeof(mbstate_t) >= sizeof(uint32_t), "mbstate_t size expectation mismatch");
    _Static_assert(_Alignof(mbstate_t) >= _Alignof(uint32_t), "mbstate_t alignment expectation mismatch");
    ASSIGN_DECL_CAST(uint32_t *, state_word, state);

    if (*state_word == 0 && chr16 >= UTF16_SURROGATE_HIGH_BEGIN && chr16 < UTF16_SURROGATE_HIGH_END) {
        *state_word = ((chr16 & UTF16_SURROGATE_MASK) << UTF16_SURROGATE_BITSHIFT);
        return 0;
    }

    wchar_t wide_char;
    if (*state_word != 0) {
        if (chr16 < UTF16_SURROGATE_LOW_BEGIN || chr16 >= UTF16_SURROGATE_LOW_END) {
            goto on_illegal_sequence;
        } else {
            wide_char = UTF16_SURROGATE_RANGE_START + *state_word + (chr16 & UTF16_SURROGATE_MASK);
        }
        *state_word = 0;
    } else {
        wide_char = (wchar_t) chr16;
    }
    return wcrtomb(mbstr, wide_char, NULL);

on_illegal_sequence:
    *state_word = 0;
    errno = EILSEQ;
    return RESULT_ENCODING_ERROR;
}

size_t mbrtoc16(char16_t *restrict chr16, const char *restrict mbstr, size_t mblength, mbstate_t *restrict state) {
    assert(state != NULL);
    assert(chr16 != NULL);
    assert(mbstr != NULL);

    _Static_assert(sizeof(mbstate_t) >= sizeof(uint32_t), "mbstate_t size expectation mismatch");
    _Static_assert(_Alignof(mbstate_t) >= _Alignof(uint32_t), "mbstate_t alignment expectation mismatch");
    ASSIGN_DECL_CAST(uint32_t *, state_word, state);

    if (*state_word != 0) {
        *chr16 = *state_word;
        *state_word = 0;
        return RESULT_SURROGATE_WRITTEN;
    }

    wchar_t wide_char;
    size_t result = mbrtowc(&wide_char, mbstr, mblength, state);
    if (result <= MAX_CHAR_WIDTH) {
        if (wide_char >= UTF16_SURROGATE_RANGE_START) {
            uint32_t codepoint = (uint32_t) wide_char;
            codepoint -= UTF16_SURROGATE_RANGE_START;
            *state_word = UTF16_SURROGATE_LOW_BEGIN + (codepoint & UTF16_SURROGATE_MASK);
            wide_char = UTF16_SURROGATE_HIGH_BEGIN + (codepoint >> UTF16_SURROGATE_BITSHIFT);
        }

        *chr16 = (char16_t) wide_char;
    }
    return result;
}
#endif
