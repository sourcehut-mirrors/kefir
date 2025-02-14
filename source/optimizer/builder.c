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

#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t block_exists(const struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id) {
    const struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_add_instruction(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id,
                                                      const struct kefir_opt_operation *operation, kefir_bool_t control,
                                                      kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(block_exists(code, block_id));

    kefir_bool_t finalized = false;
    REQUIRE_OK(kefir_opt_code_builder_is_finalized(code, block_id, &finalized));
    REQUIRE(!control || !finalized,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer code block has already been finalized"));

    kefir_opt_instruction_ref_t instr_id;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(mem, code, block_id, operation, &instr_id));
    if (control) {
        REQUIRE_OK(kefir_opt_code_container_add_control(code, block_id, instr_id, false));
    }

    ASSIGN_PTR(instr_id_ptr, instr_id);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_add_control(struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
                                                  kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(block_exists(code, block_id));
    REQUIRE_OK(kefir_opt_code_container_add_control(code, block_id, instr_ref, false));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_add_control_side_effect_free(struct kefir_opt_code_container *code,
                                                                   kefir_opt_block_id_t block_id,
                                                                   kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(block_exists(code, block_id));
    REQUIRE_OK(kefir_opt_code_container_add_control(code, block_id, instr_ref, true));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_is_finalized(const struct kefir_opt_code_container *code,
                                                   kefir_opt_block_id_t block_id, kefir_bool_t *finalized_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(finalized_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    kefir_opt_instruction_ref_t prev_instr_ref;
    const struct kefir_opt_instruction *prev_instr = NULL;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, block, &prev_instr_ref));
    if (prev_instr_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, prev_instr_ref, &prev_instr));
        switch (prev_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_JUMP:
            case KEFIR_OPT_OPCODE_IJUMP:
            case KEFIR_OPT_OPCODE_BRANCH:
            case KEFIR_OPT_OPCODE_RETURN:
                *finalized_ptr = true;
                break;

            case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
                const struct kefir_opt_inline_assembly_node *inline_asm = NULL;
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

static kefir_result_t instr_exists(const struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
                                   kefir_opt_instruction_ref_t instr_id, kefir_bool_t permit_none) {
    UNUSED(block_id);
    REQUIRE(permit_none || instr_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Optimized code instruction with requested identifier is not found"));

    if (!permit_none) {
        const struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_id, &instr));
    }
    return KEFIR_OK;
}

static kefir_result_t phi_exists(const struct kefir_opt_code_container *code, kefir_opt_phi_id_t phi_ref) {
    const struct kefir_opt_phi_node *phi = NULL;
    REQUIRE_OK(kefir_opt_code_container_phi(code, phi_ref, &phi));
    return KEFIR_OK;
}

static kefir_result_t call_exists(const struct kefir_opt_code_container *code, kefir_opt_call_id_t call_ref) {
    const struct kefir_opt_call_node *call = NULL;
    REQUIRE_OK(kefir_opt_code_container_call(code, call_ref, &call));
    return KEFIR_OK;
}

static kefir_result_t inline_asm_exists(const struct kefir_opt_code_container *code,
                                        kefir_opt_inline_assembly_id_t inline_asm_ref) {
    const struct kefir_opt_inline_assembly_node *inline_asm = NULL;
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_JUMP,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_IJUMP, .parameters.refs[0] = arg_instr_id}, true,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_finalize_branch(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id,
                                                      kefir_opt_branch_condition_variant_t condition_variant,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_BRANCH,
                                       .parameters.branch = {.target_block = target_block,
                                                             .alternative_block = alternative_block,
                                                             .condition_variant = condition_variant,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_RETURN, .parameters.refs[0] = arg_instr_id}, true,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_get_argument(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                   kefir_opt_block_id_t block_id, kefir_size_t index,
                                                   kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_GET_ARGUMENT, .parameters.index = index}, false,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_PHI, .parameters.phi_ref = identifier}, false,
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

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_INLINE_ASSEMBLY,
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
        &(struct kefir_opt_operation) {
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_INVOKE_VIRTUAL,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_INT_CONST, .parameters.imm.integer = value}, false,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_UINT_CONST, .parameters.imm.uinteger = value}, false,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_FLOAT32_CONST, .parameters.imm.float32 = value},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_float64_constant(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t block_id, kefir_float64_t value,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_FLOAT64_CONST, .parameters.imm.float64 = value},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_long_double_constant(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                           kefir_opt_block_id_t block_id, kefir_long_double_t value,
                                                           kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_STRING_REF, .parameters.imm.string_ref = ref}, false,
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_BLOCK_LABEL, .parameters.imm.block_ref = ref}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_int_placeholder(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id,
                                                      kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_INT_PLACEHOLDER}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_float32_placeholder(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                          kefir_opt_block_id_t block_id,
                                                          kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_FLOAT32_PLACEHOLDER}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_float64_placeholder(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                          kefir_opt_block_id_t block_id,
                                                          kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_FLOAT64_PLACEHOLDER}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_get_global(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_id, kefir_id_t identifier,
                                                 kefir_int64_t offset, kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_GET_GLOBAL,
                                       .parameters.variable = {.global_ref = identifier, .offset = offset}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_get_thread_local(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t block_id, kefir_id_t identifier,
                                                       kefir_int64_t offset,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_GET_THREAD_LOCAL,
                                       .parameters.variable = {.global_ref = identifier, .offset = offset}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_alloc_local(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_opt_block_id_t block_id, kefir_id_t type_id,
                                                  kefir_size_t type_index, kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_ALLOC_LOCAL,
                                       .parameters.type = {.type_id = type_id, .type_index = type_index}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_ref_local(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t instr_ref,
                                                kefir_int64_t offset, kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    const struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Local reference optimizer instruction may only consume local variable allocation"));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_REF_LOCAL,
                                       .parameters = {.refs = {instr_ref}, .offset = offset}},
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_ZERO_MEMORY,
                                       .parameters = {.refs = {location_ref, KEFIR_ID_NONE},
                                                      .type = {.type_id = type_id, .type_index = type_index}}},
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
        &(struct kefir_opt_operation) {
            .opcode = KEFIR_OPT_OPCODE_COPY_MEMORY,
            .parameters = {.refs = {location_ref, source_ref}, .type = {.type_id = type_id, .type_index = type_index}}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_atomic_copy_memory_from(
    struct kefir_mem *mem, struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
    kefir_opt_instruction_ref_t location_ref, kefir_opt_instruction_ref_t source_ref,
    kefir_opt_memory_order_t memory_model, kefir_id_t type_id, kefir_size_t type_index,
    kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, location_ref, false));
    REQUIRE_OK(instr_exists(code, block_id, source_ref, false));
    switch (memory_model) {
        case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected atomic model");
    }
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_FROM,
                                       .parameters = {.refs = {location_ref, source_ref, KEFIR_ID_NONE},
                                                      .atomic_op = {.model = memory_model},
                                                      .type.type_id = type_id,
                                                      .type.type_index = type_index}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_atomic_copy_memory_to(
    struct kefir_mem *mem, struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
    kefir_opt_instruction_ref_t location_ref, kefir_opt_instruction_ref_t source_ref,
    kefir_opt_memory_order_t memory_model, kefir_id_t type_id, kefir_size_t type_index,
    kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, location_ref, false));
    REQUIRE_OK(instr_exists(code, block_id, source_ref, false));
    switch (memory_model) {
        case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected atomic model");
    }
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_TO,
                                       .parameters = {.refs = {location_ref, source_ref, KEFIR_ID_NONE},
                                                      .atomic_op = {.model = memory_model},
                                                      .type.type_id = type_id,
                                                      .type.type_index = type_index}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_atomic_compare_exchange_memory(
    struct kefir_mem *mem, struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
    kefir_opt_instruction_ref_t location_ref, kefir_opt_instruction_ref_t expected_ref,
    kefir_opt_instruction_ref_t desired_ref, kefir_opt_memory_order_t memory_model, kefir_id_t type_id,
    kefir_size_t type_index, kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, location_ref, false));
    REQUIRE_OK(instr_exists(code, block_id, expected_ref, false));
    REQUIRE_OK(instr_exists(code, block_id, desired_ref, false));
    switch (memory_model) {
        case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected atomic model");
    }
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_MEMORY,
                                       .parameters = {.refs = {location_ref, expected_ref, desired_ref},
                                                      .atomic_op = {.model = memory_model},
                                                      .type.type_id = type_id,
                                                      .type.type_index = type_index}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_fenv_save(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                kefir_opt_block_id_t block_id,
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_FENV_SAVE}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_fenv_clear(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_id,
                                                 kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_FENV_CLEAR}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_fenv_update(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref,
                                                  kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(instr_exists(code, block_id, ref, false));
    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_FENV_UPDATE, .parameters.refs[0] = ref}, false,
        instr_id_ptr));
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED,
                                       .parameters = {.refs[KEFIR_OPT_BITFIELD_BASE_REF] = base_ref,
                                                      .refs[KEFIR_OPT_BITFIELD_VALUE_REF] = KEFIR_ID_NONE,
                                                      .bitfield = {.offset = offset, .length = length}}},
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED,
                                       .parameters = {.refs[KEFIR_OPT_BITFIELD_BASE_REF] = base_ref,
                                                      .refs[KEFIR_OPT_BITFIELD_VALUE_REF] = KEFIR_ID_NONE,
                                                      .bitfield = {.offset = offset, .length = length}}},
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_BITS_INSERT,
                                       .parameters = {.refs[KEFIR_OPT_BITFIELD_BASE_REF] = base_ref,
                                                      .refs[KEFIR_OPT_BITFIELD_VALUE_REF] = value_ref,
                                                      .bitfield = {.offset = offset, .length = length}}},
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
        &(struct kefir_opt_operation) {
            .opcode = KEFIR_OPT_OPCODE_VARARG_GET,
            .parameters = {.refs = {source_ref}, .type = {.type_id = type_id, .type_index = type_index}}},
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
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_STACK_ALLOC,
                                       .parameters = {.refs[KEFIR_OPT_STACK_ALLOCATION_SIZE_REF] = size_ref,
                                                      .refs[KEFIR_OPT_STACK_ALLOCATION_ALIGNMENT_REF] = alignment_ref,
                                                      .stack_allocation.within_scope = within_scope}},
        false, instr_id_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_scope_push(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_id,
                                                 kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id, &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_SCOPE_PUSH}, false,
        instr_id_ptr));
    return KEFIR_OK;
}

