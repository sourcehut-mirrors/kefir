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

extern _Atomic char i8;
extern _Atomic short i16;
extern _Atomic int i32;
extern _Atomic long i64;
extern _Atomic(long *) ptr;
extern _Atomic float f32;
extern _Atomic double f64;
extern _Atomic long double ld;

char postinc_i8(void);
short postinc_i16(void);
int postinc_i32(void);
long postinc_i64(void);
long *postinc_ptr(void);
float postinc_f32(void);
double postinc_f64(void);
long double postinc_ld(void);

char postdec_i8(void);
short postdec_i16(void);
int postdec_i32(void);
long postdec_i64(void);
long *postdec_ptr(void);
float postdec_f32(void);
double postdec_f64(void);
long double postdec_ld(void);

#endif
