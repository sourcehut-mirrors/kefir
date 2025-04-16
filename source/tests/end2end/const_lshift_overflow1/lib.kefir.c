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

signed char i8[6] = {1 << 7, 0 << 7, 1 << 8, 0 << 8, 2 << 7, 3 << 7};

signed short i16[6] = {1 << 15, 0 << 15, 1 << 16, 0 << 16, 2 << 15, 3 << 15};

signed int i32[6] = {1 << 31, 0 << 31, 1 << 32, 0 << 32, 2 << 31, 3 << 31};

signed long i64[6] = {1ll << 63, 0ll << 63, 1ll << 64, 0ll << 64, 2ll << 63, 3ll << 63};

unsigned char u8[6] = {1u << 7, 0u << 7, 1u << 8, 0u << 8, 2u << 7, 3u << 7};

unsigned short u16[6] = {1u << 15, 0u << 15, 1u << 16, 0u << 16, 2u << 15, 3u << 15};

unsigned int u32[6] = {1u << 31, 0u << 31, 1u << 32, 0u << 32, 2u << 31, 3u << 31};

unsigned long u64[6] = {1ull << 63, 0ull << 63, 1ull << 64, 0ull << 64, 2ull << 63, 3ull << 63};
