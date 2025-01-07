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

struct Struct1 {
    unsigned int field0 : 1;
    unsigned int field1 : 1;
    unsigned int field2 : 1;
    unsigned int field3 : 1;
    unsigned int field4 : 3;
    unsigned int field5 : 1;
    unsigned int field6 : 2;
    unsigned int field7 : 1;
    unsigned int field8 : 4;
    unsigned int field9 : 2;
    unsigned int field10 : 1;
    unsigned int field11 : 1;
    unsigned int field12 : 1;
    unsigned int field13 : 1;
    unsigned int field14 : 1;
    unsigned int field15 : 1;
    unsigned int field16 : 2;
    unsigned int field17 : 1;
    unsigned int field18 : 1;
    unsigned int field19 : 3;
    unsigned int field20 : 1;
    unsigned int field21 : 1;
    unsigned int field22 : 1;
    unsigned int field23 : 2;
    unsigned int field24 : 2;
    unsigned int field25 : 2;
    unsigned int field26 : 2;
    unsigned int field27 : 3;
    unsigned int field28 : 1;
    unsigned int field29 : 3;
    unsigned int field30 : 1;
    unsigned int field31 : 3;
    unsigned int field32 : 1;
    unsigned int field33 : 1;
    unsigned int field34 : 3;
    unsigned int field35 : 1;
    unsigned int field36 : 2;
    unsigned int field37 : 2;
    unsigned int field38 : 1;
    unsigned int field39 : 1;
    unsigned int field40 : 1;
};

#define STRUCT1_INIT                                                                                                   \
    {                                                                                                                  \
        1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
            0, 1, 0, 1                                                                                                 \
    }

extern struct Struct1 struct1;

unsigned int getfield40(void);

#endif
