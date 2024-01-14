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

int factorial(int x) {
    if (x < 0)
        return 0;
    if (x <= 2)
        return x;

    int result = 1;
    for (; x > 1; --x) {
        result *= x;
    }
    return result;
}

long factorial64(long x) {
    if (x < 0) {
        return 0;
    } else if (x <= 2) {
        return x;
    } else {
        long result = 1;
        for (; x > 1; --x) {
            result *= x;
        }
        return result;
    }
}

float factorialf32(float x) {
    if (x < 0) {
        return 0.0f;
    } else if (x <= 2) {
        return x;
    } else {
        float result = 1.0f;
        for (; x > 1.0f; --x) {
            result *= x;
        }
        return result;
    }
}

double factorialf64(double x) {
    if (x < 0) {
        return 0.0;
    } else if (x <= 2) {
        return x;
    } else {
        double result = 1.0f;
        for (; x > 1.0; --x) {
            result *= x;
        }
        return result;
    }
}

extern double sqrt(double);

double quad_equation(double a, double b, double c, int s) {
    double D = b * b - 4 * a * c;
    return (-b + (s ? 1 : -1) * sqrt(D)) / (2 * a);
}
