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

#if true
#error "Why true!"
#else
SOMEWHAT_TRUE
#endif

#define TR(x) tr##x
#if TR(ue)
#error "Why true!"
#else
SOMEWHAT_TRUE2
#endif

#if false
#error "False!"
#else
FALSE
#endif

#define FA(x) fa##x
#if FA(lse)
#error "False!"
#else
FALSE2
#endif
