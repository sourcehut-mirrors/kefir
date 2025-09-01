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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define STR0_CONTENT     \
    "\xff\xff\xee\u2EF0" \
    "Привет, wōrl\000"   \
    "d\U00000f0f"
#define STR1_CONTENT     \
    "\xff\xff\xee\u2EF0" \
    u8"Привет, wōrl\000" \
    "d\U00000f0f"
#define STR2_CONTENT      \
    "\xff\xfff\xee\u2EF0" \
    u"Привет, wōrl\000"   \
    "d\U00000f0f"
#define STR3_CONTENT      \
    "\xff\xfff\xee\u2EF0" \
    "Привет, wōrl\000"    \
    U"d\U00000f0f"
#define STR4_CONTENT      \
    "\xff\xfff\xee\u2EF0" \
    "Привет, wōrl\000"    \
    L"d\U00000f0f"

extern const char STR0[];
extern const char STR1[];
extern const unsigned short STR2[];
extern const unsigned int STR3[];
extern const int STR4[];

#endif
