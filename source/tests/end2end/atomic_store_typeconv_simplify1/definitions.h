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

extern _Atomic unsigned char src8;
extern _Atomic unsigned char dst8;
extern _Atomic unsigned short src16;
extern _Atomic unsigned short dst16;
extern _Atomic unsigned int src32;
extern _Atomic unsigned int dst32;
extern _Atomic unsigned long src64;
extern _Atomic unsigned long dst64;

void copy8(void);
void copy16(void);
void copy32(void);
void copy64(void);

#endif
