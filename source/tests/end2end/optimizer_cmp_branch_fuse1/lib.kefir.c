/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

int equals(int x, int y) {
    if (x == y) {
        return 1;
    } else {
        return 0;
    }
}

int not_equals(int x, int y) {
    if (x != y) {
        return 1;
    } else {
        return 0;
    }
}

int not_equals2(int x, int y) {
    if (!(x == y)) {
        return 1;
    } else {
        return 0;
    }
}

int greater(int x, int y) {
    if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

int not_greater(int x, int y) {
    if (!(x > y)) {
        return 1;
    } else {
        return 0;
    }
}

int less(int x, int y) {
    if (x < y) {
        return 1;
    } else {
        return 0;
    }
}

int not_less(int x, int y) {
    if (!(x < y)) {
        return 1;
    } else {
        return 0;
    }
}

int above(unsigned int x, unsigned int y) {
    if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

int not_above(unsigned int x, unsigned int y) {
    if (!(x > y)) {
        return 1;
    } else {
        return 0;
    }
}

int below(unsigned int x, unsigned int y) {
    if (x < y) {
        return 1;
    } else {
        return 0;
    }
}

int not_below(unsigned int x, unsigned int y) {
    if (!(x < y)) {
        return 1;
    } else {
        return 0;
    }
}
