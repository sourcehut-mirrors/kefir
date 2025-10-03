/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#line __LINE__ "decimal_inline_asm1"

#if defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__x86_64__)
int test() {
    int res;
    asm("mov %1, %0\n"
        : "=r"(res)
        : "i"(4.21df)
        :);
    return res;
}

_Decimal32 test2(_Decimal32 x) {
    _Decimal32 res;
    asm("mov %1, %0"
            : "=r"(res)
            : "r"(x));
    return res;
}

_Decimal32 test3(_Decimal32 x) {
    _Decimal32 res;
    asm("movdqu %1, %0"
            : "=x"(res)
            : "x"(x));
    return res;
}

_Decimal32 test4(_Decimal32 x) {
    _Decimal32 res;
    asm("movd %1, %0"
            : "=m"(res)
            : "x"(x));
    return res;
}

_Decimal64 test5(_Decimal64 x) {
    _Decimal64 res;
    asm("mov %1, %0"
            : "=r"(res)
            : "r"(x));
    return res;
}

_Decimal64 test6(_Decimal64 x) {
    _Decimal64 res;
    asm("movdqu %1, %0"
            : "=x"(res)
            : "x"(x));
    return res;
}

_Decimal64 test7(_Decimal64 x) {
    _Decimal64 res;
    asm("movq %1, %0"
            : "=m"(res)
            : "x"(x));
    return res;
}

_Decimal128 test8(_Decimal128 x) {
    _Decimal128 res;
    asm("movdqu %1, %0"
            : "=x"(res)
            : "x"(x));
    return res;
}

_Decimal128 test9(_Decimal128 x) {
    _Decimal128 res;
    asm("movdqu %1, %0"
            : "=m"(res)
            : "x"(x));
    return res;
}
#endif
