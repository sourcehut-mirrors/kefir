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

#define DECL_OP(_name, _type) _type _name(_Bool, _type, _type)
#define DECL_OPS(_name)           \
    DECL_OP(_name, unsigned int); \
    DECL_OP(u##_name, unsigned int)

DECL_OPS(add);
DECL_OPS(sub);
DECL_OPS(mul);
DECL_OPS(and);
DECL_OPS(or);
DECL_OPS(xor);
DECL_OPS(shl);
DECL_OPS(shr);

#undef DECL_OPS
#undef DECL_OP

unsigned int test(_Bool, unsigned int, unsigned int);
int test2(_Bool, unsigned int);

#endif
