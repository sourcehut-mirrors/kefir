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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

struct S3 {
    unsigned long arr[3];
};

extern struct S3 x, y;

struct S3 add(struct S3);
struct S3 sub(struct S3);
struct S3 imul(struct S3);
struct S3 umul(struct S3);
struct S3 idiv(struct S3);
struct S3 udiv(struct S3);
struct S3 imod(struct S3);
struct S3 umod(struct S3);
struct S3 and (struct S3);
struct S3 or (struct S3);
struct S3 xor (struct S3);
struct S3 lshift(struct S3);
struct S3 rshift(struct S3);
struct S3 arshift(struct S3);

#endif
