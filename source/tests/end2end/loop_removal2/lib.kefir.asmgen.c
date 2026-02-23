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

void fn(void);

long test1(long x) {
    long y = 0;
    for (long i = 0; i < x; i++) {
        (void) y;
    }
    return y;
}

long test2(long x) {
    volatile long y = 0;
    for (long i = 0; i < x; i++) {
        (void) y;
    }
    return y;
}

long test3(long x) {
    _Atomic long y = 0;
    for (long i = 0; i < x; i++) {
        (void) y;
    }
    return y;
}

long test4(long x) {
    for (long i = 0; i < 10; i++) {
        fn();
    }
    return 0;
}

long test5(long x) {
    for (long i = 0; i < 0; i++) {
        fn();
    }
    return 0;
}

long test6(long x) {
    for (long i = 0; i <= 0; i++) {
        fn();
    }
    return 0;
}

long test7(long x) {
    for (unsigned long i = 0; i < 10; i++) {
        fn();
    }
    return 0;
}

long test8(long x) {
    for (unsigned long i = 0; i < 0; i++) {
        fn();
    }
    return 0;
}

long test9(long x) {
    for (unsigned long i = 0; i <= 0; i++) {
        fn();
    }
    return 0;
}

long test10(long x) {
    for (long i = (__constexpr long) {(1ull << 63)}; i < (__constexpr long) {(1ull << 63) - 1}; i++) {
        fn();
    }
    return 0;
}

long test11(long x) {
    for (long i = (__constexpr long) {(1ull << 63) - 1}; i < (__constexpr long) {(1ull << 63)}; i++) {
        fn();
    }
    return 0;
}

long test12(long x) {
    for (long i = (__constexpr long) {(1ull << 63)}; i < 0; i++) {
        fn();
    }
    return 0;
}

long test13(long x) {
    for (long i = (__constexpr long) {(1ull << 63) - 1}; i < 0; i++) {
        fn();
    }
    return 0;
}

long test14(long x) {
    for (long i = 0; i < (__constexpr long) {(1ull << 63) - 1}; i++) {
        fn();
    }
    return 0;
}

long test15(long x) {
    for (long i = 0; i < (__constexpr long) {(1ull << 63)}; i++) {
        fn();
    }
    return 0;
}

long test16(long x) {
    for (unsigned long i = (__constexpr unsigned long) {(1ull << 63)};
         i < (__constexpr unsigned long) {(1ull << 63) - 1}; i++) {
        fn();
    }
    return 0;
}

long test17(long x) {
    for (unsigned long i = (__constexpr unsigned long) {(1ull << 63) - 1};
         i < (__constexpr unsigned long) {(1ull << 63)}; i++) {
        fn();
    }
    return 0;
}

long test18(long x) {
    for (unsigned long i = (__constexpr unsigned long) {0}; i < (__constexpr unsigned long) {(1ull << 63)}; i++) {
        fn();
    }
    return 0;
}

long test19(long x) {
    for (unsigned long i = (__constexpr unsigned long) {0}; i < (__constexpr unsigned long) {~0ull}; i++) {
        fn();
    }
    return 0;
}

long test20(long x) {
    for (unsigned long i = (__constexpr unsigned long) {~0ull}; i < (__constexpr unsigned long) {0}; i++) {
        fn();
    }
    return 0;
}

long test21(long x) {
    for (long i = (__constexpr long) {~0ull}; i < (__constexpr long) {0}; i++) {
        fn();
    }
    return 0;
}
