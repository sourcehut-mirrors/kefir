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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

long sum(long *, ...);

int main(void) {
    assert(sum(NULL) == 0);
    assert(sum(&(long) {1}, NULL) == 1);
    assert(sum(&(long) {1}, &(long) {2}, NULL) == 3);
    assert(sum(&(long) {1}, &(long) {2}, &(long) {-10}, NULL) == -7);
    assert(sum(&(long) {1}, &(long) {2}, &(long) {-10}, &(long) {100}, NULL) == 93);
    assert(sum(&(long) {1}, &(long) {2}, &(long) {-10}, &(long) {100}, NULL, &(long) {-1}) == 93);
    assert(sum(&(long) {1}, &(long) {2}, &(long) {-10}, &(long) {100}, &(long) {1}, NULL, &(long) {-1}) == 94);
    return EXIT_SUCCESS;
}
