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

#define DECL(_op) long select_##_op(long, long, long, long)
DECL(equals);
DECL(not_equals);
DECL(greater);
DECL(greater_or_equals);
DECL(lesser);
DECL(lesser_or_equals);
#undef DECL

#define DECL(_op) long select_##_op(unsigned long, unsigned long, long, long)
DECL(above);
DECL(above_or_equals);
DECL(below);
DECL(below_or_equals);
#undef DECL

#endif
