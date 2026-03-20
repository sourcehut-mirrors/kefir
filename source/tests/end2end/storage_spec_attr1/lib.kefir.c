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

#include "./definitions.h"

_Static_assert(sizeof(s1t) == 1);
_Static_assert(_Alignof(s1t) == 16);
_Static_assert(sizeof(s2t) == 1);
_Static_assert(_Alignof(s2t) == 16);
_Static_assert(sizeof(s3t) == 16);
_Static_assert(_Alignof(s3t) == 16);
_Static_assert(sizeof(s4t) == 16);
_Static_assert(_Alignof(s4t) == 16);
_Static_assert(sizeof(s5t) == 1);
_Static_assert(_Alignof(s5t) == 16);

int arr[] = {sizeof(s1t),   _Alignof(s1t), sizeof(s2t),   _Alignof(s2t), sizeof(s3t),
             _Alignof(s3t), sizeof(s4t),   _Alignof(s4t), sizeof(s5t),   _Alignof(s5t)};
