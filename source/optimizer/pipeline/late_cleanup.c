/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/code.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_int_t is_factored_computation(const struct kefir_opt_instruction *instr) {
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_ADD:
        case KEFIR_OPT_OPCODE_INT16_ADD:
        case KEFIR_OPT_OPCODE_INT32_ADD:
        case KEFIR_OPT_OPCODE_INT64_ADD:
        case KEFIR_OPT_OPCODE_INT8_SUB:
        case KEFIR_OPT_OPCODE_INT16_SUB:
        case KEFIR_OPT_OPCODE_INT32_SUB:
        case KEFIR_OPT_OPCODE_INT64_SUB:
        case KEFIR_OPT_OPCODE_INT8_MUL:
        case KEFIR_OPT_OPCODE_INT16_MUL:
        case KEFIR_OPT_OPCODE_INT32_MUL:
        case KEFIR_OPT_OPCODE_INT64_MUL:
        case KEFIR_OPT_OPCODE_UINT8_MUL:
        case KEFIR_OPT_OPCODE_UINT16_MUL:
        case KEFIR_OPT_OPCODE_UINT32_MUL:
        case KEFIR_OPT_OPCODE_UINT64_MUL:
        case KEFIR_OPT_OPCODE_INT8_DIV:
        case KEFIR_OPT_OPCODE_INT16_DIV:
        case KEFIR_OPT_OPCODE_INT32_DIV:
        case KEFIR_OPT_OPCODE_INT64_DIV:
        case KEFIR_OPT_OPCODE_UINT8_DIV:
        case KEFIR_OPT_OPCODE_UINT16_DIV:
        case KEFIR_OPT_OPCODE_UINT32_DIV:
        case KEFIR_OPT_OPCODE_UINT64_DIV:
        case KEFIR_OPT_OPCODE_INT8_MOD:
        case KEFIR_OPT_OPCODE_INT16_MOD:
        case KEFIR_OPT_OPCODE_INT32_MOD:
        case KEFIR_OPT_OPCODE_INT64_MOD:
        case KEFIR_OPT_OPCODE_UINT8_MOD:
        case KEFIR_OPT_OPCODE_UINT16_MOD:
        case KEFIR_OPT_OPCODE_UINT32_MOD:
        case KEFIR_OPT_OPCODE_UINT64_MOD:
        case KEFIR_OPT_OPCODE_INT8_AND:
        case KEFIR_OPT_OPCODE_INT16_AND:
        case KEFIR_OPT_OPCODE_INT32_AND:
        case KEFIR_OPT_OPCODE_INT64_AND:
        case KEFIR_OPT_OPCODE_INT8_OR:
        case KEFIR_OPT_OPCODE_INT16_OR:
        case KEFIR_OPT_OPCODE_INT32_OR:
        case KEFIR_OPT_OPCODE_INT64_OR:
        case KEFIR_OPT_OPCODE_INT8_XOR:
        case KEFIR_OPT_OPCODE_INT16_XOR:
        case KEFIR_OPT_OPCODE_INT32_XOR:
        case KEFIR_OPT_OPCODE_INT64_XOR:
        case KEFIR_OPT_OPCODE_INT8_LSHIFT:
        case KEFIR_OPT_OPCODE_INT16_LSHIFT:
        case KEFIR_OPT_OPCODE_INT32_LSHIFT:
        case KEFIR_OPT_OPCODE_INT64_LSHIFT:
        case KEFIR_OPT_OPCODE_INT8_RSHIFT:
        case KEFIR_OPT_OPCODE_INT16_RSHIFT:
        case KEFIR_OPT_OPCODE_INT32_RSHIFT:
        case KEFIR_OPT_OPCODE_INT64_RSHIFT:
        case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
        case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
        case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
        case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
        case KEFIR_OPT_OPCODE_SCALAR_COMPARE:
            return 2;

        case KEFIR_OPT_OPCODE_INT8_NEG:
        case KEFIR_OPT_OPCODE_INT16_NEG:
        case KEFIR_OPT_OPCODE_INT32_NEG:
        case KEFIR_OPT_OPCODE_INT64_NEG:
        case KEFIR_OPT_OPCODE_INT8_NOT:
        case KEFIR_OPT_OPCODE_INT16_NOT:
        case KEFIR_OPT_OPCODE_INT32_NOT:
        case KEFIR_OPT_OPCODE_INT64_NOT:
        case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
            return 1;

        default:
            return -1;
    }
}

