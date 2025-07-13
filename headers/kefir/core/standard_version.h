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

#ifndef KEFIR_CORE_STANDARD_VERSION_H_
#define KEFIR_CORE_STANDARD_VERSION_H_

#include "kefir/core/base.h"

typedef enum kefir_c_language_standard_version {
    KEFIR_C17_STANDARD_VERSION = 201710,
    KEFIR_C23_STANDARD_VERSION = 202311
} kefir_c_language_standard_version_t;

#define KEFIR_DEFAULT_STANDARD_VERSION KEFIR_C17_STANDARD_VERSION

#endif
