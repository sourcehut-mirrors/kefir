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

#include "./definitions.h"

void test_atomic_thread_fence(void) {
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
}

void test_atomic_signal_fence(void) {
    __atomic_signal_fence(__ATOMIC_SEQ_CST);
}

_Bool test_is_lock_free(void) {
    return __atomic_is_lock_free(sizeof(_Bool), (_Bool *) 0) == __GCC_ATOMIC_BOOL_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(char), (char *) 0) == __GCC_ATOMIC_CHAR_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(__CHAR16_TYPE__), (__CHAR16_TYPE__ *) 0) == __GCC_ATOMIC_CHAR16_T_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(__CHAR32_TYPE__), (__CHAR32_TYPE__ *) 0) == __GCC_ATOMIC_CHAR32_T_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(__WCHAR_TYPE__), (__WCHAR_TYPE__ *) 0) == __GCC_ATOMIC_WCHAR_T_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(short), (short *) 0) == __GCC_ATOMIC_SHORT_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(int), (int *) 0) == __GCC_ATOMIC_INT_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(long), (long *) 0) == __GCC_ATOMIC_LONG_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(long long), (long long *) 0) == __GCC_ATOMIC_LLONG_LOCK_FREE &&
           __atomic_is_lock_free(sizeof(void *), (void **) 0) == __GCC_ATOMIC_POINTER_LOCK_FREE;
}

_Bool test_and_set(_Bool *ptr) {
    return __atomic_test_and_set(ptr, __ATOMIC_SEQ_CST);
}

void clear(_Bool *ptr) {
    __atomic_clear(ptr, __ATOMIC_SEQ_CST);
}
