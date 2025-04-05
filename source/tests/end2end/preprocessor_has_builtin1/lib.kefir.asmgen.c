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

#define PRINT_BUILTIN(_builtin) _builtin = __has_builtin(_builtin)

PRINT_BUILTIN(__builtin_va_start);
PRINT_BUILTIN(__builtin_va_end);
PRINT_BUILTIN(__builtin_va_arg);
PRINT_BUILTIN(__builtin_va_copy);
PRINT_BUILTIN(__builtin_alloca);
PRINT_BUILTIN(__builtin_alloca_with_align);
PRINT_BUILTIN(__builtin_alloca_with_align_and_max);
PRINT_BUILTIN(__builtin_offsetof);
PRINT_BUILTIN(__builtin_types_compatible_p);
PRINT_BUILTIN(__builtin_choose_expr);
PRINT_BUILTIN(__builtin_constant_p);
PRINT_BUILTIN(__builtin_classify_type);

PRINT_BUILTIN(TEST123)
PRINT_BUILTIN(xyz)