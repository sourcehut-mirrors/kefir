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

unsigned _BitInt(128) a = ((unsigned _BitInt(128)) 123456uwb) * 1000000000000000uwb;
unsigned _BitInt(128) b = ((unsigned _BitInt(128)) 444198219381931uwb) * 1228144483818319281uwb;
unsigned _BitInt(128) c = ((unsigned _BitInt(128)) 4441938219381931uwb) * 122814448300000818319281uwb;

signed _BitInt(128) d = ((signed _BitInt(128)) - 0x221f22de4wb) * -2wb;
signed _BitInt(128) e = ((signed _BitInt(128)) 0017313617361371367136wb) * 81737813718371wb;
signed _BitInt(128) f = ((signed _BitInt(128)) - 0b010101110101010101010101010101111111111111111111111111111wb) *
                        0x1fffffffffffffffffffffffffffffffwb;

signed _BitInt(128) g = ((signed _BitInt(128)) - 122424wb) * 133182382478274827917482784uwb;
unsigned _BitInt(128) h = ((unsigned _BitInt(128)) 122222213132324423242553244uwb) * -125835352485292598529wb;
