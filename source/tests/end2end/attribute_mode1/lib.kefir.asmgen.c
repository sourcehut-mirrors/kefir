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

int i8_1 __attribute__((mode(byte)));
int i8_2 __attribute__((mode(__byte__)));
int i8_3 __attribute__((mode(QI)));

int i16_1 __attribute__((mode(HI)));
char i32_1 __attribute__((mode(SI)));
int i64_1 __attribute__((mode(DI)));

int word_1 __attribute__((mode(word)));
int word_2 __attribute__((mode(__word__)));

int ptr_1 __attribute__((mode(pointer)));
int ptr_2 __attribute__((mode(__pointer__)));

unsigned int u8_1 __attribute__((mode(byte)));
unsigned int u8_2 __attribute__((mode(__byte__)));
unsigned int u8_3 __attribute__((mode(QI)));

unsigned int u16_1 __attribute__((mode(HI)));
unsigned char u32_1 __attribute__((mode(SI)));
unsigned int u64_1 __attribute__((mode(DI)));

unsigned int uword_1 __attribute__((mode(word)));
unsigned int uword_2 __attribute__((mode(__word__)));

unsigned int uptr_1 __attribute__((mode(pointer)));
unsigned int uptr_2 __attribute__((mode(__pointer__)));

const volatile int test1 __attribute__((__mode__(DI)));

long double f32_1 __attribute__((mode(SF)));
long double f64_1 __attribute__((mode(DF)));
float f80_1 __attribute__((mode(XF)));

#define ASSERT_TYPE(_expr, _type) _Static_assert(_Generic(_expr, _type: 1, default: 0))

ASSERT_TYPE(&i8_1, signed char *);
ASSERT_TYPE(&i8_2, signed char *);
ASSERT_TYPE(&i8_3, signed char *);

ASSERT_TYPE(&i16_1, signed short *);
ASSERT_TYPE(&i32_1, signed int *);
ASSERT_TYPE(&i64_1, signed long *);

ASSERT_TYPE(&word_1, signed long *);
ASSERT_TYPE(&word_2, signed long *);

ASSERT_TYPE(&ptr_1, signed long *);
ASSERT_TYPE(&ptr_2, signed long *);

ASSERT_TYPE(&u8_1, unsigned char *);
ASSERT_TYPE(&u8_2, unsigned char *);
ASSERT_TYPE(&u8_3, unsigned char *);

ASSERT_TYPE(&u16_1, unsigned short *);
ASSERT_TYPE(&u32_1, unsigned int *);
ASSERT_TYPE(&u64_1, unsigned long *);

ASSERT_TYPE(&uword_1, unsigned long *);
ASSERT_TYPE(&uword_2, unsigned long *);

ASSERT_TYPE(&uptr_1, unsigned long *);
ASSERT_TYPE(&uptr_2, unsigned long *);

ASSERT_TYPE(&test1, const volatile long *);

ASSERT_TYPE(&f32_1, float *);
ASSERT_TYPE(&f64_1, double *);
ASSERT_TYPE(&f80_1, long double *);
