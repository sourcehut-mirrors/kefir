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

int test1(int x) {
    int y = 0;
    for (int i = 0; i < x; i++) {
        (void) y;
    }
    return y;
}

int test2(int x) {
    volatile int y = 0;
    for (int i = 0; i < x; i++) {
        (void) y;
    }
    return y;
}

int test3(int x) {
    _Atomic int y = 0;
    for (int i = 0; i < x; i++) {
        (void) y;
    }
    return y;
}

int test4(int x) {
    for (int i = 0; i < 10; i++) {
        fn();
    }
    return 0;
}

int test5(int x) {
    for (int i = 0; i < 0; i++) {
        fn();
    }
    return 0;
}

int test6(int x) {
    for (int i = 0; i <= 0; i++) {
        fn();
    }
    return 0;
}

int test7(int x) {
    for (unsigned int i = 0; i < 10; i++) {
        fn();
    }
    return 0;
}

int test8(int x) {
    for (unsigned int i = 0; i < 0; i++) {
        fn();
    }
    return 0;
}

int test9(int x) {
    for (unsigned int i = 0; i <= 0; i++) {
        fn();
    }
    return 0;
}

int test10(int x) {
    for (int i = (__constexpr int) {(1ull << 31)}; i < (__constexpr int) {(1ull << 31) - 1}; i++) {
        fn();
    }
    return 0;
}

int test11(int x) {
    for (int i = (__constexpr int) {(1ull << 31) - 1}; i < (__constexpr int) {(1ull << 31)}; i++) {
        fn();
    }
    return 0;
}

int test12(int x) {
    for (int i = (__constexpr int) {(1ull << 31)}; i < 0; i++) {
        fn();
    }
    return 0;
}

int test13(int x) {
    for (int i = (__constexpr int) {(1ull << 31) - 1}; i < 0; i++) {
        fn();
    }
    return 0;
}

int test14(int x) {
    for (int i = 0; i < (__constexpr int) {(1ull << 31) - 1}; i++) {
        fn();
    }
    return 0;
}

int test15(int x) {
    for (int i = 0; i < (__constexpr int) {(1ull << 31)}; i++) {
        fn();
    }
    return 0;
}

int test16(int x) {
    for (unsigned int i = (__constexpr unsigned int) {(1ull << 31)}; i < (__constexpr unsigned int) {(1ull << 31) - 1};
         i++) {
        fn();
    }
    return 0;
}

int test17(int x) {
    for (unsigned int i = (__constexpr unsigned int) {(1ull << 31) - 1}; i < (__constexpr unsigned int) {(1ull << 31)};
         i++) {
        fn();
    }
    return 0;
}

int test18(int x) {
    for (unsigned int i = (__constexpr unsigned int) {0}; i < (__constexpr unsigned int) {(1ull << 31)}; i++) {
        fn();
    }
    return 0;
}

int test19(int x) {
    for (unsigned int i = (__constexpr unsigned int) {0}; i < (__constexpr unsigned int) {~0ull}; i++) {
        fn();
    }
    return 0;
}

int test20(int x) {
    for (unsigned int i = (__constexpr unsigned int) {~0ull}; i < (__constexpr unsigned int) {0}; i++) {
        fn();
    }
    return 0;
}

int test21(int x) {
    for (int i = (__constexpr int) {~0ull}; i < (__constexpr int) {0}; i++) {
        fn();
    }
    return 0;
}
