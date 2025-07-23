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

#define DEF_ATTR(_attr, ...) \
    void fn##_attr(void)[[gnu::_attr __VA_ARGS__]] {}

DEF_ATTR(weak)
DEF_ATTR(noipa)
DEF_ATTR(packed)
DEF_ATTR(__weak__)
DEF_ATTR(noinline)
DEF_ATTR(__noipa__)
DEF_ATTR(__packed__)
DEF_ATTR(destructor)
DEF_ATTR(gnu_inline)
DEF_ATTR(visibility, ("hidden"))
DEF_ATTR(constructor)
DEF_ATTR(__returns_twice__)
DEF_ATTR(__destructor__)
DEF_ATTR(__gnu_inline__)
DEF_ATTR(__visibility__, ("hidden"))
DEF_ATTR(returns_twice)
DEF_ATTR(__noinline__)
DEF_ATTR(always_inline)
DEF_ATTR(__constructor__)
DEF_ATTR(__always_inline__)

#undef DEF_ATTR

#define DEF_ATTR(_attr, ...) extern int var##_attr[[gnu::_attr __VA_ARGS__]];

DEF_ATTR(aligned)
DEF_ATTR(__alias__, ("TEST"))
DEF_ATTR(__aligned__)
DEF_ATTR(alias, ("test"))
