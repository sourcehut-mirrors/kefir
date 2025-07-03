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

int eq1(void) {
    return 0x123456789abcdef0987654321wb == 0x123456789abcdef0987654321uwb;
}

int eq2(void) {
    return 0x123456789abcdef0987654321wb == 0x123456789abcdef0987654320wb;
}

int greater1(void) {
    return 0x123456789abcdef0987654321wb > 0x123456789abcdef0987654321wb;
}

int greater2(void) {
    return 0x123456789abcdef0987654321wb > 0x123456789abcdef0987654320wb;
}

int greater3(void) {
    return 0x123456789abcdef0987654320wb > 0x123456789abcdef0987654321wb;
}

int less1(void) {
    return 0x123456789abcdef0987654321wb < 0x123456789abcdef0987654321wb;
}

int less2(void) {
    return 0x123456789abcdef0987654321wb < 0x123456789abcdef0987654320wb;
}

int less3(void) {
    return 0x123456789abcdef0987654320wb < 0x123456789abcdef0987654321wb;
}

int above1(void) {
    return 0x123456789abcdef0987654321uwb > 0x123456789abcdef0987654321uwb;
}

int above2(void) {
    return 0x123456789abcdef0987654321uwb > 0x123456789abcdef0987654320uwb;
}

int above3(void) {
    return 0x123456789abcdef0987654320uwb > 0x123456789abcdef0987654321uwb;
}

int below1(void) {
    return 0x123456789abcdef0987654321uwb < 0x123456789abcdef0987654321uwb;
}

int below2(void) {
    return 0x123456789abcdef0987654321uwb < 0x123456789abcdef0987654320uwb;
}

int below3(void) {
    return 0x123456789abcdef0987654320uwb < 0x123456789abcdef0987654321uwb;
}
