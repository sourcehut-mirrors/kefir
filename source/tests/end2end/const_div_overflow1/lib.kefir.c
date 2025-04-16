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

#include "./definitions.h"

signed char i8[2] = {
    ((signed char) (1ll << 7)) / -1,
    ((signed char) (1ll << 7)) % -1,
};

signed short i16[2] = {
    ((signed short) (1ll << 15)) / -1,
    ((signed short) (1ll << 15)) % -1,
};

signed int i32[2] = {
    ((signed int) (1ll << 31)) / -1,
    ((signed int) (1ll << 31)) % -1,
};

signed long i64[2] = {
    ((signed long) (1ll << 63)) / -1,
    ((signed long) (1ll << 63)) % -1,
};

unsigned char u8[2] = {
    ((unsigned char) (1ll << 7)) / -1,
    ((unsigned char) (1ll << 7)) % -1,
};

unsigned short u16[2] = {
    ((unsigned short) (1ll << 15)) / -1,
    ((unsigned short) (1ll << 15)) % -1,
};

unsigned int u32[2] = {
    ((unsigned int) (1ll << 31)) / -1,
    ((unsigned int) (1ll << 31)) % -1,
};

unsigned long u64[2] = {
    ((unsigned long) (1ull << 63)) / -1,
    ((unsigned long) (1ull << 63)) % -1,
};
