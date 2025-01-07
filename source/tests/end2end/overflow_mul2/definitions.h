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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

_Bool overflow_smul(int, int, int *);
_Bool overflow_smull(long int, long int, long int *);
_Bool overflow_smulll(long long int, long long int, long long int *);

_Bool overflow_umul(unsigned int, unsigned int, unsigned int *);
_Bool overflow_umull(unsigned long int, unsigned long int, unsigned long int *);
_Bool overflow_umulll(unsigned long long int, unsigned long long int, unsigned long long int *);

#endif
