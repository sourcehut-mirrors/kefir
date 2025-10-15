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

_Bool b32[] = {
    (_Complex float) (0.0f),
    (_Complex float) (0.0f + 0.0if),
    (_Complex float) (0.0if),
    (_Complex float) (1.0f),
    (_Complex float) (1.0f + 0.0if),
    (_Complex float) (1.0if),
    (_Complex float) (1.0f + 1.0if)
};

_Bool b32_2[] = {
    (_Complex _Float32) (0.0f),
    (_Complex _Float32) (0.0f + 0.0if),
    (_Complex _Float32) (0.0if),
    (_Complex _Float32) (1.0f),
    (_Complex _Float32) (1.0f + 0.0if),
    (_Complex _Float32) (1.0if),
    (_Complex _Float32) (1.0f + 1.0if)
};

_Bool b64[] = {
    (_Complex double) (0.0f),
    (_Complex double) (0.0f + 0.0if),
    (_Complex double) (0.0if),
    (_Complex double) (1.0f),
    (_Complex double) (1.0f + 0.0if),
    (_Complex double) (1.0if),
    (_Complex double) (1.0f + 1.0if)
};

_Bool b64_2[] = {
    (_Complex _Float64) (0.0f),
    (_Complex _Float64) (0.0f + 0.0if),
    (_Complex _Float64) (0.0if),
    (_Complex _Float64) (1.0f),
    (_Complex _Float64) (1.0f + 0.0if),
    (_Complex _Float64) (1.0if),
    (_Complex _Float64) (1.0f + 1.0if)
};

_Bool b64_3[] = {
    (_Complex _Float32x) (0.0f),
    (_Complex _Float32x) (0.0f + 0.0if),
    (_Complex _Float32x) (0.0if),
    (_Complex _Float32x) (1.0f),
    (_Complex _Float32x) (1.0f + 0.0if),
    (_Complex _Float32x) (1.0if),
    (_Complex _Float32x) (1.0f + 1.0if)
};

_Bool b80[] = {
    (_Complex long double) (0.0f),
    (_Complex long double) (0.0f + 0.0if),
    (_Complex long double) (0.0if),
    (_Complex long double) (1.0f),
    (_Complex long double) (1.0f + 0.0if),
    (_Complex long double) (1.0if),
    (_Complex long double) (1.0f + 1.0if)
};

_Bool b80_2[] = {
    (_Complex _Float80) (0.0f),
    (_Complex _Float80) (0.0f + 0.0if),
    (_Complex _Float80) (0.0if),
    (_Complex _Float80) (1.0f),
    (_Complex _Float80) (1.0f + 0.0if),
    (_Complex _Float80) (1.0if),
    (_Complex _Float80) (1.0f + 1.0if)
};

_Bool b80_3[] = {
    (_Complex _Float64x) (0.0f),
    (_Complex _Float64x) (0.0f + 0.0if),
    (_Complex _Float64x) (0.0if),
    (_Complex _Float64x) (1.0f),
    (_Complex _Float64x) (1.0f + 0.0if),
    (_Complex _Float64x) (1.0if),
    (_Complex _Float64x) (1.0f + 1.0if)
};

_Bool c32_to_bool(_Complex float x) {
    return x;
}

_Bool c32_to_bool2(_Complex _Float32 x) {
    return x;
}

_Bool c64_to_bool(_Complex double x) {
    return x;
}

_Bool c64_to_bool2(_Complex _Float64 x) {
    return x;
}

_Bool c64_to_bool3(_Complex _Float32x x) {
    return x;
}

_Bool c80_to_bool(_Complex long double x) {
    return x;
}

_Bool c80_to_bool2(_Complex _Float80 x) {
    return x;
}

_Bool c80_to_bool3(_Complex _Float64x x) {
    return x;
}
