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

extern _Atomic _Bool b;
extern _Atomic unsigned char u8;
extern _Atomic unsigned short u16;
extern _Atomic unsigned int u32;
extern _Atomic unsigned long u64;
extern _Atomic float f32;
extern _Atomic double f64;

void set_b(_Bool);
void set_u8(unsigned char);
void set_u16(unsigned short);
void set_u32(unsigned int);
void set_u64(unsigned long);
void set_f32(float);
void set_f64(double);

#endif
