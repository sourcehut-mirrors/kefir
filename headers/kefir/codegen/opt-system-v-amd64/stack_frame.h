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

#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_STACK_FRAME_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_STACK_FRAME_H_

#include "kefir/core/bitset.h"
#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/target/abi/system-v-amd64/data_layout.h"

extern const kefir_asm_amd64_xasmgen_register_t KefirCodegenOptSysvAmd64StackFramePreservedRegs[];
enum { KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs = 5 };

typedef struct kefir_codegen_opt_sysv_amd64_stack_frame {
    kefir_uint64_t preserved_regs_content[KEFIR_BITSET_STATIC_CONTENT_CAPACITY(
        KefirCodegenOptSysvAmd64StackFrameNumOfPreservedRegs)];
    struct {
        struct kefir_bitset regs;
        kefir_bool_t x87_control_word;
        kefir_bool_t mxcsr_register;
        kefir_bool_t implicit_parameter;
    } preserve;
    kefir_size_t preserve_area_size;
    kefir_size_t spill_area_size;
    kefir_size_t register_aggregate_area_size;
    kefir_size_t local_area_size;
    kefir_size_t local_area_alignment;
} kefir_codegen_opt_sysv_amd64_stack_frame_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_init(struct kefir_codegen_opt_sysv_amd64_stack_frame *);
kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_preserve_register(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *, kefir_asm_amd64_xasmgen_register_t);
kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_preserve_x87cw(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *);
kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_preserve_mxcsr(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *);
kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_preserve_implicit_parameter(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *);
kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_ensure_spill(struct kefir_codegen_opt_sysv_amd64_stack_frame *,
                                                                     kefir_size_t);
kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_allocate_register_aggregate(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *, kefir_size_t, kefir_size_t *);
kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_allocate_set_locals(
    struct kefir_codegen_opt_sysv_amd64_stack_frame *, const struct kefir_ir_type *,
    const struct kefir_abi_sysv_amd64_type_layout *);

typedef struct kefir_codegen_opt_sysv_amd64_stack_frame_map {
    struct {
        kefir_int64_t previous_base;
        kefir_int64_t preserved_general_purpose_registers;
        kefir_int64_t x87_control_word;
        kefir_int64_t mxcsr;
        kefir_int64_t implicit_parameter;
        kefir_int64_t spill_area;
        kefir_int64_t register_aggregate_area;
        kefir_int64_t local_area;
    } offset;
} kefir_codegen_opt_sysv_amd64_stack_frame_map_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_compute(const struct kefir_codegen_opt_sysv_amd64_stack_frame *,
                                                                struct kefir_codegen_opt_sysv_amd64_stack_frame_map *);

kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_prologue(
    const struct kefir_codegen_opt_sysv_amd64_stack_frame *, struct kefir_amd64_xasmgen *);
kefir_result_t kefir_codegen_opt_sysv_amd64_stack_frame_epilogue(
    const struct kefir_codegen_opt_sysv_amd64_stack_frame *, struct kefir_amd64_xasmgen *);

#endif
