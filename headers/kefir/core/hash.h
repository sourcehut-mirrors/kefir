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

#ifndef KEFIR_CORE_HASH_H_
#define KEFIR_CORE_HASH_H_

#include "kefir/core/basic-types.h"

#define KEFIR_SPLITMIX64_MAGIC1 0x9e3779b97f4a7c15ull
#define KEFIR_SPLITMIX64_MAGIC2 0xbf58476d1ce4e5b9ull
#define KEFIR_SPLITMIX64_MAGIC3 0x94d049bb133111ebull

kefir_uint64_t kefir_splitmix64(kefir_uint64_t);

#endif
