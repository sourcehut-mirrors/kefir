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

#line __LINE__ "decimal_builtins1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 d32_inf = __builtin_infd32();
_Decimal64 d64_inf = __builtin_infd32();
_Decimal128 d128_inf = __builtin_infd32();

_Decimal32 d32_nan = __builtin_nand32("");
_Decimal64 d64_nan = __builtin_nand32("");
_Decimal128 d128_nan = __builtin_nand32("");

_Decimal32 d32_snan = __builtin_nansd32("");
_Decimal64 d64_snan = __builtin_nansd64("");
_Decimal128 d128_snan = __builtin_nansd128("");
_Decimal64x d64x_snan = __builtin_nansd64x("");

_Decimal32 get_d32_inf(void) {
    return __builtin_infd32();
}

_Decimal64 get_d64_inf(void) {
    return __builtin_infd32();
}

_Decimal128 get_d128_inf(void) {
    return __builtin_infd32();
}

_Decimal32 get_d32_nan(void) {
    return __builtin_nand32("");
}

_Decimal64 get_d64_nan(void) {
    return __builtin_nand32("");
}

_Decimal128 get_d128_nan(void) {
    return __builtin_nand32("");
}

_Decimal32 get_d32_snan(void) {
    return __builtin_nansd32("");
}

_Decimal64 get_d64_snan(void) {
    return __builtin_nansd64("");
}

_Decimal128 get_d128_snan(void) {
    return __builtin_nansd128("");
}

_Decimal64x get_d64x_snan(void) {
    return __builtin_nansd64x("");
}
#endif
