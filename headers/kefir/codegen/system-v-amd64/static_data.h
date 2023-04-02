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

#ifndef KEFIR_CODEGEN_SYSTEM_V_AMD64_STATIC_DATA_H_
#define KEFIR_CODEGEN_SYSTEM_V_AMD64_STATIC_DATA_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/vector.h"
#include "kefir/core/mem.h"
#include "kefir/codegen/system-v-amd64.h"
#include "kefir/codegen/system-v-amd64/registers.h"
#include "kefir/ir/data.h"

kefir_result_t kefir_amd64_sysv_static_data(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                            const struct kefir_ir_data *, const char *);

#endif
