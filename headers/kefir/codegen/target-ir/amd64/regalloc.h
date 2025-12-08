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

#ifndef KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_H_
#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_H_

#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/target/abi/amd64/base.h"
#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/util/json.h"

#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_CLASS_REGISTERS 16

#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA 0ull
#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP 1ull
#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE 2ull
#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL 3ull

#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_NA ((kefir_codegen_target_ir_regalloc_allocation_t) (KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_NA << 56))
#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_GP(_reg) (kefir_codegen_target_ir_regalloc_allocation_t) ((KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_GP << 56) | (kefir_uint32_t) (_reg))
#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_SSE(_reg) (kefir_codegen_target_ir_regalloc_allocation_t) ((KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SSE << 56) | (kefir_uint32_t) (_reg))
#define KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_SPILL(_index, _length) (kefir_codegen_target_ir_regalloc_allocation_t) ((KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_TYPE_SPILL << 56) | (((kefir_uint64_t) (kefir_uint16_t) (_index)) << 32) | (kefir_uint32_t) (_length))

typedef struct kefir_codegen_target_ir_amd64_regalloc_class {
    struct kefir_codegen_target_ir_regalloc_class klass;
    kefir_abi_amd64_variant_t abi_variant;
    kefir_asm_amd64_xasmgen_register_t gp_registers[KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_CLASS_REGISTERS];
    kefir_size_t num_of_gp_registers;
    kefir_asm_amd64_xasmgen_register_t sse_registers[KEFIR_CODEGEN_TARGET_IR_AMD64_REGALLOC_CLASS_REGISTERS];
    kefir_size_t num_of_sse_registers;
} kefir_codegen_target_ir_amd64_regalloc_class_t;

kefir_result_t kefir_codegen_target_ir_amd64_regalloc_class_init(struct kefir_mem *, struct kefir_codegen_target_ir_amd64_regalloc_class *, kefir_abi_amd64_variant_t);
kefir_result_t kefir_codegen_target_ir_amd64_regalloc_format_allocation(struct kefir_json_output *, kefir_codegen_target_ir_regalloc_allocation_t);

#endif