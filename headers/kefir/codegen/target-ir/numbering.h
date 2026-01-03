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

#ifndef KEFIR_CODEGEN_TARGET_IR_NUMBERING_H_
#define KEFIR_CODEGEN_TARGET_IR_NUMBERING_H_

#include "kefir/codegen/target-ir/code.h"

typedef struct kefir_codegen_target_ir_numbering {
    kefir_uint32_t *block_lengths;
    kefir_uint32_t *instruction_seq_nums;
    kefir_size_t block_count;
    kefir_size_t length;
} kefir_codegen_target_ir_numbering_t;

kefir_result_t kefir_codegen_target_ir_numbering_init(struct kefir_codegen_target_ir_numbering *);
kefir_result_t kefir_codegen_target_ir_numbering_free(struct kefir_mem *, struct kefir_codegen_target_ir_numbering *);
kefir_result_t kefir_codegen_target_ir_numbering_reset(struct kefir_mem *, struct kefir_codegen_target_ir_numbering *);

kefir_result_t kefir_codegen_target_ir_numbering_build(struct kefir_mem *, struct kefir_codegen_target_ir_numbering *, const struct kefir_codegen_target_ir_code *);

kefir_result_t kefir_codegen_target_ir_numbering_block_length(const struct kefir_codegen_target_ir_numbering *, kefir_codegen_target_ir_block_ref_t, kefir_size_t *);
kefir_result_t kefir_codegen_target_ir_numbering_instruction_seq_index(const struct kefir_codegen_target_ir_numbering *, kefir_codegen_target_ir_instruction_ref_t, kefir_size_t *);

#endif
