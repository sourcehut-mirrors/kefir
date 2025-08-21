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

#include "kefir/core/hash.h"

kefir_uint64_t kefir_splitmix64(kefir_uint64_t value) {
    value += KEFIR_SPLITMIX64_MAGIC1;
    value = (value ^ (value >> 30)) * KEFIR_SPLITMIX64_MAGIC2;
    value = (value ^ (value >> 27)) * KEFIR_SPLITMIX64_MAGIC3;
    return value ^ (value >> 31);
}
