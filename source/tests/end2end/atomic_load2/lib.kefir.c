/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#define DEF(_id, _type)                     \
    _type load_##_id(_Atomic(_type) *ptr) { \
        return *ptr;                        \
    }

DEF(b, _Bool)
DEF(u8, unsigned char)
DEF(u16, unsigned short)
DEF(u32, unsigned int)
DEF(u64, unsigned long)
DEF(f32, float)
DEF(f64, double)
