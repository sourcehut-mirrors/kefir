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

#ifndef KEFIR_TARGET_ABI_AMD64_SYSTEM_V_DATA_H_
#define KEFIR_TARGET_ABI_AMD64_SYSTEM_V_DATA_H_

#include "kefir/target/abi/amd64/base.h"

typedef enum kefir_abi_amd64_sysv_data_class {
    KEFIR_AMD64_SYSV_PARAM_INTEGER = 0,
    KEFIR_AMD64_SYSV_PARAM_SSE,
    KEFIR_AMD64_SYSV_PARAM_SSEUP,
    KEFIR_AMD64_SYSV_PARAM_X87,
    KEFIR_AMD64_SYSV_PARAM_X87UP,
    KEFIR_AMD64_SYSV_PARAM_COMPLEX_X87,
    KEFIR_AMD64_SYSV_PARAM_NO_CLASS,
    KEFIR_AMD64_SYSV_PARAM_MEMORY
} kefir_abi_amd64_sysv_data_class_t;

#endif
