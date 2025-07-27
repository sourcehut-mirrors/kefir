/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation
version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not
see <http://www.gnu.org/licenses/>.
*/

#define PRINT_ATTR(_attr) _attr = __has_c_attribute(_attr)

PRINT_ATTR(deprecated)
PRINT_ATTR(__deprecated__)
PRINT_ATTR(fallthrough)
PRINT_ATTR(__fallthrough__)
PRINT_ATTR(maybe_unused)
PRINT_ATTR(__maybe_unused__)
PRINT_ATTR(nodiscard)
PRINT_ATTR(__nodiscard__)
PRINT_ATTR(noreturn)
PRINT_ATTR(__noreturn__)
PRINT_ATTR(_Noreturn)
PRINT_ATTR(reproducible)
PRINT_ATTR(__reproducible__)
PRINT_ATTR(unsequenced)
PRINT_ATTR(__unsequenced__)

PRINT_ATTR(TEST1234);
PRINT_ATTR(XYZ1);
PRINT_ATTR(x::XYZ1);