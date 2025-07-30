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
constexpr unsigned _BitInt(135) p135_1 = 4187273188098476865668456823817923591884uwb;
constexpr unsigned _BitInt(135) p135_2 = 6807534906955493735159263291613578822092uwb;
constexpr unsigned _BitInt(135) p135_3 = 41026661032626929517457579844514879338728uwb;
constexpr unsigned _BitInt(228) r228 = 13524290028307164406682090022182954141616uwb;

int result = p135_1 + p135_2 - p135_3 != r228;
