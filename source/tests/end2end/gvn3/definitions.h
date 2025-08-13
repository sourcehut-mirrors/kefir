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

struct S1 {
    union {
        struct {
            char char1;
            char char2;
        };

        struct {
            short short1;
            short short2;
        };

        struct {
            int int1;
            int int2;
        };

        struct {
            long long1;
            long long2;
        };
    };
};

#define DECL_OP(_name, _type) struct S1 _name##_type(_type)
#define DECL_OPS(_name)    \
    DECL_OP(_name, char);  \
    DECL_OP(_name, short); \
    DECL_OP(_name, int);   \
    DECL_OP(_name, long)

DECL_OPS(not);
DECL_OPS(neg);
DECL_OPS(bnot);
DECL_OPS(bool);
DECL_OPS(conv8);
DECL_OPS(conv16);
DECL_OPS(conv32);
DECL_OPS(uconv8);
DECL_OPS(uconv16);
DECL_OPS(uconv32);

#undef DECL_OPS
#undef DECL_OP

#endif