#define UNARY_OP(_id, _opcode)                                                                                      \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,       \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,    \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                        \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));          \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container")); \
        REQUIRE_OK(instr_exists(code, block_id, ref1, false));                                                      \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                          \
            mem, code, block_id, &(struct kefir_opt_operation) {.opcode = (_opcode), .parameters.refs = {ref1}},    \
            false, instr_id_ptr));                                                                                  \
        return KEFIR_OK;                                                                                            \
    }

UNARY_OP(int8_not, KEFIR_OPT_OPCODE_INT8_NOT)
UNARY_OP(int16_not, KEFIR_OPT_OPCODE_INT16_NOT)
UNARY_OP(int32_not, KEFIR_OPT_OPCODE_INT32_NOT)
UNARY_OP(int64_not, KEFIR_OPT_OPCODE_INT64_NOT)
UNARY_OP(int8_neg, KEFIR_OPT_OPCODE_INT8_NEG)
UNARY_OP(int16_neg, KEFIR_OPT_OPCODE_INT16_NEG)
UNARY_OP(int32_neg, KEFIR_OPT_OPCODE_INT32_NEG)
UNARY_OP(int64_neg, KEFIR_OPT_OPCODE_INT64_NEG)
UNARY_OP(int8_bool_not, KEFIR_OPT_OPCODE_INT8_BOOL_NOT)
UNARY_OP(int16_bool_not, KEFIR_OPT_OPCODE_INT16_BOOL_NOT)
UNARY_OP(int32_bool_not, KEFIR_OPT_OPCODE_INT32_BOOL_NOT)
UNARY_OP(int64_bool_not, KEFIR_OPT_OPCODE_INT64_BOOL_NOT)

