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

#include "definitions.h"

int cmps[] = {0uwb <= 100uwb,
              0uwb <= 100000000000000000uwb,
              100uwb <= 100uwb,
              101uwb <= 100uwb,
              10000000000000000000000000000uwb <= 100uwb,
              1234uwb <= 67,
              1234uwb <= 67000,
              1234 <= 67uwb,
              1234 <= 67000uwb,

              0wb <= 100wb,
              0wb <= -100wb,
              100wb <= 100wb,
              100wb <= -100wb,
              -100wb <= -100wb,
              -100wb <= 100wb,
              -100wb <= -50,
              -100 <= -50wb,
              -100wb <= -500000,
              -100 <= -500000wb,

              0xfffffffuwb <= -500000wb,
              -500000wb <= 0xfffffffuwb,
              0xfffffffwb <= 500000uwb,
              500000uwb <= 0xfffffffwb,

              1000 <= 1000uwb,
              1000 <= 1000wb,
              1000uwb <= 1000,
              1000wb <= 1000,
              1000wb <= 1000uwb,
              1000uwb <= 1000wb};
