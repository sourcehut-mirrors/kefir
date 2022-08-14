/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "./definitions.h"

unsigned int Sizes[] = {sizeof(__SIZE_TYPE__),         sizeof(__PTRDIFF_TYPE__),      sizeof(__WCHAR_TYPE__),
                        sizeof(__WINT_TYPE__),         sizeof(__INTMAX_TYPE__),       sizeof(__UINTMAX_TYPE__),
                        sizeof(__INT8_TYPE__),         sizeof(__INT16_TYPE__),        sizeof(__INT32_TYPE__),
                        sizeof(__INT64_TYPE__),        sizeof(__UINT8_TYPE__),        sizeof(__UINT16_TYPE__),
                        sizeof(__UINT32_TYPE__),       sizeof(__UINT64_TYPE__),       sizeof(__INT_LEAST8_TYPE__),
                        sizeof(__INT_LEAST16_TYPE__),  sizeof(__INT_LEAST32_TYPE__),  sizeof(__INT_LEAST64_TYPE__),
                        sizeof(__UINT_LEAST8_TYPE__),  sizeof(__UINT_LEAST16_TYPE__), sizeof(__UINT_LEAST32_TYPE__),
                        sizeof(__UINT_LEAST64_TYPE__), sizeof(__INT_FAST8_TYPE__),    sizeof(__INT_FAST16_TYPE__),
                        sizeof(__INT_FAST32_TYPE__),   sizeof(__INT_FAST64_TYPE__),   sizeof(__UINT_FAST8_TYPE__),
                        sizeof(__UINT_FAST16_TYPE__),  sizeof(__UINT_FAST32_TYPE__),  sizeof(__UINT_FAST64_TYPE__),
                        sizeof(__INTPTR_TYPE__),       sizeof(__UINTPTR_TYPE__)};
