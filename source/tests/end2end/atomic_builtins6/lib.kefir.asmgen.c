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

#define UNWRAP(_x) _x
#define DUMP(_x) #_x = UNWRAP(_x)

DUMP(__ATOMIC_RELAXED)
DUMP(__ATOMIC_CONSUME)
DUMP(__ATOMIC_ACQUIRE)
DUMP(__ATOMIC_RELEASE)
DUMP(__ATOMIC_ACQ_REL)
DUMP(__ATOMIC_SEQ_CST)

DUMP(__GCC_ATOMIC_BOOL_LOCK_FREE)
DUMP(__GCC_ATOMIC_CHAR_LOCK_FREE)
DUMP(__GCC_ATOMIC_CHAR8_T_LOCK_FREE)
DUMP(__GCC_ATOMIC_CHAR16_T_LOCK_FREE)
DUMP(__GCC_ATOMIC_CHAR32_T_LOCK_FREE)
DUMP(__GCC_ATOMIC_WCHAR_T_LOCK_FREE)
DUMP(__GCC_ATOMIC_SHORT_LOCK_FREE)
DUMP(__GCC_ATOMIC_INT_LOCK_FREE)
DUMP(__GCC_ATOMIC_LONG_LOCK_FREE)
DUMP(__GCC_ATOMIC_LLONG_LOCK_FREE)
DUMP(__GCC_ATOMIC_POINTER_LOCK_FREE)

DUMP(__GCC_ATOMIC_TEST_AND_SET_TRUEVAL)