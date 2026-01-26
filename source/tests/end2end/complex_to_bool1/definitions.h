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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

extern _Bool b0[], b32[], b32_2[], b64[], b64_2[], b64_3[], b80[], b80_2[], b80_3[];

_Bool c32_to_bool(_Complex float);
_Bool c32_to_bool2(_Complex float);
_Bool c64_to_bool(_Complex double);
_Bool c64_to_bool2(_Complex double);
_Bool c64_to_bool3(_Complex double);
_Bool c80_to_bool(_Complex long double);
_Bool c80_to_bool2(_Complex long double);
_Bool c80_to_bool3(_Complex long double);

#endif
