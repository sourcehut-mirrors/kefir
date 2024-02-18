/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

char preinc_i8(void);
short preinc_i16(void);
int preinc_i32(void);
long preinc_i64(void);
long *preinc_ptr(void);
float preinc_f32(void);
double preinc_f64(void);
long double preinc_ld(void);

char predec_i8(void);
short predec_i16(void);
int predec_i32(void);
long predec_i64(void);
long *predec_ptr(void);
float predec_f32(void);
double predec_f64(void);
long double predec_ld(void);

#endif
