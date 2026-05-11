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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/rematerialize.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/loop_nest.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct remat_params {
    struct kefir_mem *mem;
    struct kefir_opt_code_container *code;
    struct kefir_opt_code_debug_info *debug_info;
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_loop_collection loops;

    struct kefir_list queue;
    struct kefir_hashtable rematerializations;
    struct kefir_list remat_queue;
};

static kefir_result_t resolve_remat_at(struct remat_params *params, kefir_opt_block_id_t block_ref,
                                       kefir_opt_instruction_ref_t instr_ref,
                                       kefir_opt_instruction_ref_t *remat_instr_ref) {
    kefir_hashtable_value_t table_value;
    kefir_result_t res =
        kefir_hashtable_at(&params->rematerializations, (kefir_hashtable_key_t) block_ref, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *remat_instr_ref = (kefir_opt_instruction_ref_t) table_value;
    } else {
        REQUIRE_OK(kefir_opt_code_container_copy_instruction(params->mem, params->code, block_ref, instr_ref,
                                                             remat_instr_ref));
        REQUIRE_OK(kefir_hashtable_insert(params->mem, &params->rematerializations, (kefir_hashtable_key_t) block_ref,
                                          (kefir_hashtable_value_t) *remat_instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t remat_instr(struct remat_params *params, kefir_opt_instruction_ref_t instr_ref) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(params->code, instr_ref, &instr));

    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    REQUIRE_OK(kefir_hashtable_clear(params->mem, &params->rematerializations));
    REQUIRE_OK(kefir_list_clear(params->mem, &params->remat_queue));
    for (res = kefir_opt_code_container_instruction_use_instr_iter(params->code, instr_ref, &use_iter); res == KEFIR_OK;
         res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        REQUIRE_OK(kefir_list_insert_after(params->mem, &params->remat_queue, NULL,
                                           (void *) (kefir_uptr_t) use_iter.use_instr_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&params->remat_queue); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, use_instr_ref, (kefir_uptr_t) iter->value);

        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(params->code, use_instr_ref, &use_instr));
        kefir_opt_block_id_t use_block_ref = use_instr->block_id;

        if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
            struct kefir_opt_phi_node_link_iterator link_iter;
            kefir_opt_block_id_t link_block_id;
            kefir_opt_instruction_ref_t link_instr_ref;
            for (res = kefir_opt_phi_node_link_iter(params->code, use_instr_ref, &link_iter, &link_block_id,
                                                    &link_instr_ref);
                 res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {
                if (link_instr_ref == instr_ref) {
                    kefir_opt_instruction_ref_t remat_instr_ref = KEFIR_ID_NONE;
                    REQUIRE_OK(resolve_remat_at(params, link_block_id, instr_ref, &remat_instr_ref));
                    REQUIRE_OK(kefir_opt_code_container_phi_replace(params->mem, params->code, use_instr_ref,
                                                                    link_block_id, remat_instr_ref));
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } else {
            kefir_opt_instruction_ref_t remat_instr_ref = KEFIR_ID_NONE;
            REQUIRE_OK(resolve_remat_at(params, use_block_ref, instr_ref, &remat_instr_ref));
            REQUIRE_OK(kefir_opt_code_container_replace_references_in(params->mem, params->code, use_instr_ref,
                                                                      remat_instr_ref, instr_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t remat_impl(struct remat_params *params) {
    REQUIRE_OK(kefir_opt_code_control_flow_build(params->mem, &params->control_flow, params->code));
    REQUIRE_OK(kefir_opt_code_loop_collection_build(params->mem, &params->loops, &params->control_flow));

    kefir_result_t res;
    for (kefir_opt_block_id_t block_ref = 0; block_ref < kefir_opt_code_container_block_count(params->code);
         block_ref++) {
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(params->code, block_ref, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(params->code, instr_ref, &instr_ref)) {
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(params->code, instr_ref, &instr));
            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT_CONST:
                case KEFIR_OPT_OPCODE_UINT_CONST:
                case KEFIR_OPT_OPCODE_FLOAT32_CONST:
                case KEFIR_OPT_OPCODE_FLOAT64_CONST:
                case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
                case KEFIR_OPT_OPCODE_BLOCK_LABEL:
                case KEFIR_OPT_OPCODE_STRING_REF:
                case KEFIR_OPT_OPCODE_REF_LOCAL:
                case KEFIR_OPT_OPCODE_GET_GLOBAL:
                case KEFIR_OPT_OPCODE_GET_THREAD_LOCAL:
                    REQUIRE_OK(
                        kefir_list_insert_after(params->mem, &params->queue, NULL, (void *) (kefir_uptr_t) instr_ref));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&params->queue); iter != NULL;
         iter = kefir_list_head(&params->queue)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(params->mem, &params->queue, iter));
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(params->code, instr_ref, &instr));

        REQUIRE_OK(remat_instr(params, instr_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_rematerialize_function(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                          struct kefir_opt_function *func,
                                                          const struct kefir_optimizer_configuration *configuration) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid optimizer function"));
    REQUIRE(configuration != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expectd valid optimizer configuration"));

    struct remat_params params = {
        .mem = mem,
        .code = &func->code,
        .debug_info = &func->debug_info,
    };
    REQUIRE_OK(kefir_opt_code_control_flow_init(&params.control_flow));
    REQUIRE_OK(kefir_opt_code_loop_collection_init(&params.loops));
    REQUIRE_OK(kefir_list_init(&params.queue));
    REQUIRE_OK(kefir_list_init(&params.remat_queue));
    REQUIRE_OK(kefir_hashtable_init(&params.rematerializations, &kefir_hashtable_uint_ops));

    kefir_result_t res = remat_impl(&params);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_loop_collection_free(mem, &params.loops);
        kefir_opt_code_control_flow_free(mem, &params.control_flow);
        kefir_hashtable_free(params.mem, &params.rematerializations);
        kefir_list_free(mem, &params.remat_queue);
        kefir_list_free(mem, &params.queue);
        return res;
    });
    res = kefir_opt_code_loop_collection_free(mem, &params.loops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &params.control_flow);
        kefir_hashtable_free(params.mem, &params.rematerializations);
        kefir_list_free(mem, &params.remat_queue);
        kefir_list_free(mem, &params.queue);
        return res;
    });
    res = kefir_opt_code_control_flow_free(mem, &params.control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(params.mem, &params.rematerializations);
        kefir_list_free(mem, &params.remat_queue);
        kefir_list_free(mem, &params.queue);
        return res;
    });
    res = kefir_hashtable_free(params.mem, &params.rematerializations);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &params.remat_queue);
        kefir_list_free(mem, &params.queue);
        return res;
    });
    res = kefir_list_free(mem, &params.remat_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &params.queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &params.queue));
    return KEFIR_OK;
}
