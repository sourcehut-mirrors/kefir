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

#define DEF_STRUCT(_name) \
    struct {              \
        char a;           \
        long b;           \
    } _name = {1, 2}

DEF_STRUCT(a);

void fn() {
    volatile int x = 100;
    {
        fn();
#pragma pack(1)
    }
}
DEF_STRUCT(b);

#pragma pack()
DEF_STRUCT(c);

int xyz[] = {
#pragma pack(push, 1)
    1, 2,
#pragma pack(push, 1)
    3};
DEF_STRUCT(d);

#pragma pack(pop)
DEF_STRUCT(e);

void fn2() {
    {
        {
            { fn2(); }
            fn2();
            fn2();
#pragma pack(pop)
        }
    }
}
DEF_STRUCT(f);

#pragma pack(pop)
DEF_STRUCT(g);
