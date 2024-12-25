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

#include "./definitions.h"

int less_thanf(float x, float y) {
    if (x < y) {
        return 1;
    } else {
        return 0;
    }
}

int less_than(double x, double y) {
    if (x < y) {
        return 1;
    } else {
        return 0;
    }
}

int less_thanl(long double x, long double y) {
    if (x < y) {
        return 1;
    } else {
        return 0;
    }
}

int eq_less_thanf(float x, float y) {
    if (x <= y) {
        return 1;
    } else {
        return 0;
    }
}

int eq_less_than(double x, double y) {
    if (x <= y) {
        return 1;
    } else {
        return 0;
    }
}

int eq_less_thanl(long double x, long double y) {
    if (x <= y) {
        return 1;
    } else {
        return 0;
    }
}

int eqf(float x, float y) {
    if (x == y) {
        return 1;
    } else {
        return 0;
    }
}

int eq(double x, double y) {
    if (x == y) {
        return 1;
    } else {
        return 0;
    }
}

int eql(long double x, long double y) {
    if (x == y) {
        return 1;
    } else {
        return 0;
    }
}

int neqf(float x, float y) {
    if (x != y) {
        return 1;
    } else {
        return 0;
    }
}

int neq(double x, double y) {
    if (x != y) {
        return 1;
    } else {
        return 0;
    }
}

int neql(long double x, long double y) {
    if (x != y) {
        return 1;
    } else {
        return 0;
    }
}

int greater_thanf(float x, float y) {
    if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

int greater_than(double x, double y) {
    if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

int greater_thanl(long double x, long double y) {
    if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

int eq_greater_thanf(float x, float y) {
    if (x >= y) {
        return 1;
    } else {
        return 0;
    }
}

int eq_greater_than(double x, double y) {
    if (x >= y) {
        return 1;
    } else {
        return 0;
    }
}

int eq_greater_thanl(long double x, long double y) {
    if (x >= y) {
        return 1;
    } else {
        return 0;
    }
}
