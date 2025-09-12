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

#pragma STDC FENV_ACCESS OFF
#pragma STDC FP_CONTRACT ON
#pragma STDC CX_LIMITED_RANGE DEFAULT

int var1 = 1;
void test1(void) {
#pragma STDC FP_CONTRACT DEFAULT
}

#pragma STDC CX_LIMITED_RANGE OFF
#pragma STDC FENV_ROUND FE_TONEAREST

int var2 = 1;
void test2(void) {
#pragma STDC FENV_ACCESS DEFAULT
}

#pragma STDC FENV_DEC_ROUND FE_DEC_TOWARDZERO
#pragma STDC FP_CONTRACT OFF

int var3 = 1;
void test3(void) {}
