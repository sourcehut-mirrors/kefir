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

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
int a[] = {
    31.4df ? 1 : -1,
    0.0df ? 1 : -1,
    31.4dd ? 1 : -1,
    0.0dd ? 1 : -1,
    31.4dl ? 1 : -1,
    0.0dl ? 1 : -1
};
#else
int a[] = {
    1, -1,
    1, -1,
    1, -1
};
#endif
