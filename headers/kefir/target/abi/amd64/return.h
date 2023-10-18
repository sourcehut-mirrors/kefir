/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#ifndef KEFIR_TARGET_ABI_AMD64_RETURN_H_
#define KEFIR_TARGET_ABI_AMD64_RETURN_H_

#include "kefir/core/basic-types.h"
#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/target/abi/amd64/base.h"

kefir_size_t kefir_abi_amd64_num_of_general_purpose_return_registers(kefir_abi_amd64_variant_t);
kefir_result_t kefir_abi_amd64_general_purpose_return_register(kefir_abi_amd64_variant_t, kefir_size_t,
                                                               kefir_asm_amd64_xasmgen_register_t *);
kefir_size_t kefir_abi_amd64_num_of_sse_return_registers(kefir_abi_amd64_variant_t);
kefir_result_t kefir_abi_amd64_sse_return_register(kefir_abi_amd64_variant_t, kefir_size_t,
                                                   kefir_asm_amd64_xasmgen_register_t *);

#endif
