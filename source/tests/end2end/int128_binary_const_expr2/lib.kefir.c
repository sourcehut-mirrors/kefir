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

unsigned __int128 a = ((unsigned __int128) 123456uwb) * ((unsigned __int128) 1000000000000000uwb);
unsigned __int128 b = ((unsigned __int128) 444198219381931uwb) * ((unsigned __int128) 1228144483818319281uwb);
unsigned __int128 c = ((unsigned __int128) 4441938219381931uwb) * ((unsigned __int128) 122814448300000818319281uwb);

signed __int128 d = ((signed __int128) - 0x221f22de4wb) * ((signed __int128) -2wb);
signed __int128 e = ((signed __int128) 0017313617361371367136wb) * ((signed __int128) 81737813718371wb);
signed __int128 f = ((signed __int128) - 0b010101110101010101010101010101111111111111111111111111111wb) *
                        ((signed __int128) 0x1fffffffffffffffffffffffffffffffwb);

signed __int128 g = ((signed __int128) - 122424wb) * ((unsigned __int128) 133182382478274827917482784uwb);
unsigned __int128 h = ((unsigned __int128) 122222213132324423242553244uwb) * ((signed __int128) -125835352485292598529wb);
