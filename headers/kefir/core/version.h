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

#ifndef KEFIR_CORE_VERSION_H_
#define KEFIR_CORE_VERSION_H_

#include "kefir/core/base.h"

#define KEFIR_VERSION_MAJOR 0
#define KEFIR_VERSION_MINOR 5
#define KEFIR_VERSION_PATCH 0

#define KEFIR_BUILD_RELEASE_STR_HELPER(_x) #_x
#define KEFIR_BUILD_RELEASE_STR(_x) KEFIR_BUILD_RELEASE_STR_HELPER(_x)
#define KEFIR_VERSION_SHORT                      \
    KEFIR_BUILD_RELEASE_STR(KEFIR_VERSION_MAJOR) \
    "." KEFIR_BUILD_RELEASE_STR(KEFIR_VERSION_MINOR) "." KEFIR_BUILD_RELEASE_STR(KEFIR_VERSION_PATCH)
#ifdef KEFIR_BUILD_RELEASE
#define KEFIR_VERSION_FULL KEFIR_VERSION_SHORT
#else
#ifdef KEFIR_BUILD_SOURCE_ID
#define KEFIR_VERSION_FULL KEFIR_VERSION_SHORT ".dev." KEFIR_BUILD_SOURCE_ID
#else
#define KEFIR_VERSION_FULL KEFIR_VERSION_SHORT ".dev"
#endif
#endif

#endif
