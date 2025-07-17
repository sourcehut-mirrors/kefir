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

#include <typeclass.h>

typedef typeof(nullptr) nullptr_t;

#define ASSERT_TYPE(_expr, _type) static_assert(_Generic((_expr), _type: 1, default: 0))

ASSERT_TYPE((1 ? nullptr : (const int *) 0), const int *);
ASSERT_TYPE((1 ? (const volatile long **) 0 : nullptr), const volatile long **);

static_assert(__builtin_classify_type(nullptr) == pointer_type_class);

ASSERT_TYPE((nullptr_t) 0, nullptr_t);
ASSERT_TYPE((nullptr_t) 1, nullptr_t);
ASSERT_TYPE((nullptr_t) (void *) 0, nullptr_t);

volatile nullptr_t a = 0;
nullptr_t b = (void *) 0;
nullptr_t c = (const long *) 0;
_Bool d = !nullptr;
_Bool e = !!nullptr;
nullptr_t f = !nullptr;
_Bool g = nullptr && 1;
_Bool h = nullptr && nullptr;
_Bool i = nullptr || 1;
_Bool j = 1 || nullptr;

static_assert(!nullptr);
static_assert(!!!nullptr);
static_assert(!(nullptr && true));
static_assert(nullptr || true);

void set1(long x) {
    a = x;
}

void set2(long *x) {
    a = x;
}

nullptr_t get1() {
    return a;
}
