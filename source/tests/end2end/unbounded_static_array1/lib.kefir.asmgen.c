/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

int publicarray[];
_Thread_local tpublicarray[];

static int somearray[];
static int someotherarray[];
static int someotherarray[10];

static _Thread_local int tsomearray[];
static _Thread_local int tsomeotherarray[];
static _Thread_local int tsomeotherarray[10];

void *get() {
    return somearray;
}

void *get2() {
    return someotherarray;
}

void *tget() {
    return tsomearray;
}

void *tget2() {
    return tsomeotherarray;
}