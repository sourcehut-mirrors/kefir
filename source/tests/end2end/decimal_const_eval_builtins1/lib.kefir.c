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

#line __LINE__ "decimal_const_eval_builtins1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
int a[] = {
    __builtin_isnan(1.0df),
    __builtin_isnan(1.0df / 2.0df),
    __builtin_isnan(0.0df / 0.0df),
    __builtin_isnan(-0.0df / 0.0df),
    __builtin_isnan(1.0df / 0.0df),
    __builtin_isnan(-1.0df / 0.0df),
    __builtin_isnan(1.0dd),
    __builtin_isnan(1.0dd / 2.0dd),
    __builtin_isnan(0.0dd / 0.0dd),
    __builtin_isnan(-0.0dd / 0.0dd),
    __builtin_isnan(1.0dd / 0.0dd),
    __builtin_isnan(-1.0dd / 0.0dd),
    __builtin_isnan(1.0dl),
    __builtin_isnan(1.0dl / 2.0dl),
    __builtin_isnan(0.0dl / 0.0dl),
    __builtin_isnan(-0.0dl / 0.0dl),
    __builtin_isnan(1.0dl / 0.0dl),
    __builtin_isnan(-1.0dl / 0.0dl)
};

int b[] = {
    __builtin_isinf_sign(1.0df),
    __builtin_isinf_sign(1.0df / 2.0df),
    __builtin_isinf_sign(0.0df / 0.0df),
    __builtin_isinf_sign(-0.0df / 0.0df),
    __builtin_isinf_sign(1.0df / 0.0df),
    __builtin_isinf_sign(-1.0df / 0.0df),
    __builtin_isinf_sign(1.0dd),
    __builtin_isinf_sign(1.0dd / 2.0dd),
    __builtin_isinf_sign(0.0dd / 0.0dd),
    __builtin_isinf_sign(-0.0dd / 0.0dd),
    __builtin_isinf_sign(1.0dd / 0.0dd),
    __builtin_isinf_sign(-1.0dd / 0.0dd),
    __builtin_isinf_sign(1.0dl),
    __builtin_isinf_sign(1.0dl / 2.0dl),
    __builtin_isinf_sign(0.0dl / 0.0dl),
    __builtin_isinf_sign(-0.0dl / 0.0dl),
    __builtin_isinf_sign(1.0dl / 0.0dl),
    __builtin_isinf_sign(-1.0dl / 0.0dl)
};

int c[] = {
    __builtin_isfinite(1.0df),
    __builtin_isfinite(1.0df / 2.0df),
    __builtin_isfinite(0.0df / 0.0df),
    __builtin_isfinite(-0.0df / 0.0df),
    __builtin_isfinite(1.0df / 0.0df),
    __builtin_isfinite(-1.0df / 0.0df),
    __builtin_isfinite(1.0dd),
    __builtin_isfinite(1.0dd / 2.0dd),
    __builtin_isfinite(0.0dd / 0.0dd),
    __builtin_isfinite(-0.0dd / 0.0dd),
    __builtin_isfinite(1.0dd / 0.0dd),
    __builtin_isfinite(-1.0dd / 0.0dd),
    __builtin_isfinite(1.0dl),
    __builtin_isfinite(1.0dl / 2.0dl),
    __builtin_isfinite(0.0dl / 0.0dl),
    __builtin_isfinite(-0.0dl / 0.0dl),
    __builtin_isfinite(1.0dl / 0.0dl),
    __builtin_isfinite(-1.0dl / 0.0dl)
};
#else
int a[] = {
    0,
    0,
    1,
    1,
    0,
    0,

    0,
    0,
    1,
    1,
    0,
    0,

    0,
    0,
    1,
    1,
    0,
    0
};

int b[] = {
    0,
    0,
    0,
    0,
    1,
    1,

    0,
    0,
    0,
    0,
    1,
    1,

    0,
    0,
    0,
    0,
    1,
    1
};

int c[] = {
    1,
    1,
    0,
    0,
    0,
    0,
    
    1,
    1,
    0,
    0,
    0,
    0,

    1,
    1,
    0,
    0,
    0,
    0
};
#endif
