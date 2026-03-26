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

#define DECL(_id)                                                                          \
    void test_atomic_store8_##_id(_Atomic char *, char);                                   \
    void test_atomic_store16_##_id(_Atomic short *, short);                                \
    void test_atomic_store32_##_id(_Atomic int *, int);                                    \
    void test_atomic_store64_##_id(_Atomic long *, long);                                  \
    void test_atomic_store128_##_id(_Atomic long double *, long double);                   \
    void test_atomic_store256_##_id(_Atomic _Complex long double *, _Complex long double); \
    void test2_atomic_store8_##_id(_Atomic char *, char);                                  \
    void test2_atomic_store16_##_id(_Atomic short *, short);                               \
    void test2_atomic_store32_##_id(_Atomic int *, int);                                   \
    void test2_atomic_store64_##_id(_Atomic long *, long);                                 \
    void test2_atomic_store128_##_id(_Atomic long double *, long double);                  \
    void test2_atomic_store256_##_id(_Atomic _Complex long double *, _Complex long double)

DECL(relaxed);
DECL(consume);
DECL(acquire);
DECL(release);
DECL(acq_rel);
DECL(seq_cst);

#undef DECL

#endif
