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

unsigned _BitInt(128) a = ((unsigned _BitInt(128)) 0x12345670000000uwb) / 10000uwb;
unsigned _BitInt(128) b = ((unsigned _BitInt(128)) 0x12445uwb) / 0xfffffffffuwb;
unsigned _BitInt(128) c = ((unsigned _BitInt(128)) 0x1245453535353553545uwb) / 0x1uwb;

signed _BitInt(128) d = ((signed _BitInt(128)) 0x124542483945uwb) / -1wb;
signed _BitInt(128) e = ((signed _BitInt(128)) - 04353646357426464101wb) / 248247742847wb;
signed _BitInt(128) f = ((signed _BitInt(128)) - 0x43535edfe44242424414edfabc2wb) / -1111001wb;
signed _BitInt(128) g = ((signed _BitInt(128)) 0x43535edfe44242424414edfabc2wb) / 1111001wb;

signed _BitInt(128) h = ((signed _BitInt(128)) - 0x43535edfe44242424414edfabc2wb) / 0x100000001uwb;
unsigned _BitInt(128) i = ((unsigned _BitInt(128)) 424453535243363533uwb) / -535353522wb;
