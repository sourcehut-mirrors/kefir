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

_Float32 a = 3.14159f32;
_Float32x b = 2.71828f32x;
_Static_assert(_Generic(3.14159f32, _Float32: 1, default: 0));
#ifndef NO_LONG_DOUBLE
_Static_assert(_Generic(2.71828f32x, _Float32x: 1, default: 0));
#endif

_Float64 c = 3.14159f64;
#ifndef NO_LONG_DOUBLE
_Float64x d = 2.71828f64x;
#endif
_Static_assert(_Generic(3.14159f64, _Float64: 1, default: 0));
#ifndef NO_LONG_DOUBLE
_Static_assert(_Generic(2.71828f64x, _Float64x: 1, default: 0));
#endif

#ifndef NO_LONG_DOUBLE
_Float80 e = 3.14159f80;
_Static_assert(_Generic(3.14159f80, _Float80: 1, default: 0));
#endif

_Float32 mygeta(void) {
    return 3.14159f32;
}

_Float32x mygetb(void) {
    return 2.71828f32x;
}

_Float64 mygetc(void) {
    return 3.14159f64;
}

#ifndef NO_LONG_DOUBLE
_Float64x mygetd(void) {
    return 2.71828f64x;
}

_Float80 mygete(void) {
    return 3.14159f80;
}
#endif
