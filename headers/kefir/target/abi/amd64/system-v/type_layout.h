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

#ifndef KEFIR_TARGET_ABI_AMD64_SYSTEM_V_TYPE_LAYOUT_H_
#define KEFIR_TARGET_ABI_AMD64_SYSTEM_V_TYPE_LAYOUT_H_

#include "kefir/target/abi/amd64/type_layout.h"

kefir_result_t kefir_abi_amd64_sysv_calculate_type_layout(struct kefir_mem *,
                                                          kefir_abi_amd64_type_layout_context_t context,
                                                          const struct kefir_ir_type *, kefir_size_t, kefir_size_t,
                                                          struct kefir_vector *);

#define KEFIR_ABI_AMD64_SYSV_MAX_ALIGNMENT (2 * KEFIR_AMD64_ABI_QWORD)

#endif
