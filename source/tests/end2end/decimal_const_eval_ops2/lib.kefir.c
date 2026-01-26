/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#line __LINE__ "decimal_const_eval_ops2"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
int a[] = {!(3.14159df),
           !!(3.14159df),
           !(0.0df),
           !!(0.0df),
           3.14159df >= 2.7183df,
           3.14159df >= 20.7183df,
           -3.14159df >= 2.7183df,
           -3.14159df >= -3.14159df,
           3.14159df > 3.14159df,
           3.1416df > 3.14159df,
           3.1416df > 18313.0df,
           3.14159df <= 2.7183df,
           3.14159df <= 20.7183df,
           -3.14159df <= 2.7183df,
           -3.14159df <= -3.14159df,
           3.14159df < 3.14159df,
           3.1416df < 3.14159df,
           3.1416df < 18313.0df};

int b[] = {!(3.14159dd),
           !!(3.14159dd),
           !(0.0dd),
           !!(0.0dd),
           3.14159dd >= 2.7183dd,
           3.14159dd >= 20.7183dd,
           -3.14159dd >= 2.7183dd,
           -3.14159dd >= -3.14159dd,
           3.14159dd > 3.14159dd,
           3.1416dd > 3.14159dd,
           3.1416dd > 18313.0dd,
           3.14159dd <= 2.7183dd,
           3.14159dd <= 20.7183dd,
           -3.14159dd <= 2.7183dd,
           -3.14159dd <= -3.14159dd,
           3.14159dd < 3.14159dd,
           3.1416dd < 3.14159dd,
           3.1416dd < 18313.0dd};

int c[] = {!(3.14159dl),
           !!(3.14159dl),
           !(0.0dl),
           !!(0.0dl),
           3.14159dl >= 2.7183dl,
           3.14159dl >= 20.7183dl,
           -3.14159dl >= 2.7183dl,
           -3.14159dl >= -3.14159dl,
           3.14159dl > 3.14159dl,
           3.1416dl > 3.14159dl,
           3.1416dl > 18313.0dl,
           3.14159dl <= 2.7183dl,
           3.14159dl <= 20.7183dl,
           -3.14159dl <= 2.7183dl,
           -3.14159dl <= -3.14159dl,
           3.14159dl < 3.14159dl,
           3.1416dl < 3.14159dl,
           3.1416dl < 18313.0dl};
#else
int a[] = {0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1};

int b[] = {0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1};

int c[] = {0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1};
#endif
