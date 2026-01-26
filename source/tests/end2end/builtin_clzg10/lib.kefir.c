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

int arr[] = {__builtin_clzg((__int128) 0, -1000),          __builtin_clzg((__int128) 1),
             __builtin_clzg(((__int128) 1) << 49),         __builtin_clzg(((__int128) 1) << 100, -3000),
             __builtin_clzg(((__int128) 1) << 127, -5000), __builtin_clzg(((__int128) 1) << 128, -4000)};
