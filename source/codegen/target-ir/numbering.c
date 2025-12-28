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

#include "kefir/codegen/target-ir/numbering.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_target_ir_numbering_init(struct kefir_codegen_target_ir_numbering *numbering) {
    REQUIRE(numbering != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR numbering"));

    numbering->instruction_seq_nums = NULL;
    numbering->length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_numbering_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_numbering *numbering) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(numbering != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR numbering"));

    KEFIR_FREE(mem, numbering->instruction_seq_nums);
    numbering->instruction_seq_nums = NULL;
    numbering->length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_numbering_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_numbering *numbering,
    const struct kefir_codegen_target_ir_code *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(numbering != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR numbering"));
    REQUIRE(numbering->instruction_seq_nums == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid clean target IR numbering"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));
    
    numbering->instruction_seq_nums = KEFIR_MALLOC(mem, sizeof(kefir_uint32_t) * code->code_length);
    REQUIRE(numbering->instruction_seq_nums != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR numbering"));
    numbering->length = code->code_length;

    numbering->block_lengths = KEFIR_MALLOC(mem, sizeof(kefir_uint32_t) * kefir_codegen_target_ir_code_block_count(code));
    REQUIRE(numbering->block_lengths != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR numbering"));
    numbering->block_count = kefir_codegen_target_ir_code_block_count(code);

    for (kefir_size_t i = 0; i < numbering->block_count; i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);

        kefir_size_t local_index = 0;
        for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(code, block_ref);
            instr_ref != KEFIR_ID_NONE;
            instr_ref = kefir_codegen_target_ir_code_control_next(code, instr_ref), local_index++) {
            numbering->instruction_seq_nums[instr_ref] = local_index;
        }

        numbering->block_lengths[block_ref] = local_index;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_numbering_block_length(const struct kefir_codegen_target_ir_numbering *numbering,
    kefir_codegen_target_ir_block_ref_t block_ref, kefir_size_t *length_ptr) {
    REQUIRE(numbering != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR numbering"));
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < numbering->block_count, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Block reference is out of numbering bounds"));
    REQUIRE(length_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to length"));

    *length_ptr = numbering->block_lengths[block_ref];
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_numbering_instruction_seq_index(const struct kefir_codegen_target_ir_numbering *numbering,
    kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_size_t *index_ptr) {
    REQUIRE(numbering != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR numbering"));
    REQUIRE(instr_ref != KEFIR_ID_NONE && instr_ref < numbering->length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Instruction reference is out of numbering bounds"));
    REQUIRE(index_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to index"));

    *index_ptr = numbering->instruction_seq_nums[instr_ref];
    return KEFIR_OK;
}
