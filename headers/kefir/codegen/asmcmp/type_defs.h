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

#ifndef KEFIR_CODEGEN_ASMCMP_TYPE_DEFS_H_
#define KEFIR_CODEGEN_ASMCMP_TYPE_DEFS_H_

#include "kefir/core/basic-types.h"
#include "kefir/codegen/asmcmp/base.h"

typedef kefir_size_t kefir_asmcmp_virtual_register_index_t;
typedef kefir_size_t kefir_asmcmp_physical_register_index_t;
typedef kefir_size_t kefir_asmcmp_instruction_opcode_t;
typedef kefir_size_t kefir_asmcmp_instruction_index_t;
typedef kefir_size_t kefir_asmcmp_label_index_t;
typedef kefir_size_t kefir_asmcmp_stash_index_t;
typedef kefir_size_t kefir_asmcmp_inline_assembly_index_t;
typedef kefir_size_t kefir_asmcmp_lifetime_index_t;

#define KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS 3
#define KEFIR_ASMCMP_INDEX_NONE (~(kefir_asmcmp_instruction_index_t) 0ull)

#endif
