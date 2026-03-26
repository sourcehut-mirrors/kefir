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

#include "./definitions.h"

#define GEN_STORE(_id, _order)                                       \
    void test_atomic_store8_##_id(_Atomic char *ptr, char val) {     \
        __atomic_store(ptr, &val, _order);                           \
    }                                                                \
                                                                     \
    void test_atomic_store16_##_id(_Atomic short *ptr, short val) {  \
        __atomic_store(ptr, &val, _order);                           \
    }                                                                \
                                                                     \
    void test_atomic_store32_##_id(_Atomic int *ptr, int val) {      \
        __atomic_store(ptr, &val, _order);                           \
    }                                                                \
                                                                     \
    void test_atomic_store64_##_id(_Atomic long *ptr, long val) {    \
        __atomic_store(ptr, &val, _order);                           \
    }                                                                \
                                                                     \
    void test2_atomic_store8_##_id(_Atomic char *ptr, char val) {    \
        __atomic_store_n(ptr, val, _order);                          \
    }                                                                \
                                                                     \
    void test2_atomic_store16_##_id(_Atomic short *ptr, short val) { \
        __atomic_store_n(ptr, val, _order);                          \
    }                                                                \
                                                                     \
    void test2_atomic_store32_##_id(_Atomic int *ptr, int val) {     \
        __atomic_store_n(ptr, val, _order);                          \
    }                                                                \
                                                                     \
    void test2_atomic_store64_##_id(_Atomic long *ptr, long val) {   \
        __atomic_store_n(ptr, val, _order);                          \
    }

GEN_STORE(relaxed, __ATOMIC_RELAXED)
GEN_STORE(consume, __ATOMIC_CONSUME)
GEN_STORE(acquire, __ATOMIC_ACQUIRE)
GEN_STORE(release, __ATOMIC_RELEASE)
GEN_STORE(acq_rel, __ATOMIC_ACQ_REL)
GEN_STORE(seq_cst, __ATOMIC_SEQ_CST)
#undef GEN_STORE

#if !defined(__DragonFly__) || defined(KEFIR_END2END_ASMGEN)
#define GEN_STORE(_id, _order)                                                                      \
    void test_atomic_store128_##_id(_Atomic long double *ptr, long double val) {                    \
        __atomic_store(ptr, &val, _order);                                                \
    }                                                                                               \
                                                                                                    \
    void test_atomic_store256_##_id(_Atomic _Complex long double *ptr, _Complex long double val) {  \
        __atomic_store(ptr, &val, _order);                                                \
    }                                                                                               \
                                                                                                    \
    void test2_atomic_store128_##_id(_Atomic long double *ptr, long double val) {                   \
        __atomic_store_n(ptr, val, _order);                                               \
    }                                                                                               \
                                                                                                    \
    void test2_atomic_store256_##_id(_Atomic _Complex long double *ptr, _Complex long double val) { \
        __atomic_store_n(ptr, val, _order);                                               \
    }

GEN_STORE(relaxed, __ATOMIC_RELAXED)
GEN_STORE(consume, __ATOMIC_CONSUME)
GEN_STORE(acquire, __ATOMIC_ACQUIRE)
GEN_STORE(release, __ATOMIC_RELEASE)
GEN_STORE(acq_rel, __ATOMIC_ACQ_REL)
GEN_STORE(seq_cst, __ATOMIC_SEQ_CST)
#endif
