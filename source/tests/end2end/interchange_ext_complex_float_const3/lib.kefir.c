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

_Complex _Float32 a = 0x3a.47p4f32i;
_Complex _Float32x b = 0x9fe.43bp1f32xi;
_Static_assert(_Generic(0x3a.47p4f32i, _Complex _Float32 : 1, default : 0));
#ifndef NO_LONG_DOUBLE
_Static_assert(_Generic(0x9fe.43bp1f32xi, _Complex _Float32x : 1, default : 0));
#endif

_Complex _Float64 c = 0x3a.47p4f64i;
#ifndef NO_LONG_DOUBLE
_Complex _Float64x d = 0x9fe.43bp1f64xi;
#endif
_Static_assert(_Generic(0x3a.47p4f64i, _Complex _Float64 : 1, default : 0));
#ifndef NO_LONG_DOUBLE
_Static_assert(_Generic(0x9fe.43bp1f64xi, _Complex _Float64x : 1, default : 0));
#endif

#ifndef NO_LONG_DOUBLE
_Complex _Float80 e = 0x3a.47p4f80i;
_Static_assert(_Generic(0x3a.47p4f80i, _Complex _Float80 : 1, default : 0));
#endif

_Complex _Float32 mygeta(void) {
    return 0x3a.47p4f32i;
}

_Complex _Float32x mygetb(void) {
    return 0x9fe.43bp1f32xi;
}

_Complex _Float64 mygetc(void) {
    return 0x3a.47p4f64i;
}

#ifndef NO_LONG_DOUBLE
_Complex _Float64x mygetd(void) {
    return 0x9fe.43bp1f64xi;
}

_Complex _Float80 mygete(void) {
    return 0x3a.47p4f80i;
}
#endif
