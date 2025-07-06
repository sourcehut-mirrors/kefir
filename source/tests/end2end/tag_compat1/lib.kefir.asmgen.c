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

enum enum1 { CONST1 = 1, CONST2 = 3 * 9, CONST3 = 1ull << 63 };

typedef struct structure2 {
    unsigned int x : 12, : 1, y : 5;
} structure2_t;

enum enum1 { CONST2 = 27, CONST3 = 1ull << 63, CONST1 = 1 };

struct structure1 {
    struct {
        long x;
        float y;
    };
    _BitInt(23) z;
};

union union1 {
    short x;
    double y;
};

enum enum2 {
    CONSTX = 1,
    CONSTY = CONSTX * 100,
};

union union1 {
    short x;
    double y;
};

typedef struct structure2 {
    unsigned int x : 12, : 1, y : 5;
} structure2_t;

struct structure1 {
    struct {
        long x;
        float y;
    };
    _BitInt(23) z;
} fn1(void) {
    struct structure3 {
        unsigned _BitInt(200) x;
    };

    struct structure4 {
        union {
            struct structure3 s3;
            _Bool x;
        };
    };

    {
        struct structure3 {
            unsigned _BitInt(200) x;
        };

        struct structure4 {
            union {
                _Bool x;
                struct structure3 s3;
            };
        };
    }

    struct structure3 {
        unsigned _BitInt(200) x;
    };

    struct structure4 {
        union {
            _Bool x;
            struct structure3 s3;
        };
    };
}

struct structure1 {
    struct {
        long x;
        float y;
    };
    _BitInt(23) z;
};

enum enum2 { CONSTX = CONSTX, CONSTY = CONSTY };

enum enum1 { CONST3 = 1ull << 63, CONST2 = 27, CONST1 = 1 };
