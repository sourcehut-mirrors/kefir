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

#define kefir_ast_new_constant_bool_noarena(_mem, _value) (kefir_ast_new_constant_bool((_mem), NULL, (_value)))
#define kefir_ast_new_constant_char_noarena(_mem, _value) (kefir_ast_new_constant_char((_mem), NULL, (_value)))
#define kefir_ast_new_constant_wide_char_noarena(_mem, _value) (kefir_ast_new_constant_wide_char((_mem), NULL, (_value)))
#define kefir_ast_new_constant_unicode16_char_noarena(_mem, _value) (kefir_ast_new_constant_unicode16_char((_mem), NULL, (_value)))
#define kefir_ast_new_constant_unicode32_char_noarena(_mem, _value) (kefir_ast_new_constant_unicode32_char((_mem), NULL, (_value)))
#define kefir_ast_new_constant_int_noarena(_mem, _value) (kefir_ast_new_constant_int((_mem), NULL, (_value)))
#define kefir_ast_new_constant_uint_noarena(_mem, _value) (kefir_ast_new_constant_uint((_mem), NULL, (_value)))
#define kefir_ast_new_constant_long_noarena(_mem, _value) (kefir_ast_new_constant_long((_mem), NULL, (_value)))
#define kefir_ast_new_constant_ulong_noarena(_mem, _value) (kefir_ast_new_constant_ulong((_mem), NULL, (_value)))
#define kefir_ast_new_constant_long_long_noarena(_mem, _value) (kefir_ast_new_constant_long_long((_mem), NULL, (_value)))
#define kefir_ast_new_constant_ulong_long_noarena(_mem, _value) (kefir_ast_new_constant_ulong_long((_mem), NULL, (_value)))
#define kefir_ast_new_constant_float_noarena(_mem, _value) (kefir_ast_new_constant_float((_mem), NULL, (_value)))
#define kefir_ast_new_constant_double_noarena(_mem, _value) (kefir_ast_new_constant_double((_mem), NULL, (_value)))
#define kefir_ast_new_constant_long_double_noarena(_mem, _value) (kefir_ast_new_constant_long_double((_mem), NULL, (_value)))
