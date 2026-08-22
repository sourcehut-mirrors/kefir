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

#ifndef KEFIR_LEXER_UTIL_H_
#define KEFIR_LEXER_UTIL_H_

#include "kefir/lexer/source_cursor.h"

kefir_bool_t kefir_lexer_char_isspace(kefir_lexer_char_t);
kefir_bool_t kefir_lexer_char_isdigit(kefir_lexer_char_t);
kefir_bool_t kefir_lexer_char_isoctdigit(kefir_lexer_char_t);
kefir_bool_t kefir_lexer_char_ishexdigit(kefir_lexer_char_t);
kefir_bool_t kefir_lexer_char_isnondigit(kefir_lexer_char_t);
kefir_uint32_t kefir_lexer_char_hex2dec(kefir_lexer_char_t);

#endif