UNARY_OP(int8_to_bool, KEFIR_OPT_OPCODE_INT8_TO_BOOL)
UNARY_OP(int16_to_bool, KEFIR_OPT_OPCODE_INT16_TO_BOOL)
UNARY_OP(int32_to_bool, KEFIR_OPT_OPCODE_INT32_TO_BOOL)
UNARY_OP(int64_to_bool, KEFIR_OPT_OPCODE_INT64_TO_BOOL)
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

#define BINARY_OP(_id, _opcode)                                                                                        \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,          \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,       \
                                                kefir_opt_instruction_ref_t ref2,                                      \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                           \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));             \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));    \
        REQUIRE_OK(instr_exists(code, block_id, ref1, false));                                                         \
        REQUIRE_OK(instr_exists(code, block_id, ref2, false));                                                         \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                             \
            mem, code, block_id, &(struct kefir_opt_operation) {.opcode = (_opcode), .parameters.refs = {ref1, ref2}}, \
            false, instr_id_ptr));                                                                                     \
        return KEFIR_OK;                                                                                               \
    }

BINARY_OP(int8_add, KEFIR_OPT_OPCODE_INT8_ADD)
BINARY_OP(int16_add, KEFIR_OPT_OPCODE_INT16_ADD)
BINARY_OP(int32_add, KEFIR_OPT_OPCODE_INT32_ADD)
BINARY_OP(int64_add, KEFIR_OPT_OPCODE_INT64_ADD)
BINARY_OP(int8_sub, KEFIR_OPT_OPCODE_INT8_SUB)
BINARY_OP(int16_sub, KEFIR_OPT_OPCODE_INT16_SUB)
BINARY_OP(int32_sub, KEFIR_OPT_OPCODE_INT32_SUB)
BINARY_OP(int64_sub, KEFIR_OPT_OPCODE_INT64_SUB)
BINARY_OP(int8_mul, KEFIR_OPT_OPCODE_INT8_MUL)
BINARY_OP(int16_mul, KEFIR_OPT_OPCODE_INT16_MUL)
BINARY_OP(int32_mul, KEFIR_OPT_OPCODE_INT32_MUL)
BINARY_OP(int64_mul, KEFIR_OPT_OPCODE_INT64_MUL)
BINARY_OP(uint8_mul, KEFIR_OPT_OPCODE_UINT8_MUL)
BINARY_OP(uint16_mul, KEFIR_OPT_OPCODE_UINT16_MUL)
BINARY_OP(uint32_mul, KEFIR_OPT_OPCODE_UINT32_MUL)
BINARY_OP(uint64_mul, KEFIR_OPT_OPCODE_UINT64_MUL)
BINARY_OP(int8_div, KEFIR_OPT_OPCODE_INT8_DIV)
BINARY_OP(int16_div, KEFIR_OPT_OPCODE_INT16_DIV)
BINARY_OP(int32_div, KEFIR_OPT_OPCODE_INT32_DIV)
BINARY_OP(int64_div, KEFIR_OPT_OPCODE_INT64_DIV)
BINARY_OP(int8_mod, KEFIR_OPT_OPCODE_INT8_MOD)
BINARY_OP(int16_mod, KEFIR_OPT_OPCODE_INT16_MOD)
BINARY_OP(int32_mod, KEFIR_OPT_OPCODE_INT32_MOD)
BINARY_OP(int64_mod, KEFIR_OPT_OPCODE_INT64_MOD)
BINARY_OP(uint8_div, KEFIR_OPT_OPCODE_UINT8_DIV)
BINARY_OP(uint16_div, KEFIR_OPT_OPCODE_UINT16_DIV)
BINARY_OP(uint32_div, KEFIR_OPT_OPCODE_UINT32_DIV)
BINARY_OP(uint64_div, KEFIR_OPT_OPCODE_UINT64_DIV)
BINARY_OP(uint8_mod, KEFIR_OPT_OPCODE_UINT8_MOD)
BINARY_OP(uint16_mod, KEFIR_OPT_OPCODE_UINT16_MOD)
BINARY_OP(uint32_mod, KEFIR_OPT_OPCODE_UINT32_MOD)
BINARY_OP(uint64_mod, KEFIR_OPT_OPCODE_UINT64_MOD)
BINARY_OP(int8_and, KEFIR_OPT_OPCODE_INT8_AND)
BINARY_OP(int16_and, KEFIR_OPT_OPCODE_INT16_AND)
BINARY_OP(int32_and, KEFIR_OPT_OPCODE_INT32_AND)
BINARY_OP(int64_and, KEFIR_OPT_OPCODE_INT64_AND)
BINARY_OP(int8_or, KEFIR_OPT_OPCODE_INT8_OR)
BINARY_OP(int16_or, KEFIR_OPT_OPCODE_INT16_OR)
BINARY_OP(int32_or, KEFIR_OPT_OPCODE_INT32_OR)
BINARY_OP(int64_or, KEFIR_OPT_OPCODE_INT64_OR)
BINARY_OP(int8_xor, KEFIR_OPT_OPCODE_INT8_XOR)
BINARY_OP(int16_xor, KEFIR_OPT_OPCODE_INT16_XOR)
BINARY_OP(int32_xor, KEFIR_OPT_OPCODE_INT32_XOR)
BINARY_OP(int64_xor, KEFIR_OPT_OPCODE_INT64_XOR)
BINARY_OP(int8_lshift, KEFIR_OPT_OPCODE_INT8_LSHIFT)
BINARY_OP(int16_lshift, KEFIR_OPT_OPCODE_INT16_LSHIFT)
BINARY_OP(int32_lshift, KEFIR_OPT_OPCODE_INT32_LSHIFT)
BINARY_OP(int64_lshift, KEFIR_OPT_OPCODE_INT64_LSHIFT)
BINARY_OP(int8_rshift, KEFIR_OPT_OPCODE_INT8_RSHIFT)
BINARY_OP(int16_rshift, KEFIR_OPT_OPCODE_INT16_RSHIFT)
BINARY_OP(int32_rshift, KEFIR_OPT_OPCODE_INT32_RSHIFT)
BINARY_OP(int64_rshift, KEFIR_OPT_OPCODE_INT64_RSHIFT)
BINARY_OP(int8_arshift, KEFIR_OPT_OPCODE_INT8_ARSHIFT)
BINARY_OP(int16_arshift, KEFIR_OPT_OPCODE_INT16_ARSHIFT)
BINARY_OP(int32_arshift, KEFIR_OPT_OPCODE_INT32_ARSHIFT)
BINARY_OP(int64_arshift, KEFIR_OPT_OPCODE_INT64_ARSHIFT)
BINARY_OP(int8_equals, KEFIR_OPT_OPCODE_INT8_EQUALS)
BINARY_OP(int16_equals, KEFIR_OPT_OPCODE_INT16_EQUALS)
BINARY_OP(int32_equals, KEFIR_OPT_OPCODE_INT32_EQUALS)
BINARY_OP(int64_equals, KEFIR_OPT_OPCODE_INT64_EQUALS)
BINARY_OP(int8_greater, KEFIR_OPT_OPCODE_INT8_GREATER)
BINARY_OP(int16_greater, KEFIR_OPT_OPCODE_INT16_GREATER)
BINARY_OP(int32_greater, KEFIR_OPT_OPCODE_INT32_GREATER)
BINARY_OP(int64_greater, KEFIR_OPT_OPCODE_INT64_GREATER)
BINARY_OP(int8_lesser, KEFIR_OPT_OPCODE_INT8_LESSER)
BINARY_OP(int16_lesser, KEFIR_OPT_OPCODE_INT16_LESSER)
BINARY_OP(int32_lesser, KEFIR_OPT_OPCODE_INT32_LESSER)
BINARY_OP(int64_lesser, KEFIR_OPT_OPCODE_INT64_LESSER)
BINARY_OP(int8_above, KEFIR_OPT_OPCODE_INT8_ABOVE)
BINARY_OP(int16_above, KEFIR_OPT_OPCODE_INT16_ABOVE)
BINARY_OP(int32_above, KEFIR_OPT_OPCODE_INT32_ABOVE)
BINARY_OP(int64_above, KEFIR_OPT_OPCODE_INT64_ABOVE)
BINARY_OP(int8_below, KEFIR_OPT_OPCODE_INT8_BELOW)
BINARY_OP(int16_below, KEFIR_OPT_OPCODE_INT16_BELOW)
BINARY_OP(int32_below, KEFIR_OPT_OPCODE_INT32_BELOW)
BINARY_OP(int64_below, KEFIR_OPT_OPCODE_INT64_BELOW)
BINARY_OP(int8_bool_and, KEFIR_OPT_OPCODE_INT8_BOOL_AND)
BINARY_OP(int16_bool_and, KEFIR_OPT_OPCODE_INT16_BOOL_AND)
BINARY_OP(int32_bool_and, KEFIR_OPT_OPCODE_INT32_BOOL_AND)
BINARY_OP(int64_bool_and, KEFIR_OPT_OPCODE_INT64_BOOL_AND)
BINARY_OP(int8_bool_or, KEFIR_OPT_OPCODE_INT8_BOOL_OR)
BINARY_OP(int16_bool_or, KEFIR_OPT_OPCODE_INT16_BOOL_OR)
BINARY_OP(int32_bool_or, KEFIR_OPT_OPCODE_INT32_BOOL_OR)
BINARY_OP(int64_bool_or, KEFIR_OPT_OPCODE_INT64_BOOL_OR)

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
BINARY_OP(float32_lesser, KEFIR_OPT_OPCODE_FLOAT32_LESSER)
BINARY_OP(float64_equals, KEFIR_OPT_OPCODE_FLOAT64_EQUALS)
BINARY_OP(float64_greater, KEFIR_OPT_OPCODE_FLOAT64_GREATER)
BINARY_OP(float64_lesser, KEFIR_OPT_OPCODE_FLOAT64_LESSER)
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
            &(struct kefir_opt_operation) {.opcode = (_opcode),                                                      \
                                           .parameters = {.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF] = location,    \
                                                          .memory_access.flags = *flags}},                           \
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
LOAD_OP(complex_float32_load, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD)
LOAD_OP(complex_float64_load, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD)
LOAD_OP(complex_long_double_load, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD)

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
            &(struct kefir_opt_operation) {.opcode = (_opcode),                                                     \
                                           .parameters = {.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF] = location,   \
                                                          .refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF] = value,         \
                                                          .memory_access.flags = *flags}},                          \
            false, instr_id_ptr));                                                                                  \
        return KEFIR_OK;                                                                                            \
    }

