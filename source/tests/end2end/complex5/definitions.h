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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

extern _Complex float sum32(_Complex float, _Complex float, _Complex float, _Complex float, _Complex float,
                            _Complex float, _Complex float, _Complex float, _Complex float, _Complex float,
                            _Complex float, _Complex float);
extern _Complex double sum64(_Complex double, _Complex double, _Complex double, _Complex double, _Complex double,
                             _Complex double, _Complex double, _Complex double, _Complex double, _Complex double,
                             _Complex double, _Complex double);
extern _Complex long double sumld(_Complex long double, _Complex long double, _Complex long double,
                                  _Complex long double, _Complex long double, _Complex long double,
                                  _Complex long double, _Complex long double, _Complex long double,
                                  _Complex long double, _Complex long double, _Complex long double);

_Complex float test32(_Complex float);
_Complex double test64(_Complex double);
_Complex long double testld(_Complex long double);

struct Struct1 {
    _Complex float a;
    _Complex long double b;
    _Complex double c;
    _Complex long double x;
    _Complex float y;
    _Complex double z;
};

union Union1 {
    _Complex float a;
    _Complex double b;
    _Complex long double c;
};

union Union1 test_struct(struct Struct1);

_Complex float vsum32(int, ...);
_Complex double vsum64(int, ...);
_Complex long double vsumld(int, ...);

#endif
