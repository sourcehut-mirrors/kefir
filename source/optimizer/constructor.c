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

#include "kefir/optimizer/constructor.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct constructor_state {
    struct kefir_hashtree code_blocks;
    kefir_opt_block_id_t current_block;
    kefir_size_t ir_pc;
};

static kefir_result_t identify_code_blocks(struct kefir_mem *mem, const struct kefir_irblock *ir_block,
                                           struct kefir_opt_code_container *code, struct constructor_state *state) {
    kefir_opt_block_id_t code_block_id;
    kefir_bool_t start_new_block = true;
    for (kefir_size_t i = 0; i < kefir_irblock_length(ir_block); i++) {
        if (start_new_block && !kefir_hashtree_has(&state->code_blocks, (kefir_hashtree_key_t) i)) {
            REQUIRE_OK(kefir_opt_code_container_new_block(mem, code, i == 0, &code_block_id));
            REQUIRE_OK(kefir_hashtree_insert(mem, &state->code_blocks, (kefir_hashtree_key_t) i,
                                             (kefir_hashtree_value_t) code_block_id));
        }
        start_new_block = false;

        const struct kefir_irinstr *instr = kefir_irblock_at(ir_block, i);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));
        switch (instr->opcode) {
            case KEFIR_IROPCODE_JMP:
            case KEFIR_IROPCODE_BRANCH:
            case KEFIR_IROPCODE_PUSHLABEL: {
                kefir_size_t jump_target = instr->arg.u64;
                if (!kefir_hashtree_has(&state->code_blocks, (kefir_hashtree_key_t) jump_target)) {
                    REQUIRE_OK(kefir_opt_code_container_new_block(mem, code, false, &code_block_id));
                    REQUIRE_OK(kefir_hashtree_insert(mem, &state->code_blocks, (kefir_hashtree_key_t) jump_target,
                                                     (kefir_hashtree_value_t) code_block_id));
                }
                start_new_block = true;
            } break;

            case KEFIR_IROPCODE_IJMP:
            case KEFIR_IROPCODE_RET:
                start_new_block = true;
                break;

            default:
                // Intentionally left blank
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_opt_block_id_t get_code_block_by_pc(const struct constructor_state *state, kefir_size_t pc) {
    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&state->code_blocks, (kefir_hashtree_key_t) pc, &node);
    if (res != KEFIR_OK) {
        return KEFIR_ID_NONE;
    } else {
        return (kefir_opt_block_id_t) node->value;
    }
}

static kefir_result_t find_current_code_block(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                              struct constructor_state *state) {
    kefir_opt_block_id_t block_by_pc = get_code_block_by_pc(state, state->ir_pc);
    if (block_by_pc != KEFIR_ID_NONE && block_by_pc != state->current_block) {
        if (state->current_block != KEFIR_ID_NONE) {
            kefir_bool_t current_block_finalized = false;
            REQUIRE_OK(kefir_opt_code_builder_is_finalized(code, state->current_block, &current_block_finalized));
            if (!current_block_finalized) {
                REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, code, state->current_block, block_by_pc, NULL));
            }
        }
        state->current_block = block_by_pc;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_instruction(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_code_container *code, struct constructor_state *state,
                                            const struct kefir_irinstr *instr) {
    UNUSED(module);
    switch (instr->opcode) {
        case KEFIR_IROPCODE_JMP: {
            kefir_opt_block_id_t jump_target_block = get_code_block_by_pc(state, instr->arg.u64);
            REQUIRE(jump_target_block != KEFIR_ID_NONE,
                    KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid optimizer block to exist for the jump"));
            REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, code, state->current_block, jump_target_block, NULL));
        } break;

        case KEFIR_IROPCODE_BRANCH: {
            kefir_opt_block_id_t jump_target_block = get_code_block_by_pc(state, instr->arg.u64);
            kefir_opt_block_id_t alternative_block = get_code_block_by_pc(state, state->ir_pc + 1);
            REQUIRE(
                jump_target_block != KEFIR_ID_NONE && alternative_block != KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid optimizer blocks to exist for both branches"));
            REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, code, state->current_block, KEFIR_ID_NONE,
                                                              jump_target_block, alternative_block, NULL));
        } break;

        case KEFIR_IROPCODE_RET: {
            REQUIRE_OK(kefir_opt_code_builder_finalize_return(mem, code, state->current_block, KEFIR_ID_NONE, NULL));
        } break;

        default:
            // TODO Implement complete translation
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_code(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                     const struct kefir_irblock *ir_block, struct kefir_opt_code_container *code,
                                     struct constructor_state *state) {
    UNUSED(module);
    state->current_block = KEFIR_ID_NONE;
    state->ir_pc = 0;
    for (; state->ir_pc < kefir_irblock_length(ir_block); state->ir_pc++) {
        REQUIRE_OK(find_current_code_block(mem, code, state));

        const struct kefir_irinstr *instr = kefir_irblock_at(ir_block, state->ir_pc);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));
        REQUIRE_OK(translate_instruction(mem, module, code, state, instr));
    }

    REQUIRE_OK(find_current_code_block(mem, code, state));
    kefir_bool_t last_block_finalized = false;
    REQUIRE_OK(kefir_opt_code_builder_is_finalized(code, state->current_block, &last_block_finalized));
    if (!last_block_finalized) {
        REQUIRE_OK(kefir_opt_code_builder_finalize_return(mem, code, state->current_block, KEFIR_ID_NONE, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t construct_code_from_ir(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             const struct kefir_irblock *ir_block,
                                             struct kefir_opt_code_container *code, struct constructor_state *state) {
    REQUIRE_OK(identify_code_blocks(mem, ir_block, code, state));
    REQUIRE_OK(translate_code(mem, module, ir_block, code, state));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_construct_code_from_ir(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                const struct kefir_irblock *ir_block,
                                                struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(ir_block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block"));
    REQUIRE(code != NULL && kefir_opt_code_container_is_empty(code),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid empty optimizer code container"));

    struct constructor_state state = {0};
    REQUIRE_OK(kefir_hashtree_init(&state.code_blocks, &kefir_hashtree_uint_ops));

    kefir_result_t res = construct_code_from_ir(mem, module, ir_block, code, &state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &state.code_blocks);
        return res;
    });

    REQUIRE_OK(kefir_hashtree_free(mem, &state.code_blocks));
    return KEFIR_OK;
}
