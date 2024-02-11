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

    kefir_bool_t finalized = false;
    REQUIRE_OK(kefir_opt_code_builder_is_finalized(code, block_id, &finalized));
    REQUIRE(!control || !finalized,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer code block has already been finalized"));

    kefir_opt_instruction_ref_t instr_id;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(mem, code, block, operation, &instr_id));
    if (control) {
        REQUIRE_OK(kefir_opt_code_container_add_control(code, block, instr_id));
    }

    ASSIGN_PTR(instr_id_ptr, instr_id);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_add_control(struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
                                                  kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));
    REQUIRE_OK(kefir_opt_code_container_add_control(code, block, instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_is_finalized(const struct kefir_opt_code_container *code,
                                                   kefir_opt_block_id_t block_id, kefir_bool_t *finalized_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(finalized_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    struct kefir_opt_instruction *prev_instr = NULL;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, block, &prev_instr));
    if (prev_instr != NULL) {
        switch (prev_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_JUMP:
            case KEFIR_OPT_OPCODE_IJUMP:
            case KEFIR_OPT_OPCODE_BRANCH:
            case KEFIR_OPT_OPCODE_COMPARE_BRANCH:
            case KEFIR_OPT_OPCODE_RETURN:
                *finalized_ptr = true;
                break;

            case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
                struct kefir_opt_inline_assembly_node *inline_asm = NULL;
                REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                    code, prev_instr->operation.parameters.inline_asm_ref, &inline_asm));
                *finalized_ptr = !kefir_hashtree_empty(&inline_asm->jump_targets);
            } break;

            default:
                *finalized_ptr = false;
                break;
        }
    }

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

    if (!permit_none) {
        struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_id, &instr));
        REQUIRE(
            instr->block_id == block_id || block_id == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimized code instruction does not belong to specified block"));
    }
    return KEFIR_OK;
}

static kefir_result_t phi_exists(const struct kefir_opt_code_container *code, kefir_opt_phi_id_t phi_ref) {
    struct kefir_opt_phi_node *phi = NULL;
    REQUIRE_OK(kefir_opt_code_container_phi(code, phi_ref, &phi));
    return KEFIR_OK;
}

static kefir_result_t call_exists(const struct kefir_opt_code_container *code, kefir_opt_call_id_t call_ref) {
    struct kefir_opt_call_node *call = NULL;
    REQUIRE_OK(kefir_opt_code_container_call(code, call_ref, &call));
    return KEFIR_OK;
}

static kefir_result_t inline_asm_exists(const struct kefir_opt_code_container *code,
                                        kefir_opt_inline_assembly_id_t inline_asm_ref) {
    struct kefir_opt_inline_assembly_node *inline_asm = NULL;
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, inline_asm_ref, &inline_asm));
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
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_JUMP,
                                      .parameters.branch = {.target_block = target_block,
                                                            .alternative_block = KEFIR_ID_NONE,
                                                            .condition_ref = KEFIR_ID_NONE}},
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

