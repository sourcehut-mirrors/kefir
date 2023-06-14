/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#define IMPL_NEG_DISCRIMINANT(_type) \
    _type neg_discriminant_##_type(_type a, _type b, _type c) { return NEG_DISCRIMINANT(a, b, c); }

IMPL_NEG_DISCRIMINANT(char)
IMPL_NEG_DISCRIMINANT(uchar)
IMPL_NEG_DISCRIMINANT(short)
IMPL_NEG_DISCRIMINANT(ushort)
IMPL_NEG_DISCRIMINANT(int)
IMPL_NEG_DISCRIMINANT(uint)
IMPL_NEG_DISCRIMINANT(long)
IMPL_NEG_DISCRIMINANT(ulong)
IMPL_NEG_DISCRIMINANT(llong)
IMPL_NEG_DISCRIMINANT(ullong)