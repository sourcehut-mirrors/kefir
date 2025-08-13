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

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

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

        struct {
            uchar uchar1;
            uchar uchar2;
        };

        struct {
            ushort ushort1;
            ushort ushort2;
        };

        struct {
            uint uint1;
            uint uint2;
        };

        struct {
            ulong ulong1;
            ulong ulong2;
        };
    };
};

#define DECL_OP(_name, _type) struct S1 _name##_type(_type, _type)
#define DECL_OPS(_name)    \
    DECL_OP(_name, char);  \
    DECL_OP(_name, short); \
    DECL_OP(_name, int);   \
    DECL_OP(_name, long)
#define DECL_UOPS(_name)    \
    DECL_OP(_name, uchar);  \
    DECL_OP(_name, ushort); \
    DECL_OP(_name, uint);   \
    DECL_OP(_name, ulong)

DECL_OPS(add);
DECL_OPS(sub);
DECL_OPS(mul);
DECL_OPS(div);
DECL_OPS(mod);
DECL_OPS(and);
DECL_OPS(or);
DECL_OPS(xor);
DECL_OPS(shl);
DECL_OPS(sar);
DECL_OPS(band);
DECL_OPS(bor);

DECL_UOPS(umul);
DECL_UOPS(udiv);
DECL_UOPS(umod);
DECL_UOPS(shr);

#undef DECL_UOPS
#undef DECL_OPS
#undef DECL_OP

#endif
