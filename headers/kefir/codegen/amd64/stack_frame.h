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

#ifndef KEFIR_CODEGEN_AMD64_STACK_FRAME_H_
#define KEFIR_CODEGEN_AMD64_STACK_FRAME_H_

#include "kefir/codegen/amd64/codegen.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/target/asm/amd64/xasmgen.h"

typedef struct kefir_codegen_amd64_stack_frame {
    struct {
        kefir_size_t preserved_regs;
        kefir_size_t local_area;
        kefir_size_t local_area_alignment;
        kefir_size_t spill_area;
        kefir_size_t temporary_area;
        kefir_size_t temporary_area_alignment;
        kefir_size_t vararg_area;
        kefir_size_t vararg_area_alignment;
        kefir_size_t allocated_size;
        kefir_size_t total_size;
    } sizes;

    struct {
        kefir_int64_t previous_base;
        kefir_int64_t preserved_regs;
        kefir_int64_t x87_control_word;
        kefir_int64_t mxcsr;
        kefir_int64_t local_area;
        kefir_int64_t spill_area;
        kefir_int64_t temporary_area;
        kefir_int64_t vararg_area;
        kefir_int64_t top_of_frame;
    } offsets;

    struct {
        kefir_size_t spill_area_slots;
        struct kefir_hashtreeset used_registers;
        kefir_size_t num_of_used_registers;
        kefir_size_t temporary_area_size;
        kefir_size_t temporary_area_alignment;
        kefir_bool_t reset_stack_pointer;
        kefir_bool_t vararg;
        kefir_bool_t x87_control_word_save;
        kefir_bool_t mxcsr_save;
        kefir_bool_t frame_pointer;
    } requirements;
} kefir_codegen_amd64_stack_frame_t;

kefir_result_t kefir_codegen_amd64_stack_frame_init(struct kefir_codegen_amd64_stack_frame *);
kefir_result_t kefir_codegen_amd64_stack_frame_free(struct kefir_mem *, struct kefir_codegen_amd64_stack_frame *);

kefir_result_t kefir_codegen_amd64_stack_frame_ensure_spill_area(struct kefir_codegen_amd64_stack_frame *,
                                                                 kefir_size_t);
kefir_result_t kefir_codegen_amd64_stack_frame_ensure_temporary_area(struct kefir_codegen_amd64_stack_frame *,
                                                                     kefir_size_t, kefir_size_t);
kefir_result_t kefir_codegen_amd64_stack_frame_use_register(struct kefir_mem *,
                                                            struct kefir_codegen_amd64_stack_frame *,
                                                            kefir_asm_amd64_xasmgen_register_t);
kefir_result_t kefir_codegen_amd64_stack_frame_varying_stack_pointer(struct kefir_codegen_amd64_stack_frame *);
kefir_result_t kefir_codegen_amd64_stack_frame_vararg(struct kefir_codegen_amd64_stack_frame *);
kefir_result_t kefir_codegen_amd64_stack_frame_preserve_x87_control_word(struct kefir_codegen_amd64_stack_frame *);
kefir_result_t kefir_codegen_amd64_stack_frame_preserve_mxcsr(struct kefir_codegen_amd64_stack_frame *);
kefir_result_t kefir_codegen_amd64_stack_frame_require_frame_pointer(struct kefir_codegen_amd64_stack_frame *);

kefir_result_t kefir_codegen_amd64_stack_frame_calculate(kefir_abi_amd64_variant_t, const struct kefir_ir_type *,
                                                         const struct kefir_abi_amd64_type_layout *,
                                                         struct kefir_codegen_amd64_stack_frame *);

kefir_result_t kefir_codegen_amd64_stack_frame_prologue(struct kefir_amd64_xasmgen *, kefir_abi_amd64_variant_t,
                                                        kefir_bool_t, const struct kefir_codegen_amd64_stack_frame *);
kefir_result_t kefir_codegen_amd64_stack_frame_epilogue(struct kefir_amd64_xasmgen *, kefir_abi_amd64_variant_t,
                                                        const struct kefir_codegen_amd64_stack_frame *);

#endif