static kefir_result_t do_factor(const struct kefir_opt_instruction *instr1, const struct kefir_opt_instruction *instr2,
                                kefir_size_t *factor_idx) {
    REQUIRE(instr1->operation.opcode == instr2->operation.opcode,
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match factored instructions"));

    switch (instr1->operation.opcode) {
        case KEFIR_OPT_OPCODE_SCALAR_COMPARE:
            REQUIRE(instr1->operation.parameters.comparison == instr2->operation.parameters.comparison,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match factored instructions"));
            break;

        default:
            // Intentionally left blank
            break;
    }

    kefir_size_t params = is_factored_computation(instr1);
    for (kefir_size_t i = 0; i < params; i++) {
        if (instr1->operation.parameters.refs[i] != instr2->operation.parameters.refs[i]) {
            REQUIRE(*factor_idx == ~0ull || i == *factor_idx,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match factored instructions"));
            *factor_idx = i;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t phi_factor_apply(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                       kefir_opt_block_id_t block_ref, kefir_opt_instruction_ref_t phi_instr_ref,
                                       kefir_bool_t *fixpoint_reached) {
    kefir_opt_instruction_ref_t computation_instr_ref = KEFIR_ID_NONE;
    const struct kefir_opt_instruction *computation_instr = NULL;
    kefir_size_t factor_idx = ~0ull;

    kefir_result_t res;
    struct kefir_opt_phi_node_link_iterator link_iter;
    kefir_opt_block_id_t link_block_id;
    kefir_opt_instruction_ref_t link_instr_ref;
    for (res = kefir_opt_phi_node_link_iter(code, phi_instr_ref, &link_iter, &link_block_id, &link_instr_ref);
         res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {
        kefir_bool_t movable;
        REQUIRE_OK(kefir_opt_instruction_is_moveable(code, link_instr_ref, &movable));
        REQUIRE(movable, KEFIR_OK);

        kefir_opt_instruction_ref_t sole_use_ref;
        REQUIRE_OK(kefir_opt_instruction_get_sole_use(code, link_instr_ref, &sole_use_ref));
        REQUIRE(sole_use_ref == phi_instr_ref, KEFIR_OK);

        const struct kefir_opt_instruction *link_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, link_instr_ref, &link_instr));
        if (computation_instr_ref == KEFIR_ID_NONE) {
            REQUIRE(is_factored_computation(link_instr) != -1, KEFIR_OK);
            computation_instr_ref = link_instr_ref;
            computation_instr = link_instr;
        } else {
            res = do_factor(computation_instr, link_instr, &factor_idx);
            REQUIRE(res != KEFIR_NO_MATCH, KEFIR_OK);
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE(computation_instr_ref != KEFIR_ID_NONE, KEFIR_OK);
    REQUIRE(factor_idx != ~0ull, KEFIR_OK);

    struct kefir_opt_operation oper = computation_instr->operation;

    kefir_opt_instruction_ref_t factor_phi;
    REQUIRE_OK(kefir_opt_code_container_new_phi(mem, code, block_ref, &factor_phi));
    oper.parameters.refs[factor_idx] = factor_phi;

    for (res = kefir_opt_phi_node_link_iter(code, phi_instr_ref, &link_iter, &link_block_id, &link_instr_ref);
         res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {
        const struct kefir_opt_instruction *link_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, link_instr_ref, &link_instr));
        REQUIRE_OK(kefir_opt_code_container_phi_attach(mem, code, factor_phi, link_block_id,
                                                       link_instr->operation.parameters.refs[factor_idx]));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_opt_instruction_ref_t replacement_ref;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(mem, code, block_ref, &oper, &replacement_ref));
    REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, replacement_ref, phi_instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, code, phi_instr_ref));
    *fixpoint_reached = false;
    return KEFIR_OK;
}

static kefir_result_t late_cleanup_impl(struct kefir_mem *mem, struct kefir_opt_code_container *code) {
    kefir_bool_t fixpoint_reached = false;

    for (; !fixpoint_reached;) {
        fixpoint_reached = true;

        for (kefir_opt_block_id_t block_ref = 0; block_ref < kefir_opt_code_container_block_count(code); block_ref++) {
            if (block_ref == code->gate_block) {
                continue;
            }
            kefir_result_t res;
            kefir_opt_instruction_ref_t phi_instr_ref;
            for (res = kefir_opt_code_block_phi_head(code, block_ref, &phi_instr_ref);
                 res == KEFIR_OK && phi_instr_ref != KEFIR_ID_NONE;) {
                kefir_opt_instruction_ref_t next_instr_ref;
                res = kefir_opt_phi_next_sibling(code, phi_instr_ref, &next_instr_ref);
                REQUIRE_OK(phi_factor_apply(mem, code, block_ref, phi_instr_ref, &fixpoint_reached));
                phi_instr_ref = next_instr_ref;
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t late_cleanup_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                         struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                         const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    REQUIRE_OK(late_cleanup_impl(mem, &func->code));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassLateCleanup = {
    .name = "late-cleanup", .apply = late_cleanup_apply, .payload = NULL};
