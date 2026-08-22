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

#include "kefir/lexer/util.h"
#include <ctype.h>

kefir_bool_t kefir_lexer_char_isspace(kefir_lexer_char_t chr) {
    return chr >= 0 && chr <= KEFIR_UCHAR_MAX && isspace(chr);
}

kefir_bool_t kefir_lexer_char_isdigit(kefir_lexer_char_t chr) {
    return chr >= '0' && chr <= '9';
}

kefir_bool_t kefir_lexer_char_isoctdigit(kefir_lexer_char_t chr) {
    return chr >= '0' && chr <= '7';
}

kefir_bool_t kefir_lexer_char_ishexdigit(kefir_lexer_char_t chr) {
    return chr >= 0 && chr <= KEFIR_UCHAR_MAX && isxdigit(chr);
}

kefir_bool_t kefir_lexer_char_isnondigit(kefir_lexer_char_t chr) {
    static kefir_bool_t NONDIGIT[KEFIR_UCHAR_MAX + 1] = {
        ['_'] = true,
        ['a'] = true,
        ['b'] = true,
        ['c'] = true,
        ['d'] = true,
        ['e'] = true,
        ['f'] = true,
        ['g'] = true,
        ['h'] = true,
        ['i'] = true,
        ['j'] = true,
        ['k'] = true,
        ['l'] = true,
        ['m'] = true,
        ['n'] = true,
        ['o'] = true,
        ['p'] = true,
        ['q'] = true,
        ['r'] = true,
        ['s'] = true,
        ['t'] = true,
        ['u'] = true,
        ['v'] = true,
        ['w'] = true,
        ['x'] = true,
        ['y'] = true,
        ['z'] = true,
        ['A'] = true,
        ['B'] = true,
        ['C'] = true,
        ['D'] = true,
        ['E'] = true,
        ['F'] = true,
        ['G'] = true,
        ['H'] = true,
        ['I'] = true,
        ['J'] = true,
        ['K'] = true,
        ['L'] = true,
        ['M'] = true,
        ['N'] = true,
        ['O'] = true,
        ['P'] = true,
        ['Q'] = true,
        ['R'] = true,
        ['S'] = true,
        ['T'] = true,
        ['U'] = true,
        ['V'] = true,
        ['W'] = true,
        ['X'] = true,
        ['Y'] = true,
        ['Z'] = true,
        ['@'] = true,
        ['$'] = true
    };
    return chr >= 0 && chr <= KEFIR_UCHAR_MAX && NONDIGIT[chr];
}

kefir_uint32_t kefir_lexer_char_hex2dec(kefir_lexer_char_t chr) {
    static kefir_uint32_t HEX2DEC[KEFIR_UCHAR_MAX + 1] = {
        ['0'] = 0,
        ['1'] = 1,
        ['2'] = 2,
        ['3'] = 3,
        ['4'] = 4,
        ['5'] = 5,
        ['6'] = 6,
        ['7'] = 7,
        ['8'] = 8,
        ['9'] = 9,
        ['a'] = 10,
        ['A'] = 10,
        ['b'] = 11,
        ['B'] = 11,
        ['c'] = 12,
        ['C'] = 12,
        ['d'] = 13,
        ['D'] = 13,
        ['e'] = 14,
        ['E'] = 14,
        ['f'] = 15,
        ['F'] = 15
    };
    if (chr >= 0 && chr <= KEFIR_UCHAR_MAX) {
        return HEX2DEC[chr];
    } else {
        return 0;
    }
}
