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

#line __LINE__ "decimal_const_fold3"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 int_to_dec32() {
    return -58427;
}

_Decimal32 uint_to_dec32() {
    return 842819u;
}

_Decimal32 float_to_dec32() {
    return 6.524f;
}

_Decimal32 double_to_dec32() {
    return -0.482913e5;
}

_Decimal32 long_double_to_dec32() {
    return 5284.45e4L;
}

_Decimal64 int_to_dec64() {
    return -58427;
}

_Decimal64 uint_to_dec64() {
    return 842819u;
}

_Decimal64 float_to_dec64() {
    return 6.524f;
}

_Decimal64 double_to_dec64() {
    return -0.482913e5;
}

_Decimal64 long_double_to_dec64() {
    return 5284.45e4L;
}

_Decimal128 int_to_dec128() {
    return -58427;
}

_Decimal128 uint_to_dec128() {
    return 842819u;
}

_Decimal128 float_to_dec128() {
    return 6.524f;
}

_Decimal128 double_to_dec128() {
    return -0.482913e5;
}

_Decimal128 long_double_to_dec128() {
    return 5284.45e4L;
}
#endif