kefir_result_t kefir_opt_code_builder_compare_branch(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                     kefir_opt_block_id_t block_id,
                                                     kefir_opt_compare_branch_type_t comparison_type,
                                                     kefir_opt_instruction_ref_t ref1, kefir_opt_instruction_ref_t ref2,
                                                     kefir_opt_block_id_t target_block,
                                                     kefir_opt_block_id_t alternative_block,
                                                     kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    switch (comparison_type) {
        case KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER:
        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE:
        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW:
        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_NOT_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT32_LESS_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_NOT_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS:
        case KEFIR_OPT_COMPARE_BRANCH_FLOAT64_LESS_OR_EQUALS:
            // Intentionally left blank
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected comparison type");
    }

    REQUIRE_OK(instr_exists(code, block_id, ref1, false));
    REQUIRE_OK(instr_exists(code, block_id, ref2, false));
    REQUIRE_OK(block_exists(code, target_block));
    REQUIRE_OK(block_exists(code, alternative_block));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){
            .opcode = KEFIR_OPT_OPCODE_COMPARE_BRANCH,
            .parameters.branch = {.target_block = target_block,
                                  .alternative_block = alternative_block,
                                  .comparison = {.type = comparison_type, .refs = {ref1, ref2}}}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_compare_branch_int_const(
    struct kefir_mem *mem, struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
    kefir_opt_compare_branch_type_t comparison_type, kefir_opt_instruction_ref_t ref1, kefir_int64_t imm,
    kefir_opt_block_id_t target_block, kefir_opt_block_id_t alternative_block,
    kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    switch (comparison_type) {
        case KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_CONST:
        case KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS_CONST:
            // Intentionally left blank
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected comparison type");
    }

    REQUIRE_OK(instr_exists(code, block_id, ref1, false));
    REQUIRE_OK(block_exists(code, target_block));
    REQUIRE_OK(block_exists(code, alternative_block));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){
            .opcode = KEFIR_OPT_OPCODE_COMPARE_BRANCH,
            .parameters.branch = {.target_block = target_block,
                                  .alternative_block = alternative_block,
                                  .comparison = {.type = comparison_type, .refs = {ref1}, .integer = imm}}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_get_argument(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                   kefir_opt_block_id_t block_id, kefir_size_t index,
                                                   kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_GET_ARGUMENT, .parameters.index = index}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_phi(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                          kefir_opt_block_id_t block_id, kefir_opt_phi_id_t identifier,
                                          kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(phi_exists(code, identifier));

    kefir_opt_instruction_ref_t instr_ref;
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_PHI, .parameters.phi_ref = identifier}, false,
        &instr_ref));
    REQUIRE_OK(kefir_opt_code_container_phi_set_output(code, identifier, instr_ref));
    ASSIGN_PTR(instr_id_ptr, instr_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_inline_assembly(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id,
                                                      kefir_opt_inline_assembly_id_t identifier,
                                                      kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(inline_asm_exists(code, identifier));

    REQUIRE_OK(
        kefir_opt_code_builder_add_instruction(mem, code, block_id,
                                               &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_INLINE_ASSEMBLY,
                                                                             .parameters.inline_asm_ref = identifier},
                                               false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_invoke(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                             kefir_opt_block_id_t block_id, kefir_opt_call_id_t call_ref,
                                             kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(call_exists(code, call_ref));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){
            .opcode = KEFIR_OPT_OPCODE_INVOKE,
            .parameters.function_call = {.call_ref = call_ref, .indirect_ref = KEFIR_ID_NONE}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_invoke_virtual(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                     kefir_opt_block_id_t block_id, kefir_opt_call_id_t call_ref,
                                                     kefir_opt_instruction_ref_t instr_ref,
                                                     kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(call_exists(code, call_ref));
    REQUIRE_OK(instr_exists(code, block_id, instr_ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_INVOKE_VIRTUAL,
                                      .parameters.function_call = {.call_ref = call_ref, .indirect_ref = instr_ref}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_int_constant(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                   kefir_opt_block_id_t block_id, kefir_int64_t value,
                                                   kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_INT_CONST, .parameters.imm.integer = value}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_uint_constant(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                    kefir_opt_block_id_t block_id, kefir_uint64_t value,
                                                    kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_UINT_CONST, .parameters.imm.uinteger = value}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_float32_constant(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t block_id, kefir_float32_t value,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_FLOAT32_CONST, .parameters.imm.float32 = value}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_float64_constant(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t block_id, kefir_float64_t value,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_FLOAT64_CONST, .parameters.imm.float64 = value}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_long_double_constant(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                           kefir_opt_block_id_t block_id, kefir_long_double_t value,
                                                           kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST,
                                      .parameters.imm.long_double = value},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_string_reference(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t block_id, kefir_id_t ref,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_STRING_REF, .parameters.imm.string_ref = ref}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_block_label(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_opt_block_id_t block_id, kefir_opt_block_id_t ref,
                                                  kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(block_exists(code, ref));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_BLOCK_LABEL, .parameters.imm.block_ref = ref}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_int_placeholder(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id,
                                                      kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_INT_PLACEHOLDER}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_float32_placeholder(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                          kefir_opt_block_id_t block_id,
                                                          kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_FLOAT32_PLACEHOLDER}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_float64_placeholder(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                          kefir_opt_block_id_t block_id,
                                                          kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_FLOAT64_PLACEHOLDER}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_get_global(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_id, kefir_id_t identifier,
                                                 kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_GET_GLOBAL, .parameters.ir_ref = identifier}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_get_thread_local(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t block_id, kefir_id_t identifier,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_GET_THREAD_LOCAL, .parameters.ir_ref = identifier},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_get_local(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                kefir_opt_block_id_t block_id, kefir_size_t index, kefir_int64_t offset,
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_GET_LOCAL,
                                      .parameters.local_var = {.index = index, .offset = offset}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_zero_memory(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_opt_block_id_t block_id,
                                                  kefir_opt_instruction_ref_t location_ref, kefir_id_t type_id,
                                                  kefir_size_t type_index, kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, location_ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_ZERO_MEMORY,
                                      .parameters.typed_refs = {.ref = {location_ref, KEFIR_ID_NONE},
                                                                .type_id = type_id,
                                                                .type_index = type_index}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_copy_memory(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_opt_block_id_t block_id,
                                                  kefir_opt_instruction_ref_t location_ref,
                                                  kefir_opt_instruction_ref_t source_ref, kefir_id_t type_id,
                                                  kefir_size_t type_index, kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, location_ref, false));
    REQUIRE_OK(instr_exists(code, block_id, source_ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){
            .opcode = KEFIR_OPT_OPCODE_COPY_MEMORY,
            .parameters.typed_refs = {.ref = {location_ref, source_ref}, .type_id = type_id, .type_index = type_index}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_bits_extract_signed(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                          kefir_opt_block_id_t block_id,
                                                          kefir_opt_instruction_ref_t base_ref, kefir_size_t offset,
                                                          kefir_size_t length,
                                                          kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, base_ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED,
                                      .parameters.bitfield = {.base_ref = base_ref,
                                                              .value_ref = KEFIR_ID_NONE,
                                                              .offset = offset,
                                                              .length = length}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_bits_extract_unsigned(struct kefir_mem *mem,
                                                            struct kefir_opt_code_container *code,
                                                            kefir_opt_block_id_t block_id,
                                                            kefir_opt_instruction_ref_t base_ref, kefir_size_t offset,
                                                            kefir_size_t length,
                                                            kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, base_ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED,
                                      .parameters.bitfield = {.base_ref = base_ref,
                                                              .value_ref = KEFIR_ID_NONE,
                                                              .offset = offset,
                                                              .length = length}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_bits_insert(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t base_ref,
                                                  kefir_opt_instruction_ref_t value_ref, kefir_size_t offset,
                                                  kefir_size_t length, kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, base_ref, false));
    REQUIRE_OK(instr_exists(code, block_id, value_ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){
            .opcode = KEFIR_OPT_OPCODE_BITS_INSERT,
            .parameters.bitfield = {.base_ref = base_ref, .value_ref = value_ref, .offset = offset, .length = length}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_vararg_get(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t source_ref,
                                                 kefir_id_t type_id, kefir_size_t type_index,
                                                 kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, source_ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){
            .opcode = KEFIR_OPT_OPCODE_VARARG_GET,
            .parameters.typed_refs = {.ref = {source_ref}, .type_id = type_id, .type_index = type_index}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_stack_alloc(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t size_ref,
                                                  kefir_opt_instruction_ref_t alignment_ref, kefir_bool_t within_scope,
                                                  kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, size_ref, false));
    REQUIRE_OK(instr_exists(code, block_id, alignment_ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_STACK_ALLOC,
                                      .parameters.stack_allocation = {.size_ref = size_ref,
                                                                      .alignment_ref = alignment_ref,
                                                                      .within_scope = within_scope}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_scope_push(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_id,
                                                 kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_SCOPE_PUSH}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

#define UNARY_OP(_id, _opcode)                                                                                         \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,          \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,       \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                           \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));             \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));    \
        REQUIRE_OK(instr_exists(code, block_id, ref1, false));                                                         \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                             \
            mem, code, block_id, &(struct kefir_opt_operation){.opcode = (_opcode), .parameters.refs = {ref1}}, false, \
            instr_id_ptr));                                                                                            \
        return KEFIR_OK;                                                                                               \
    }

UNARY_OP(int_not, KEFIR_OPT_OPCODE_INT_NOT)
UNARY_OP(int_neg, KEFIR_OPT_OPCODE_INT_NEG)
UNARY_OP(bool_not, KEFIR_OPT_OPCODE_BOOL_NOT)

UNARY_OP(int64_truncate_1bit, KEFIR_OPT_OPCODE_INT64_TRUNCATE_1BIT)
UNARY_OP(int64_zero_extend_8bits, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS)
UNARY_OP(int64_zero_extend_16bits, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS)
UNARY_OP(int64_zero_extend_32bits, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)
UNARY_OP(int64_sign_extend_8bits, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS)
UNARY_OP(int64_sign_extend_16bits, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS)
UNARY_OP(int64_sign_extend_32bits, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)

UNARY_OP(vararg_start, KEFIR_OPT_OPCODE_VARARG_START)
UNARY_OP(vararg_end, KEFIR_OPT_OPCODE_VARARG_END)
UNARY_OP(scope_pop, KEFIR_OPT_OPCODE_SCOPE_POP)

UNARY_OP(float32_neg, KEFIR_OPT_OPCODE_FLOAT32_NEG)
UNARY_OP(float64_neg, KEFIR_OPT_OPCODE_FLOAT64_NEG)
UNARY_OP(long_double_neg, KEFIR_OPT_OPCODE_LONG_DOUBLE_NEG)

UNARY_OP(float32_to_int, KEFIR_OPT_OPCODE_FLOAT32_TO_INT)
UNARY_OP(float64_to_int, KEFIR_OPT_OPCODE_FLOAT64_TO_INT)
UNARY_OP(float32_to_uint, KEFIR_OPT_OPCODE_FLOAT32_TO_UINT)
UNARY_OP(float64_to_uint, KEFIR_OPT_OPCODE_FLOAT64_TO_UINT)
UNARY_OP(int_to_float32, KEFIR_OPT_OPCODE_INT_TO_FLOAT32)
UNARY_OP(int_to_float64, KEFIR_OPT_OPCODE_INT_TO_FLOAT64)
UNARY_OP(uint_to_float32, KEFIR_OPT_OPCODE_UINT_TO_FLOAT32)
UNARY_OP(uint_to_float64, KEFIR_OPT_OPCODE_UINT_TO_FLOAT64)
UNARY_OP(float32_to_float64, KEFIR_OPT_OPCODE_FLOAT32_TO_FLOAT64)
UNARY_OP(float64_to_float32, KEFIR_OPT_OPCODE_FLOAT64_TO_FLOAT32)
UNARY_OP(long_double_to_int, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_INT)
UNARY_OP(long_double_to_uint, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_UINT)
UNARY_OP(long_double_to_float32, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT32)
UNARY_OP(long_double_to_float64, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT64)

UNARY_OP(int_to_long_double, KEFIR_OPT_OPCODE_INT_TO_LONG_DOUBLE)
UNARY_OP(uint_to_long_double, KEFIR_OPT_OPCODE_UINT_TO_LONG_DOUBLE)
UNARY_OP(float32_to_long_double, KEFIR_OPT_OPCODE_FLOAT32_TO_LONG_DOUBLE)
UNARY_OP(float64_to_long_double, KEFIR_OPT_OPCODE_FLOAT64_TO_LONG_DOUBLE)

UNARY_OP(complex_float32_real, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_REAL)
UNARY_OP(complex_float32_imaginary, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_IMAGINARY)
UNARY_OP(complex_float64_real, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_REAL)
UNARY_OP(complex_float64_imaginary, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_IMAGINARY)
UNARY_OP(complex_long_double_real, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_REAL)
UNARY_OP(complex_long_double_imaginary, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_IMAGINARY)

UNARY_OP(complex_float32_truncate_1bit, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_TRUNCATE_1BIT)
UNARY_OP(complex_float64_truncate_1bit, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_TRUNCATE_1BIT)
UNARY_OP(complex_long_double_truncate_1bit, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT)

UNARY_OP(complex_float32_neg, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_NEG)
UNARY_OP(complex_float64_neg, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_NEG)
UNARY_OP(complex_long_double_neg, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_NEG)

#undef UNARY_OP

#define BINARY_OP(_id, _opcode)                                                                                       \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,         \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,      \
                                                kefir_opt_instruction_ref_t ref2,                                     \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                          \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));            \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));   \
        REQUIRE_OK(instr_exists(code, block_id, ref1, false));                                                        \
        REQUIRE_OK(instr_exists(code, block_id, ref2, false));                                                        \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                            \
            mem, code, block_id, &(struct kefir_opt_operation){.opcode = (_opcode), .parameters.refs = {ref1, ref2}}, \
            false, instr_id_ptr));                                                                                    \
        return KEFIR_OK;                                                                                              \
    }

BINARY_OP(int_add, KEFIR_OPT_OPCODE_INT_ADD)
BINARY_OP(int_sub, KEFIR_OPT_OPCODE_INT_SUB)
BINARY_OP(int_mul, KEFIR_OPT_OPCODE_INT_MUL)
BINARY_OP(int_div, KEFIR_OPT_OPCODE_INT_DIV)
BINARY_OP(int_mod, KEFIR_OPT_OPCODE_INT_MOD)
BINARY_OP(uint_div, KEFIR_OPT_OPCODE_UINT_DIV)
BINARY_OP(uint_mod, KEFIR_OPT_OPCODE_UINT_MOD)
BINARY_OP(int_and, KEFIR_OPT_OPCODE_INT_AND)
BINARY_OP(int_or, KEFIR_OPT_OPCODE_INT_OR)
BINARY_OP(int_xor, KEFIR_OPT_OPCODE_INT_XOR)
BINARY_OP(int_lshift, KEFIR_OPT_OPCODE_INT_LSHIFT)
BINARY_OP(int_rshift, KEFIR_OPT_OPCODE_INT_RSHIFT)
BINARY_OP(int_arshift, KEFIR_OPT_OPCODE_INT_ARSHIFT)
BINARY_OP(int_equals, KEFIR_OPT_OPCODE_INT_EQUALS)
BINARY_OP(int_greater, KEFIR_OPT_OPCODE_INT_GREATER)
BINARY_OP(int_greater_or_equals, KEFIR_OPT_OPCODE_INT_GREATER_OR_EQUALS)
BINARY_OP(int_lesser, KEFIR_OPT_OPCODE_INT_LESSER)
BINARY_OP(int_lesser_or_equals, KEFIR_OPT_OPCODE_INT_LESSER_OR_EQUALS)
BINARY_OP(int_above, KEFIR_OPT_OPCODE_INT_ABOVE)
BINARY_OP(int_above_or_equals, KEFIR_OPT_OPCODE_INT_ABOVE_OR_EQUALS)
BINARY_OP(int_below, KEFIR_OPT_OPCODE_INT_BELOW)
BINARY_OP(int_below_or_equals, KEFIR_OPT_OPCODE_INT_BELOW_OR_EQUALS)
BINARY_OP(bool_and, KEFIR_OPT_OPCODE_BOOL_AND)
BINARY_OP(bool_or, KEFIR_OPT_OPCODE_BOOL_OR)

BINARY_OP(vararg_copy, KEFIR_OPT_OPCODE_VARARG_COPY)

BINARY_OP(float32_add, KEFIR_OPT_OPCODE_FLOAT32_ADD)
BINARY_OP(float32_sub, KEFIR_OPT_OPCODE_FLOAT32_SUB)
BINARY_OP(float32_mul, KEFIR_OPT_OPCODE_FLOAT32_MUL)
BINARY_OP(float32_div, KEFIR_OPT_OPCODE_FLOAT32_DIV)
BINARY_OP(float64_add, KEFIR_OPT_OPCODE_FLOAT64_ADD)
BINARY_OP(float64_sub, KEFIR_OPT_OPCODE_FLOAT64_SUB)
BINARY_OP(float64_mul, KEFIR_OPT_OPCODE_FLOAT64_MUL)
BINARY_OP(float64_div, KEFIR_OPT_OPCODE_FLOAT64_DIV)

BINARY_OP(float32_equals, KEFIR_OPT_OPCODE_FLOAT32_EQUALS)
BINARY_OP(float32_greater, KEFIR_OPT_OPCODE_FLOAT32_GREATER)
BINARY_OP(float32_greater_or_equals, KEFIR_OPT_OPCODE_FLOAT32_GREATER_OR_EQUALS)
BINARY_OP(float32_lesser, KEFIR_OPT_OPCODE_FLOAT32_LESSER)
BINARY_OP(float32_lesser_or_equals, KEFIR_OPT_OPCODE_FLOAT32_LESSER_OR_EQUALS)
BINARY_OP(float64_equals, KEFIR_OPT_OPCODE_FLOAT64_EQUALS)
BINARY_OP(float64_greater, KEFIR_OPT_OPCODE_FLOAT64_GREATER)
BINARY_OP(float64_greater_or_equals, KEFIR_OPT_OPCODE_FLOAT64_GREATER_OR_EQUALS)
BINARY_OP(float64_lesser, KEFIR_OPT_OPCODE_FLOAT64_LESSER)
BINARY_OP(float64_lesser_or_equals, KEFIR_OPT_OPCODE_FLOAT64_LESSER_OR_EQUALS)
BINARY_OP(long_double_equals, KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS)
BINARY_OP(long_double_greater, KEFIR_OPT_OPCODE_LONG_DOUBLE_GREATER)
BINARY_OP(long_double_lesser, KEFIR_OPT_OPCODE_LONG_DOUBLE_LESSER)

BINARY_OP(long_double_add, KEFIR_OPT_OPCODE_LONG_DOUBLE_ADD)
BINARY_OP(long_double_sub, KEFIR_OPT_OPCODE_LONG_DOUBLE_SUB)
BINARY_OP(long_double_mul, KEFIR_OPT_OPCODE_LONG_DOUBLE_MUL)
BINARY_OP(long_double_div, KEFIR_OPT_OPCODE_LONG_DOUBLE_DIV)

BINARY_OP(complex_float32_from, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_FROM)
BINARY_OP(complex_float64_from, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_FROM)
BINARY_OP(complex_long_double_from, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_FROM)

BINARY_OP(complex_float32_equals, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_EQUALS)
BINARY_OP(complex_float64_equals, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_EQUALS)
BINARY_OP(complex_long_double_equals, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_EQUALS)

BINARY_OP(complex_float32_add, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_ADD)
BINARY_OP(complex_float64_add, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_ADD)
BINARY_OP(complex_long_double_add, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_ADD)
BINARY_OP(complex_float32_sub, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_SUB)
BINARY_OP(complex_float64_sub, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_SUB)
BINARY_OP(complex_long_double_sub, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_SUB)
BINARY_OP(complex_float32_mul, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_MUL)
BINARY_OP(complex_float64_mul, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_MUL)
BINARY_OP(complex_long_double_mul, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_MUL)
BINARY_OP(complex_float32_div, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_DIV)
BINARY_OP(complex_float64_div, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_DIV)
BINARY_OP(complex_long_double_div, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_DIV)

#undef BINARY_OP

#define BINARY_INT_CONST_OP(_id, _opcode)                                                                           \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,       \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,    \
                                                kefir_int64_t imm, kefir_opt_instruction_ref_t *instr_id_ptr) {     \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));          \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container")); \
        REQUIRE_OK(instr_exists(code, block_id, ref1, false));                                                      \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                          \
            mem, code, block_id,                                                                                    \
            &(struct kefir_opt_operation){.opcode = (_opcode),                                                      \
                                          .parameters.ref_imm = {.refs = {ref1}, .integer = imm}},                  \
            false, instr_id_ptr));                                                                                  \
        return KEFIR_OK;                                                                                            \
    }

BINARY_INT_CONST_OP(int_add_const, KEFIR_OPT_OPCODE_INT_ADD_CONST)
BINARY_INT_CONST_OP(int_sub_const, KEFIR_OPT_OPCODE_INT_SUB_CONST)
BINARY_INT_CONST_OP(int_mul_const, KEFIR_OPT_OPCODE_INT_MUL_CONST)
BINARY_INT_CONST_OP(int_and_const, KEFIR_OPT_OPCODE_INT_AND_CONST)
BINARY_INT_CONST_OP(int_or_const, KEFIR_OPT_OPCODE_INT_OR_CONST)
BINARY_INT_CONST_OP(int_xor_const, KEFIR_OPT_OPCODE_INT_XOR_CONST)

BINARY_INT_CONST_OP(int_lshift_const, KEFIR_OPT_OPCODE_INT_LSHIFT_CONST)
BINARY_INT_CONST_OP(int_rshift_const, KEFIR_OPT_OPCODE_INT_RSHIFT_CONST)
BINARY_INT_CONST_OP(int_arshift_const, KEFIR_OPT_OPCODE_INT_ARSHIFT_CONST)

BINARY_INT_CONST_OP(int_equals_const, KEFIR_OPT_OPCODE_INT_EQUALS_CONST)
BINARY_INT_CONST_OP(int_greater_const, KEFIR_OPT_OPCODE_INT_GREATER_CONST)
BINARY_INT_CONST_OP(int_greater_or_equals_const, KEFIR_OPT_OPCODE_INT_GREATER_OR_EQUALS_CONST)
BINARY_INT_CONST_OP(int_lesser_const, KEFIR_OPT_OPCODE_INT_LESSER_CONST)
BINARY_INT_CONST_OP(int_lesser_or_equals_const, KEFIR_OPT_OPCODE_INT_LESSER_OR_EQUALS_CONST)
BINARY_INT_CONST_OP(int_above_const, KEFIR_OPT_OPCODE_INT_ABOVE_CONST)
BINARY_INT_CONST_OP(int_above_or_equals_const, KEFIR_OPT_OPCODE_INT_ABOVE_OR_EQUALS_CONST)
BINARY_INT_CONST_OP(int_below_const, KEFIR_OPT_OPCODE_INT_BELOW_CONST)
BINARY_INT_CONST_OP(int_below_or_equals_const, KEFIR_OPT_OPCODE_INT_BELOW_OR_EQUALS_CONST)

#undef BINARY_INT_CONST_OP

#define LOAD_OP(_id, _opcode)                                                                                        \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,        \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t location, \
                                                const struct kefir_opt_memory_access_flags *flags,                   \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                         \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));           \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));  \
        REQUIRE(flags != NULL,                                                                                       \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory access flags"));           \
        REQUIRE_OK(instr_exists(code, block_id, location, false));                                                   \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                           \
            mem, code, block_id,                                                                                     \
            &(struct kefir_opt_operation){.opcode = (_opcode),                                                       \
                                          .parameters.memory_access = {.location = location, .flags = *flags}},      \
            false, instr_id_ptr));                                                                                   \
        return KEFIR_OK;                                                                                             \
    }

LOAD_OP(int8_load_signed, KEFIR_OPT_OPCODE_INT8_LOAD_SIGNED)
LOAD_OP(int8_load_unsigned, KEFIR_OPT_OPCODE_INT8_LOAD_UNSIGNED)
LOAD_OP(int16_load_signed, KEFIR_OPT_OPCODE_INT16_LOAD_SIGNED)
LOAD_OP(int16_load_unsigned, KEFIR_OPT_OPCODE_INT16_LOAD_UNSIGNED)
LOAD_OP(int32_load_signed, KEFIR_OPT_OPCODE_INT32_LOAD_SIGNED)
LOAD_OP(int32_load_unsigned, KEFIR_OPT_OPCODE_INT32_LOAD_UNSIGNED)
LOAD_OP(int64_load, KEFIR_OPT_OPCODE_INT64_LOAD)
LOAD_OP(long_double_load, KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD)

#undef LOAD_OP

#define STORE_OP(_id, _opcode)                                                                                      \
    kefir_result_t kefir_opt_code_builder_##_id(                                                                    \
        struct kefir_mem *mem, struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,                \
        kefir_opt_instruction_ref_t location, kefir_opt_instruction_ref_t value,                                    \
        const struct kefir_opt_memory_access_flags *flags, kefir_opt_instruction_ref_t *instr_id_ptr) {             \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));          \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container")); \
        REQUIRE(flags != NULL,                                                                                      \
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer memory access flags"));          \
        REQUIRE_OK(instr_exists(code, block_id, location, false));                                                  \
        REQUIRE_OK(instr_exists(code, block_id, value, false));                                                     \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                          \
            mem, code, block_id,                                                                                    \
            &(struct kefir_opt_operation){                                                                          \
                .opcode = (_opcode),                                                                                \
                .parameters.memory_access = {.location = location, .value = value, .flags = *flags}},               \
            false, instr_id_ptr));                                                                                  \
        return KEFIR_OK;                                                                                            \
    }

