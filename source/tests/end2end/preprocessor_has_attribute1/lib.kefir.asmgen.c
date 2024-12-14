/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#define PRINT_ATTR(_attr) _attr = __has_attribute(_attr)

PRINT_ATTR(aligned);
PRINT_ATTR(__aligned__);
PRINT_ATTR(gnu_inline);
PRINT_ATTR(__gnu_inline__);
PRINT_ATTR(returns_twice);
PRINT_ATTR(__returns_twice__);
PRINT_ATTR(weak);
PRINT_ATTR(__weak__);
PRINT_ATTR(alias);
PRINT_ATTR(__alias__);
PRINT_ATTR(visibility);
PRINT_ATTR(__visibility__);

PRINT_ATTR(TEST1);
PRINT_ATTR(ONETWOTHREE);