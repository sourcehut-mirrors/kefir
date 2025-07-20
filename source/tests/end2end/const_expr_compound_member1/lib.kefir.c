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

#include "./definitions.h"

struct S1 s1 = {};
long *a = s1.b.d.arr;
double b = ((struct S1) {100, {}, 200.01}).e;
long c = ((struct S1) {100, {3.14f, {}}, -2.71, -250}).f;
_Complex float d = ((struct {
                       int a;
                       struct {
                           _Complex float b;
                       };
                       float c;
                   }) {0, {100.3}})
                       .b;
long **e = ((struct {
               float a;
               struct {
                   void **b;
               };
           }) {.b = (void **) &a})
               .b;
void *f = ((struct { void *x; }) {}).x;
long g = ((struct S1) {}).f;
double h = ((struct {
                    struct {
                        double x;
                    };
                }) {})
                    .x;
_Complex double i = ((struct {
                        int x;
                        _Complex double a;
                    }) {1})
                        .a;
const char *j = ((struct {
    char buf[5];
}){
    "Hello, world!"
}).buf;
