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

int bools[] = {123455465uwb && 3242uwb,
               123455465uwb && 0uwb,
               0uwb && -132432498wb,
               134249uwb && -132432498wb,
               0x100000000000000000000000000000000000000000000000000000uwb && 1,
               -0x100000000000000000000000000000000000000000000000000000wb && 2,
               0 && -0x100000000000000000000000000000000000000000000000000000wb,

               1232848274uwb || 0uwb,
               1232848274uwb || -1wb,
               0b00000000uwb || 0wb,
               0x1000000000000000000000000000000000000000000000000000000000000000uwb || 0x000000000000000wb,
               0x1000000000000000000000000000000000000000000000000000000000000000uwb ||
                   0x1000000000000000000000000000000000000000000wb,
               -0x000000000000000000000000000000000000000000000000000000000000000wb ||
                   -0x0000000000000000000000000000000000000000000uwb};
