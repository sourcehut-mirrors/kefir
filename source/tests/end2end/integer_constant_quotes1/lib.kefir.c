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

long arr[] = {1'0'0'0,
              0x1'0'0'0,
              0b1'0'0'0,
              01'0'0'0,
              123'456'789,
              0xabcd'1234'9876ull,
              0b1111'0000'1111'0000ll,
              0777'111'555'3333l,
              123'987'123'654'123'0wb,
              0x123'987'123'654'123'0uwb};

_BitInt(180) a = 1'9'322224484'131'314'25'4'1484'1'1'42'45'2'5'6'20184'230942'4828wb;
_BitInt(180) x = 0xabcd'1234'efdc'0987'1234'4321'd'e'f'a'000001'1wb;
_BitInt(180) y =
    0b1111'0000'1111'0000'0'0'0'0'111'1'000'1'1'0'1'0'1'11111'0'1'000001'01'111111111'0'1'1'11111111111'0wb;
_BitInt(180) z = 0'1'7'7'7'0000000'12345'432401'1737151'1271761'20717147'221'0wb;