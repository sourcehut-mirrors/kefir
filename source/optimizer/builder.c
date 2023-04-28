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

    kefir_bool_t finalized = false;
    REQUIRE_OK(kefir_opt_code_builder_is_finalized(code, block_id, &finalized));
    REQUIRE(!finalized, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer code block has already been finalized"));

    kefir_opt_instruction_ref_t instr_id;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(mem, code, block, operation, &instr_id));
    if (control) {
        REQUIRE_OK(kefir_opt_code_container_add_control(code, block, instr_id));
    }

    ASSIGN_PTR(instr_id_ptr, instr_id);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_builder_is_finalized(const struct kefir_opt_code_container *code,
                                                   kefir_opt_block_id_t block_id, kefir_bool_t *finalized_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(finalized_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

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
                *finalized_ptr = true;
                break;

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

kefir_result_t kefir_opt_code_builder_string_reference(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t block_id, kefir_id_t ref,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(kefir_opt_code_builder_add_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_STRING_REF, .parameters.refs[0] = ref}, false,
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
        &(struct kefir_opt_operation){.opcode = KEFIR_OPT_OPCODE_BLOCK_LABEL, .parameters.refs[0] = ref}, false,
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

#undef BINARY_OP
