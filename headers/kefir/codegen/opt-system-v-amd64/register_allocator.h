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

#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_REGISTER_ALLOCATOR_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_REGISTER_ALLOCATOR_H_

#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/target/abi/system-v-amd64/function.h"
#include "kefir/codegen/opt-system-v-amd64/parameters.h"
#include "kefir/codegen/opt-system-v-amd64/stack_frame.h"
#include "kefir/optimizer/module.h"
#include "kefir/optimizer/analysis.h"
#include "kefir/core/bitset.h"
#include "kefir/core/graph.h"

extern const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64GeneralPurposeRegisters[];
extern const kefir_asm_amd64_xasmgen_register_t KefirOptSysvAmd64FloatingPointRegisters[];
extern const kefir_size_t KefirOptSysvAmd64NumOfGeneralPurposeRegisters;
extern const kefir_size_t KefirOptSysvAmd64NumOfFloatingPointRegisters;

typedef enum kefir_codegen_opt_sysv_amd64_register_allocation_class {
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_CLASS_SKIP,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_CLASS_GENERAL_PURPOSE,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_CLASS_FLOATING_POINT
} kefir_codegen_opt_sysv_amd64_register_allocation_class_t;

typedef enum kefir_codegen_opt_sysv_amd64_register_allocation_type {
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_NONE,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_SPILL_AREA,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_PARAMETER_REGISTER_AGGREGATE,
    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_INDIRECT
} kefir_codegen_opt_sysv_amd64_register_allocation_type_t;

typedef struct kefir_codegen_opt_sysv_amd64_register_allocation {
    kefir_codegen_opt_sysv_amd64_register_allocation_class_t klass;

    struct {
        kefir_codegen_opt_sysv_amd64_register_allocation_type_t type;

        union {
            kefir_asm_amd64_xasmgen_register_t reg;
            kefir_size_t spill_index;
            struct {
                kefir_asm_amd64_xasmgen_register_t base_register;
                kefir_int64_t offset;
            } indirect;
            struct {
                kefir_size_t index;
                const struct kefir_abi_sysv_amd64_parameter_allocation *allocation;
            } register_aggregate;
        };
    } result;

    struct {
        kefir_bool_t present;
        kefir_asm_amd64_xasmgen_register_t hint;
    } register_hint;

    struct {
        kefir_bool_t present;
        kefir_opt_instruction_ref_t instr_ref;
    } alias_hint;
} kefir_codegen_opt_sysv_amd64_register_allocation_t;

typedef struct kefir_codegen_opt_sysv_amd64_register_allocator {
    struct kefir_bitset general_purpose_regs;
    struct kefir_bitset floating_point_regs;
    struct kefir_bitset spilled_regs;
    struct kefir_graph allocation;
} kefir_codegen_opt_sysv_amd64_register_allocator_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation(
    struct kefir_mem *, const struct kefir_opt_function *, const struct kefir_opt_code_analysis *,
    const struct kefir_codegen_opt_amd64_sysv_function_parameters *, struct kefir_codegen_opt_sysv_amd64_stack_frame *,
    struct kefir_codegen_opt_sysv_amd64_register_allocator *);

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation_of(
    const struct kefir_codegen_opt_sysv_amd64_register_allocator *, kefir_opt_instruction_ref_t,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation **);

kefir_result_t kefir_codegen_opt_sysv_amd64_register_allocation_free(
    struct kefir_mem *, struct kefir_codegen_opt_sysv_amd64_register_allocator *);

#endif
