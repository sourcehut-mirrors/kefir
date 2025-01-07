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

struct Test1 {
    struct {
        struct {
            unsigned char f1;
        } f2;
        void *f3;
        void *f4;
        unsigned long f5;
    } interpreters;

    void *f6;

    struct {
        struct {
            int f7;
            unsigned long f8;
            unsigned long f9;
        } f10;
    } f11;

    struct {
        struct {
            int f12;
        } f13[(64 + 1)];

        struct {
            int f14;
            int f15;
        } f16;
    } f17;

    struct {
        char f18[8];
        unsigned long f19;
        unsigned long f20;

        struct {
            unsigned long f21;
            unsigned long f22;
            unsigned long f23;
        } f24;

        struct {
            unsigned long f25;
            unsigned long f26;
            unsigned long f27;
            unsigned long f28;
            unsigned long f29;
            unsigned long f30;
            unsigned long f31;
            unsigned long f32;
            unsigned long f33;
            unsigned long f34;
            unsigned long f35;
            unsigned long f36;
            unsigned long f37;
        } f38;

        struct {
            unsigned long f39;
            unsigned long f40;
            unsigned long f41;
            unsigned long f42;
            unsigned long f43;
            unsigned long f44;
            unsigned long f45;
            unsigned long f46;
            unsigned long f47;
        } f48;

        struct {
            unsigned long f49;
            unsigned long f50;
            unsigned long f51;
            unsigned long f52;
            unsigned long f53;
            unsigned long f54;
        } f55;

        struct {
            unsigned long f56;
            unsigned long f57;
            unsigned long f58;
            unsigned long f59;
            unsigned long f60;
            unsigned long f61;
            unsigned long f62;
            unsigned long f63;
            unsigned long f64;
            unsigned long f65;
        } f66;

        struct {
            unsigned long f67;
            unsigned long f68;
        } f69;

        struct {
            unsigned long f70;
            unsigned long f71;
            unsigned long f72;
            unsigned long f73;
        } f74;

        struct {
            unsigned long f75;
            unsigned long f76;
            unsigned long f77;
        } f78;

        struct {
            unsigned long f79;
            unsigned long f80;
            unsigned long f81;
        } f82;

        struct {
            unsigned long f83;
            unsigned long f84;
            unsigned long f85;
        } f86;

        struct {
            unsigned long f87;
            unsigned long f88;
        } f89;
    } f90;
};

extern struct Test1 value;

#endif