STORE_OP(int8_store, KEFIR_OPT_OPCODE_INT8_STORE)
STORE_OP(int16_store, KEFIR_OPT_OPCODE_INT16_STORE)
STORE_OP(int32_store, KEFIR_OPT_OPCODE_INT32_STORE)
STORE_OP(int64_store, KEFIR_OPT_OPCODE_INT64_STORE)
STORE_OP(long_double_store, KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE)

#undef STORE_OP

#define ATOMIC_LOAD_OP(_id, _opcode)                                                                                   \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,          \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t location,   \
                                                kefir_opt_atomic_model_t model,                                        \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                           \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));             \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));    \
        REQUIRE_OK(instr_exists(code, block_id, location, false));                                                     \
        switch (model) {                                                                                               \
            case KEFIR_OPT_ATOMIC_MODEL_SEQ_CST:                                                                       \
                break;                                                                                                 \
                                                                                                                       \
            default:                                                                                                   \
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected atomic model");                            \
        }                                                                                                              \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                             \
            mem, code, block_id,                                                                                       \
            &(struct kefir_opt_operation){.opcode = (_opcode),                                                         \
                                          .parameters.atomic_op = {.ref = {location, KEFIR_ID_NONE}, .model = model}}, \
            false, instr_id_ptr));                                                                                     \
        return KEFIR_OK;                                                                                               \
    }

ATOMIC_LOAD_OP(atomic_load8, KEFIR_OPT_OPCODE_ATOMIC_LOAD8)
ATOMIC_LOAD_OP(atomic_load16, KEFIR_OPT_OPCODE_ATOMIC_LOAD16)
ATOMIC_LOAD_OP(atomic_load32, KEFIR_OPT_OPCODE_ATOMIC_LOAD32)
ATOMIC_LOAD_OP(atomic_load64, KEFIR_OPT_OPCODE_ATOMIC_LOAD64)

#undef ATOMIC_LOAD_OP

#define ATOMIC_STORE_OP(_id, _opcode)                                                                                \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,        \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t location, \
                                                kefir_opt_instruction_ref_t value, kefir_opt_atomic_model_t model,   \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                         \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));           \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));  \
        REQUIRE_OK(instr_exists(code, block_id, location, false));                                                   \
        REQUIRE_OK(instr_exists(code, block_id, value, false));                                                      \
        switch (model) {                                                                                             \
            case KEFIR_OPT_ATOMIC_MODEL_SEQ_CST:                                                                     \
                break;                                                                                               \
                                                                                                                     \
            default:                                                                                                 \
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected atomic model");                          \
        }                                                                                                            \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                           \
            mem, code, block_id,                                                                                     \
            &(struct kefir_opt_operation){.opcode = (_opcode),                                                       \
                                          .parameters.atomic_op = {.ref = {location, value}, .model = model}},       \
            false, instr_id_ptr));                                                                                   \
        return KEFIR_OK;                                                                                             \
    }

ATOMIC_STORE_OP(atomic_store8, KEFIR_OPT_OPCODE_ATOMIC_STORE8)
ATOMIC_STORE_OP(atomic_store16, KEFIR_OPT_OPCODE_ATOMIC_STORE16)
ATOMIC_STORE_OP(atomic_store32, KEFIR_OPT_OPCODE_ATOMIC_STORE32)
ATOMIC_STORE_OP(atomic_store64, KEFIR_OPT_OPCODE_ATOMIC_STORE64)

#undef ATOMIC_STORE_OP
