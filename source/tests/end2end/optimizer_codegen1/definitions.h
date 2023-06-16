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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;

#define OP_ADD(x, y) ((x) + (y))
#define OP_SUB(x, y) ((x) - (y))
#define OP_MUL(x, y) ((x) * (y))
#define OP_DIV(x, y) ((x) / (y))
#define OP_MOD(x, y) ((x) % (y))
#define OP_AND(x, y) ((x) & (y))
#define OP_OR(x, y) ((x) | (y))
#define OP_XOR(x, y) ((x) ^ (y))
#define OP_SHL(x, y) ((x) << (y))
#define OP_SHR(x, y) ((x) >> (y))
#define OP_NOT(x) (~(x))
#define OP_BNOT(x) (!(x))
#define OP_NEG(x) (-(x))

#define DECL_UN_FN(_id, _type) _type op_##_id##_##_type(_type)
#define DECL_BIN_FN(_id, _type) _type op_##_id##_##_type(_type, _type)

#define DECL_UN_FNS(_id)     \
    DECL_UN_FN(_id, char);   \
    DECL_UN_FN(_id, uchar);  \
    DECL_UN_FN(_id, short);  \
    DECL_UN_FN(_id, ushort); \
    DECL_UN_FN(_id, int);    \
    DECL_UN_FN(_id, uint);   \
    DECL_UN_FN(_id, long);   \
    DECL_UN_FN(_id, ulong);  \
    DECL_UN_FN(_id, llong);  \
    DECL_UN_FN(_id, ullong)

#define DECL_BIN_FNS(_id)     \
    DECL_BIN_FN(_id, char);   \
    DECL_BIN_FN(_id, uchar);  \
    DECL_BIN_FN(_id, short);  \
    DECL_BIN_FN(_id, ushort); \
    DECL_BIN_FN(_id, int);    \
    DECL_BIN_FN(_id, uint);   \
    DECL_BIN_FN(_id, long);   \
    DECL_BIN_FN(_id, ulong);  \
    DECL_BIN_FN(_id, llong);  \
    DECL_BIN_FN(_id, ullong)

DECL_BIN_FNS(add);
DECL_BIN_FNS(sub);
DECL_BIN_FNS(mul);
DECL_BIN_FNS(div);
DECL_BIN_FNS(mod);
DECL_BIN_FNS(and);
DECL_BIN_FNS(or);
DECL_BIN_FNS(xor);
DECL_BIN_FNS(shl);
DECL_BIN_FNS(shr);
DECL_UN_FNS(not );
DECL_UN_FNS(bnot);
DECL_UN_FNS(neg);

#endif