STORE_OP(int8_store, KEFIR_OPT_OPCODE_INT8_STORE)
STORE_OP(int16_store, KEFIR_OPT_OPCODE_INT16_STORE)
STORE_OP(int32_store, KEFIR_OPT_OPCODE_INT32_STORE)
STORE_OP(int64_store, KEFIR_OPT_OPCODE_INT64_STORE)
STORE_OP(long_double_store, KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE)
STORE_OP(complex_float32_store, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE)
STORE_OP(complex_float64_store, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE)
STORE_OP(complex_long_double_store, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE)

#undef STORE_OP

#define ATOMIC_LOAD_OP(_id, _opcode)                                                                                 \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,        \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t location, \
                                                kefir_opt_memory_order_t model,                                      \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                         \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));           \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));  \
        REQUIRE_OK(instr_exists(code, block_id, location, false));                                                   \
        switch (model) {                                                                                             \
            case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:                                                                     \
                break;                                                                                               \
                                                                                                                     \
            default:                                                                                                 \
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected atomic model");                          \
        }                                                                                                            \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                           \
            mem, code, block_id,                                                                                     \
            &(struct kefir_opt_operation) {                                                                          \
                .opcode = (_opcode),                                                                                 \
                .parameters = {.refs = {location, KEFIR_ID_NONE, KEFIR_ID_NONE}, .atomic_op.model = model}},         \
            false, instr_id_ptr));                                                                                   \
        return KEFIR_OK;                                                                                             \
    }

ATOMIC_LOAD_OP(atomic_load8, KEFIR_OPT_OPCODE_ATOMIC_LOAD8)
ATOMIC_LOAD_OP(atomic_load16, KEFIR_OPT_OPCODE_ATOMIC_LOAD16)
ATOMIC_LOAD_OP(atomic_load32, KEFIR_OPT_OPCODE_ATOMIC_LOAD32)
ATOMIC_LOAD_OP(atomic_load64, KEFIR_OPT_OPCODE_ATOMIC_LOAD64)
ATOMIC_LOAD_OP(atomic_load_long_double, KEFIR_OPT_OPCODE_ATOMIC_LOAD_LONG_DOUBLE)
ATOMIC_LOAD_OP(atomic_load_complex_float32, KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT32)
ATOMIC_LOAD_OP(atomic_load_complex_float64, KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT64)
ATOMIC_LOAD_OP(atomic_load_complex_long_double, KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE)

#undef ATOMIC_LOAD_OP

#define ATOMIC_STORE_OP(_id, _opcode)                                                                                \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,        \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t location, \
                                                kefir_opt_instruction_ref_t value, kefir_opt_memory_order_t model,   \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                         \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));           \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));  \
        REQUIRE_OK(instr_exists(code, block_id, location, false));                                                   \
        REQUIRE_OK(instr_exists(code, block_id, value, false));                                                      \
        switch (model) {                                                                                             \
            case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:                                                                     \
                break;                                                                                               \
                                                                                                                     \
            default:                                                                                                 \
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected atomic model");                          \
        }                                                                                                            \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                           \
            mem, code, block_id,                                                                                     \
            &(struct kefir_opt_operation) {                                                                          \
                .opcode = (_opcode),                                                                                 \
                .parameters = {.refs = {location, value, KEFIR_ID_NONE}, .atomic_op.model = model}},                 \
            false, instr_id_ptr));                                                                                   \
        return KEFIR_OK;                                                                                             \
    }

ATOMIC_STORE_OP(atomic_store8, KEFIR_OPT_OPCODE_ATOMIC_STORE8)
ATOMIC_STORE_OP(atomic_store16, KEFIR_OPT_OPCODE_ATOMIC_STORE16)
ATOMIC_STORE_OP(atomic_store32, KEFIR_OPT_OPCODE_ATOMIC_STORE32)
ATOMIC_STORE_OP(atomic_store64, KEFIR_OPT_OPCODE_ATOMIC_STORE64)
ATOMIC_STORE_OP(atomic_store_long_double, KEFIR_OPT_OPCODE_ATOMIC_STORE_LONG_DOUBLE)

#undef ATOMIC_STORE_OP

#define ATOMIC_CMPXCHG_OP(_id, _opcode)                                                                                \
    kefir_result_t kefir_opt_code_builder_##_id(struct kefir_mem *mem, struct kefir_opt_code_container *code,          \
                                                kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t location,   \
                                                kefir_opt_instruction_ref_t compare_value,                             \
                                                kefir_opt_instruction_ref_t new_value, kefir_opt_memory_order_t model, \
                                                kefir_opt_instruction_ref_t *instr_id_ptr) {                           \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));             \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));    \
        REQUIRE_OK(instr_exists(code, block_id, location, false));                                                     \
        REQUIRE_OK(instr_exists(code, block_id, compare_value, false));                                                \
        REQUIRE_OK(instr_exists(code, block_id, new_value, false));                                                    \
        switch (model) {                                                                                               \
            case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:                                                                       \
                break;                                                                                                 \
                                                                                                                       \
            default:                                                                                                   \
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected atomic model");                            \
        }                                                                                                              \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                             \
            mem, code, block_id,                                                                                       \
            &(struct kefir_opt_operation) {                                                                            \
                .opcode = (_opcode),                                                                                   \
                .parameters = {.refs = {location, compare_value, new_value}, .atomic_op.model = model}},               \
            false, instr_id_ptr));                                                                                     \
        return KEFIR_OK;                                                                                               \
    }

ATOMIC_CMPXCHG_OP(atomic_compare_exchange8, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8)
ATOMIC_CMPXCHG_OP(atomic_compare_exchange16, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16)
ATOMIC_CMPXCHG_OP(atomic_compare_exchange32, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32)
ATOMIC_CMPXCHG_OP(atomic_compare_exchange64, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG64)
ATOMIC_CMPXCHG_OP(atomic_compare_exchange_long_double, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_LONG_DOUBLE)
ATOMIC_CMPXCHG_OP(atomic_compare_exchange_complex_long_double, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE)

#undef ATOMIC_CMPXCHG_OP

#define OVERFLOW_ARITH(_id, _opcode)                                                                                \
    kefir_result_t kefir_opt_code_builder_##_id##_overflow(                                                         \
        struct kefir_mem *mem, struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,                \
        kefir_opt_instruction_ref_t arg1_ref, kefir_opt_instruction_ref_t arg2_ref,                                 \
        kefir_opt_instruction_ref_t result_ptr_ref, kefir_id_t type_id, kefir_size_t type_index,                    \
        kefir_uint8_t signedness, kefir_opt_instruction_ref_t *instr_id_ptr) {                                      \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));          \
        REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container")); \
                                                                                                                    \
        REQUIRE_OK(instr_exists(code, block_id, arg1_ref, false));                                                  \
        REQUIRE_OK(instr_exists(code, block_id, arg2_ref, false));                                                  \
        REQUIRE_OK(instr_exists(code, block_id, result_ptr_ref, false));                                            \
        REQUIRE_OK(kefir_opt_code_builder_add_instruction(                                                          \
            mem, code, block_id,                                                                                    \
            &(struct kefir_opt_operation) {.opcode = (_opcode),                                                     \
                                           .parameters = {.refs = {arg1_ref, arg2_ref, result_ptr_ref},             \
                                                          .type = {.type_id = type_id, .type_index = type_index},   \
                                                          .overflow_arith.signedness = signedness}},                \
            false, instr_id_ptr));                                                                                  \
        return KEFIR_OK;                                                                                            \
    }

OVERFLOW_ARITH(add, KEFIR_OPT_OPCODE_ADD_OVERFLOW)
OVERFLOW_ARITH(sub, KEFIR_OPT_OPCODE_SUB_OVERFLOW)
OVERFLOW_ARITH(mul, KEFIR_OPT_OPCODE_MUL_OVERFLOW)

#undef OVERFLOW_ARITH
