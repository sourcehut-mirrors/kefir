/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#ifndef KEFIR_CORE_OS_ERROR_H_
#define KEFIR_CORE_OS_ERROR_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/error.h"

kefir_result_t kefir_set_os_error(const char *, const char *, unsigned int, struct kefir_error **);
kefir_result_t kefir_set_os_errorf(const char *, const char *, unsigned int, struct kefir_error **, ...);

#define KEFIR_SET_OS_ERROR(_msg) kefir_set_os_error((_msg), __FILE__, __LINE__, NULL)
#define KEFIR_SET_OS_ERRORF(_msg, ...) kefir_set_os_errorf((_msg), __FILE__, __LINE__, NULL, __VA_ARGS__)

#endif
