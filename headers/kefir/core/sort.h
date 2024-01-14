/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifndef KEFIR_CORE_SORT_H_
#define KEFIR_CORE_SORT_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"

typedef kefir_result_t (*kefir_sortcomparator_fn_t)(void *, void *, kefir_int_t *, void *);

kefir_result_t kefir_mergesort(struct kefir_mem *, void *, kefir_size_t, kefir_size_t, kefir_sortcomparator_fn_t,
                               void *);

#endif
