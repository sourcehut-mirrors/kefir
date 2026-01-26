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

_Complex float neg32(void);
_Complex double neg64(void);
_Complex long double neg80(void);
_Complex float add32(void);
_Complex double add64(void);
_Complex long double add80(void);
_Complex float sub32(void);
_Complex double sub64(void);
_Complex long double sub80(void);
_Complex float mul32(void);
_Complex double mul64(void);
_Complex long double mul80(void);
_Complex float div32(void);
_Complex double div64(void);
_Complex long double div80(void);

#endif
