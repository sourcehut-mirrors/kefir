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

#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_code_builder_add_instruction(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id,
                                                      const struct kefir_opt_operation *operation, kefir_bool_t control,
                                                      kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    const struct kefir_opt_instruction *prev_instr = NULL;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, block, &prev_instr));
    if (prev_instr != NULL) {
        switch (prev_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_JUMP:
            case KEFIR_OPT_OPCODE_IJUMP:
            case KEFIR_OPT_OPCODE_BRANCH:
            case KEFIR_OPT_OPCODE_RETURN:
                return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer code block has already been finalized");

            default:
                // Intentionally left blank
                break;
        }
    }

    kefir_opt_instruction_ref_t instr_id;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(mem, code, block, operation, &instr_id));
    if (control) {
        REQUIRE_OK(kefir_opt_code_container_add_control(code, block, instr_id));
    }

    ASSIGN_PTR(instr_id_ptr, instr_id);
    return KEFIR_OK;
}

static kefir_result_t block_exists(const struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id) {
    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));
    return KEFIR_OK;
}

static kefir_result_t instr_exists(const struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
                                   kefir_opt_instruction_ref_t instr_id, kefir_bool_t permit_none) {
    REQUIRE(permit_none || instr_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Optimized code instruction with requested identifier is not found"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_id, &instr));
    REQUIRE(instr->block_id == block_id || block_id == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimized code instruction does not belong to specified block"));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_finalize_jump(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                    kefir_opt_block_id_t block_id, kefir_opt_block_id_t target_block,
                                                    kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(block_exists(code, target_block));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_JUMP, .parameters.branch.target_block = target_block},
        true, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_finalize_indirect_jump(struct kefir_mem *mem,
                                                             struct kefir_opt_code_container *code,
                                                             kefir_opt_block_id_t block_id,
                                                             kefir_opt_instruction_ref_t arg_instr_id,
                                                             kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, arg_instr_id, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_IJUMP, .parameters.refs[0] = arg_instr_id}, true,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_finalize_branch(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id,
                                                      kefir_opt_instruction_ref_t condition,
                                                      kefir_opt_block_id_t target_block,
                                                      kefir_opt_block_id_t alternative_block,
                                                      kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, condition, false));
    REQUIRE_OK(block_exists(code, target_block));
    REQUIRE_OK(block_exists(code, alternative_block));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_BRANCH,
                                      .parameters.branch = {.target_block = target_block,
                                                            .alternative_block = alternative_block,
                                                            .condition_ref = condition}},
        true, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_finalize_return(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id,
                                                      kefir_opt_instruction_ref_t arg_instr_id,
                                                      kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, arg_instr_id, true));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_RETURN, .parameters.refs[0] = arg_instr_id}, true,
        instr_id_ptr));
    return KEFIR_OK;
}
