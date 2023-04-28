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
#define KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_INCLUDE
#include "kefir/optimizer/constructor_internal.h"

static kefir_result_t identify_code_blocks(struct kefir_mem *mem, const struct kefir_irblock *ir_block,
                                           struct kefir_opt_constructor_state *state) {
    kefir_bool_t start_new_block = true;
    for (kefir_size_t i = 0; i < kefir_irblock_length(ir_block); i++) {
        if (start_new_block) {
            REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, i));
        }
        start_new_block = false;

        const struct kefir_irinstr *instr = kefir_irblock_at(ir_block, i);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));
        switch (instr->opcode) {
            case KEFIR_IROPCODE_JMP:
            case KEFIR_IROPCODE_BRANCH:
            case KEFIR_IROPCODE_PUSHLABEL:
                REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, instr->arg.u64));
                // Fallthrough

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

static kefir_result_t translate_instruction(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_code_container *code,
                                            struct kefir_opt_constructor_state *state,
                                            const struct kefir_irinstr *instr) {
    UNUSED(module);
    switch (instr->opcode) {
        case KEFIR_IROPCODE_JMP: {
            struct kefir_opt_constructor_code_block_state *jump_target_block = NULL;
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64, &jump_target_block));
            REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, code, state->current_block->block_id,
                                                            jump_target_block->block_id, NULL));
        } break;

        case KEFIR_IROPCODE_BRANCH: {
            struct kefir_opt_constructor_code_block_state *jump_target_block = NULL, *alternative_block = NULL;
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64, &jump_target_block));
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, state->ir_location + 1, &alternative_block));
            REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, code, state->current_block->block_id, KEFIR_ID_NONE,
                                                              jump_target_block->block_id, alternative_block->block_id,
                                                              NULL));
        } break;

        case KEFIR_IROPCODE_RET:
            REQUIRE_OK(
                kefir_opt_code_builder_finalize_return(mem, code, state->current_block->block_id, KEFIR_ID_NONE, NULL));
            break;

        default:
            // TODO Implement complete translation
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_code(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                     const struct kefir_irblock *ir_block, struct kefir_opt_code_container *code,
                                     struct kefir_opt_constructor_state *state) {
    UNUSED(module);
    state->current_block = NULL;
    state->ir_location = 0;
    for (; state->ir_location < kefir_irblock_length(ir_block); state->ir_location++) {
        REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state));

        const struct kefir_irinstr *instr = kefir_irblock_at(ir_block, state->ir_location);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));
        REQUIRE_OK(translate_instruction(mem, module, code, state, instr));
    }

    REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state));
    kefir_bool_t last_block_finalized = false;
    REQUIRE_OK(kefir_opt_code_builder_is_finalized(code, state->current_block->block_id, &last_block_finalized));
    if (!last_block_finalized) {
        REQUIRE_OK(
            kefir_opt_code_builder_finalize_return(mem, code, state->current_block->block_id, KEFIR_ID_NONE, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t construct_code_from_ir(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             const struct kefir_irblock *ir_block,
                                             struct kefir_opt_code_container *code,
                                             struct kefir_opt_constructor_state *state) {
    REQUIRE_OK(identify_code_blocks(mem, ir_block, state));
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

    struct kefir_opt_constructor_state state;
    REQUIRE_OK(kefir_opt_constructor_init(code, &state));

    kefir_result_t res = construct_code_from_ir(mem, module, ir_block, code, &state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_constructor_free(mem, &state);
        return res;
    });

    REQUIRE_OK(kefir_opt_constructor_free(mem, &state));
    return KEFIR_OK;
}
