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

#include "kefir/optimizer/code.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/mem2reg_util.h"
#include "kefir/ir/module.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_code_util_extend_load_value(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                     const struct kefir_opt_instruction *instr,
                                                     kefir_opt_instruction_ref_t value_ref,
                                                     kefir_opt_instruction_ref_t *ext_instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(ext_instr_ref != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instructionn reference"));

    *ext_instr_ref = value_ref;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_LOAD:
            if (instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND) {
                REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_8bits(mem, code, instr->block_id, value_ref,
                                                                          ext_instr_ref));
            } else if (instr->operation.parameters.memory_access.flags.load_extension ==
                       KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND) {
                REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_8bits(mem, code, instr->block_id, value_ref,
                                                                          ext_instr_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_INT16_LOAD:
            if (instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND) {
                REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_16bits(mem, code, instr->block_id, value_ref,
                                                                           ext_instr_ref));
            } else if (instr->operation.parameters.memory_access.flags.load_extension ==
                       KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND) {
                REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_16bits(mem, code, instr->block_id, value_ref,
                                                                           ext_instr_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_INT32_LOAD:
            if (instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND) {
                REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_32bits(mem, code, instr->block_id, value_ref,
                                                                           ext_instr_ref));
            } else if (instr->operation.parameters.memory_access.flags.load_extension ==
                       KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND) {
                REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_32bits(mem, code, instr->block_id, value_ref,
                                                                           ext_instr_ref));
            }
            break;

        case KEFIR_OPT_OPCODE_INT64_LOAD:
        case KEFIR_OPT_OPCODE_INT128_LOAD:
        case KEFIR_OPT_OPCODE_BITINT_LOAD:
        case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
        case KEFIR_OPT_OPCODE_FLOAT32_LOAD:
        case KEFIR_OPT_OPCODE_FLOAT64_LOAD:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
            // Intentionally left blank
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected load instruction opcode");
    }
    return KEFIR_OK;
}

struct mem2reg_state {
    struct kefir_mem *mem;
    struct kefir_opt_code_container *code;
    struct kefir_opt_code_debug_info *debug_info;
    const struct kefir_opt_code_control_flow *control_flow;
    const struct kefir_hashset *candidates;
    struct kefir_list block_queue;
    struct kefir_hashset visited_blocks;
    struct kefir_hashtable inserted_phis;
    struct kefir_hashtable candidate_types;
};

enum {
    CANDIDATE_INT = 1,
    CANDIDATE_INT128,
    CANDIDATE_FLOAT32,
    CANDIDATE_FLOAT64,
    CANDIDATE_LONG_DOUBLE,
    CANDIDATE_COMPLEX_FLOAT32,
    CANDIDATE_COMPLEX_FLOAT64,
    CANDIDATE_COMPLEX_LONG_DOUBLE,
    CANDIDATE_DECIMAL32,
    CANDIDATE_DECIMAL64,
    CANDIDATE_DECIMAL128,
    CANDIDATE_BITINT
};

static kefir_result_t mem2reg_collect_def_blocks(struct mem2reg_state *state, kefir_opt_instruction_ref_t instr_ref) {
    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(state->code, instr_ref, &use_iter); res == KEFIR_OK;
         res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, use_iter.use_instr_ref, &use_instr));

        switch (use_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT8_STORE:
            case KEFIR_OPT_OPCODE_INT16_STORE:
            case KEFIR_OPT_OPCODE_INT32_STORE:
            case KEFIR_OPT_OPCODE_INT64_STORE:
            case KEFIR_OPT_OPCODE_INT128_STORE:
            case KEFIR_OPT_OPCODE_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
                if (instr_ref == use_instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF]) {
                    if (!kefir_hashset_has(&state->visited_blocks, (kefir_hashset_key_t) use_instr->block_id)) {
                        REQUIRE_OK(kefir_hashset_add(state->mem, &state->visited_blocks,
                                                     (kefir_hashset_key_t) use_instr->block_id));
                        REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue, NULL,
                                                           (void *) (kefir_uptr_t) use_instr->block_id));
                    }
                }
                break;

            default:
                // Intentionally left blank
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_insert_phis(struct mem2reg_state *state, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->visited_blocks));
    for (struct kefir_list_entry *iter = kefir_list_head(&state->block_queue); iter != NULL;
         iter = kefir_list_head(&state->block_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(state->mem, &state->block_queue, iter));
        if (kefir_hashset_has(&state->visited_blocks, (kefir_hashset_key_t) block_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(state->mem, &state->visited_blocks, (kefir_hashset_key_t) block_ref));

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t entry;
        for (res = kefir_hashset_iter(&state->control_flow->blocks[block_ref].dominance_frontier, &iter, &entry);
             res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, frontier_block_ref, entry);
            kefir_uint64_t key = (((kefir_uint64_t) instr_ref) << 32) | (kefir_uint32_t) frontier_block_ref;
            if (frontier_block_ref == state->control_flow->code->entry_point ||
                kefir_hashtable_has(&state->inserted_phis, (kefir_hashtable_key_t) key)) {
                continue;
            }

            kefir_opt_instruction_ref_t phi_instr_ref;
            REQUIRE_OK(kefir_opt_code_container_new_phi(state->mem, state->code, frontier_block_ref, &phi_instr_ref));
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->inserted_phis, (kefir_hashtable_key_t) key,
                                              (kefir_hashtable_value_t) phi_instr_ref));
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue, NULL,
                                               (void *) (kefir_uptr_t) frontier_block_ref));

            REQUIRE_OK(kefir_opt_code_debug_info_add_allocation_placement(state->mem, state->debug_info, instr_ref,
                                                                          phi_instr_ref));
        }
    }
    return KEFIR_OK;
}

struct mem2reg_link_frame {
    kefir_opt_block_id_t block_ref;
    kefir_bool_t unfolded;
    struct kefir_hashtable content;
    struct mem2reg_link_frame *parent;
};

static kefir_result_t mem2reg_push_link_frame(struct mem2reg_state *state, kefir_opt_block_id_t block_ref,
                                              struct mem2reg_link_frame *parent) {
    struct mem2reg_link_frame *frame = KEFIR_MALLOC(state->mem, sizeof(struct mem2reg_link_frame));
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR phi link frame"));
    frame->block_ref = block_ref;
    frame->unfolded = false;
    frame->parent = parent;
    kefir_result_t res = kefir_hashtable_init(&frame->content, &kefir_hashtable_uint_ops);
    REQUIRE_CHAIN(&res, kefir_list_insert_after(state->mem, &state->block_queue, NULL, frame));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(state->mem, frame);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_util_mem2reg_classify_opcode(const struct kefir_opt_instruction *use_instr,
                                                           kefir_uint64_t *type) {
    REQUIRE(use_instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(type != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to mem2reg instruction classification"));

    switch (use_instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_LOAD:
        case KEFIR_OPT_OPCODE_INT16_LOAD:
        case KEFIR_OPT_OPCODE_INT32_LOAD:
        case KEFIR_OPT_OPCODE_INT64_LOAD:
        case KEFIR_OPT_OPCODE_INT8_STORE:
        case KEFIR_OPT_OPCODE_INT16_STORE:
        case KEFIR_OPT_OPCODE_INT32_STORE:
        case KEFIR_OPT_OPCODE_INT64_STORE:
            *type = CANDIDATE_INT;
            break;

        case KEFIR_OPT_OPCODE_INT128_LOAD:
        case KEFIR_OPT_OPCODE_INT128_STORE:
            *type = CANDIDATE_INT128;
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_LOAD:
        case KEFIR_OPT_OPCODE_FLOAT32_STORE:
            *type = CANDIDATE_FLOAT32;
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_LOAD:
        case KEFIR_OPT_OPCODE_FLOAT64_STORE:
            *type = CANDIDATE_FLOAT64;
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            *type = CANDIDATE_LONG_DOUBLE;
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
            *type = CANDIDATE_COMPLEX_FLOAT32;
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
            *type = CANDIDATE_COMPLEX_FLOAT64;
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            *type = CANDIDATE_COMPLEX_LONG_DOUBLE;
            break;

        case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            *type = CANDIDATE_DECIMAL32;
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            *type = CANDIDATE_DECIMAL64;
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            *type = CANDIDATE_DECIMAL128;
            break;

        case KEFIR_OPT_OPCODE_BITINT_LOAD:
        case KEFIR_OPT_OPCODE_BITINT_STORE:
        case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
        case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
            *type = ((kefir_uint64_t) use_instr->operation.parameters.bitwidth) << 32 | CANDIDATE_BITINT;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match use opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_assign_placeholder_value(struct mem2reg_state *state,
                                                       kefir_opt_instruction_ref_t original_instr_ref,
                                                       kefir_opt_block_id_t source_block_ref,
                                                       const struct kefir_opt_instruction *use_instr,
                                                       kefir_opt_instruction_ref_t *instr_ref) {
    kefir_uint64_t type = 0;
    if (use_instr != NULL) {
        REQUIRE_OK(kefir_opt_code_util_mem2reg_classify_opcode(use_instr, &type));
    } else {
        kefir_hashtable_value_t table_value;
        REQUIRE_OK(
            kefir_hashtable_at(&state->candidate_types, (kefir_hashtable_key_t) original_instr_ref, &table_value));
        type = (kefir_uint64_t) table_value;
    }

    switch ((kefir_uint32_t) type) {
        case CANDIDATE_INT:
            REQUIRE_OK(kefir_opt_code_builder_int_placeholder(state->mem, state->code, source_block_ref, instr_ref));
            break;

        case CANDIDATE_INT128:
            REQUIRE_OK(kefir_opt_code_builder_int_placeholder(state->mem, state->code, source_block_ref, instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_int128_zero_extend_64bits(state->mem, state->code, source_block_ref,
                                                                        *instr_ref, instr_ref));
            break;

        case CANDIDATE_FLOAT32:
            REQUIRE_OK(
                kefir_opt_code_builder_float32_placeholder(state->mem, state->code, source_block_ref, instr_ref));
            break;

        case CANDIDATE_FLOAT64:
            REQUIRE_OK(
                kefir_opt_code_builder_float64_placeholder(state->mem, state->code, source_block_ref, instr_ref));
            break;

        case CANDIDATE_LONG_DOUBLE:
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(state->mem, state->code, source_block_ref, 0.0L,
                                                                   instr_ref));
            break;

        case CANDIDATE_COMPLEX_FLOAT32: {
            kefir_opt_instruction_ref_t zero_instr_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_float32_placeholder(state->mem, state->code, source_block_ref, &zero_instr_ref));

            REQUIRE_OK(kefir_opt_code_builder_complex_float32_from(state->mem, state->code, source_block_ref,
                                                                   zero_instr_ref, zero_instr_ref, instr_ref));
        } break;

        case CANDIDATE_COMPLEX_FLOAT64: {
            kefir_opt_instruction_ref_t zero_instr_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_float64_placeholder(state->mem, state->code, source_block_ref, &zero_instr_ref));

            REQUIRE_OK(kefir_opt_code_builder_complex_float64_from(state->mem, state->code, source_block_ref,
                                                                   zero_instr_ref, zero_instr_ref, instr_ref));
        } break;

        case CANDIDATE_COMPLEX_LONG_DOUBLE: {
            kefir_opt_instruction_ref_t zero_instr_ref;
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(state->mem, state->code, source_block_ref, 0.0L,
                                                                   &zero_instr_ref));

            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_from(state->mem, state->code, source_block_ref,
                                                                       zero_instr_ref, zero_instr_ref, instr_ref));
        } break;

        case CANDIDATE_BITINT: {
            kefir_opt_instruction_ref_t zero_instr_ref;
            REQUIRE_OK(
                kefir_opt_code_builder_int_placeholder(state->mem, state->code, source_block_ref, &zero_instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_bitint_from_unsigned(state->mem, state->code, source_block_ref,
                                                                   type >> 32, zero_instr_ref, instr_ref));
        } break;

        case CANDIDATE_DECIMAL32:
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            REQUIRE_OK(kefir_opt_code_builder_decimal32_constant(state->mem, state->code, source_block_ref,
                                                                 kefir_dfp_decimal32_from_int64(0), instr_ref));
            break;

        case CANDIDATE_DECIMAL64:
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            REQUIRE_OK(kefir_opt_code_builder_decimal64_constant(state->mem, state->code, source_block_ref,
                                                                 kefir_dfp_decimal64_from_int64(0), instr_ref));
            break;

        case CANDIDATE_DECIMAL128:
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            REQUIRE_OK(kefir_opt_code_builder_decimal128_constant(state->mem, state->code, source_block_ref,
                                                                  kefir_dfp_decimal128_from_int64(0), instr_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected mem2reg candidate type");
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_find_link_for(struct mem2reg_state *state, struct mem2reg_link_frame *frame,
                                            kefir_opt_instruction_ref_t alloc_instr_ref,
                                            kefir_opt_block_id_t base_block_ref,
                                            const struct kefir_opt_instruction *use_instr,
                                            kefir_opt_instruction_ref_t *link_ref) {
    for (; frame != NULL; frame = frame->parent) {
        kefir_hashtable_value_t table_value;
        kefir_result_t res = kefir_hashtable_at(&frame->content, (kefir_hashtable_key_t) alloc_instr_ref, &table_value);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            *link_ref = table_value;
            return KEFIR_OK;
        }
    }

    REQUIRE_OK(mem2reg_assign_placeholder_value(state, alloc_instr_ref, base_block_ref, use_instr, link_ref));
    return KEFIR_OK;
}

static kefir_result_t mem2reg_link_successor_phis(struct mem2reg_state *state, struct mem2reg_link_frame *frame,
                                                  kefir_opt_block_id_t block_ref) {
    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(&state->control_flow->blocks[block_ref].successors, &iter, &key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_ref, (kefir_uptr_t) key);

        struct kefir_hashset_iterator iter2;
        kefir_hashset_key_t entry2;
        for (res = kefir_hashset_iter(state->candidates, &iter2, &entry2); res == KEFIR_OK;
             res = kefir_hashset_next(&iter2, &entry2)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry2);

            kefir_uint64_t key = (((kefir_uint64_t) instr_ref) << 32) | (kefir_uint32_t) successor_block_ref;
            kefir_hashtable_value_t table_value;
            res = kefir_hashtable_at(&state->inserted_phis, key, &table_value);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                kefir_opt_instruction_ref_t link_ref;
                REQUIRE_OK(mem2reg_find_link_for(state, frame, instr_ref, block_ref, NULL, &link_ref));

                REQUIRE_OK(kefir_opt_code_container_phi_attach(
                    state->mem, state->code, (kefir_opt_instruction_ref_t) table_value, block_ref, link_ref));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t mem2reg_assign(struct mem2reg_state *state, struct mem2reg_link_frame *frame) {
    kefir_result_t res;
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(state->code, frame->block_ref, &block));

    kefir_opt_instruction_ref_t instr_ref;
    for (res = kefir_opt_code_block_instr_control_head(state->code, frame->block_ref, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;) {
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));

        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT8_LOAD:
            case KEFIR_OPT_OPCODE_INT16_LOAD:
            case KEFIR_OPT_OPCODE_INT32_LOAD:
            case KEFIR_OPT_OPCODE_INT64_LOAD:
            case KEFIR_OPT_OPCODE_INT128_LOAD:
            case KEFIR_OPT_OPCODE_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
                if (kefir_hashset_has(
                        state->candidates,
                        (kefir_hashset_key_t) instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF])) {
                    kefir_opt_instruction_ref_t link_ref;
                    REQUIRE_OK(mem2reg_find_link_for(
                        state, frame, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        instr->block_id, instr, &link_ref));
                    REQUIRE_OK(
                        kefir_opt_code_util_extend_load_value(state->mem, state->code, instr, link_ref, &link_ref));
                    REQUIRE_OK(
                        kefir_opt_code_container_replace_references(state->mem, state->code, link_ref, instr_ref));
                    kefir_opt_instruction_ref_t del_instr_ref = instr_ref;
                    res = kefir_opt_instruction_next_control(state->code, instr_ref, &instr_ref);
                    REQUIRE_OK(kefir_opt_code_container_drop_control(state->code, del_instr_ref));
                    REQUIRE_OK(kefir_opt_code_container_drop_instr(state->mem, state->code, del_instr_ref));
                } else {
                    res = kefir_opt_instruction_next_control(state->code, instr_ref, &instr_ref);
                }
                break;

            case KEFIR_OPT_OPCODE_INT8_STORE:
            case KEFIR_OPT_OPCODE_INT16_STORE:
            case KEFIR_OPT_OPCODE_INT32_STORE:
            case KEFIR_OPT_OPCODE_INT64_STORE:
            case KEFIR_OPT_OPCODE_INT128_STORE:
            case KEFIR_OPT_OPCODE_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
                if (kefir_hashset_has(
                        state->candidates,
                        (kefir_hashset_key_t) instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF])) {
                    kefir_hashtable_value_t *table_value;
                    res = kefir_hashtable_at_mut(
                        &frame->content,
                        (kefir_hashtable_key_t) instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        &table_value);
                    if (res != KEFIR_NOT_FOUND) {
                        REQUIRE_OK(res);
                        *table_value = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF];
                    } else {
                        REQUIRE_OK(kefir_hashtable_insert(
                            state->mem, &frame->content,
                            (kefir_hashtable_key_t)
                                instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                            (kefir_hashtable_value_t)
                                instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]));
                    }
                    kefir_opt_instruction_ref_t del_instr_ref = instr_ref;
                    res = kefir_opt_instruction_next_control(state->code, instr_ref, &instr_ref);
                    REQUIRE_OK(kefir_opt_code_container_drop_control(state->code, del_instr_ref));
                    REQUIRE_OK(kefir_opt_code_container_drop_instr(state->mem, state->code, del_instr_ref));

                    REQUIRE_OK(kefir_opt_code_debug_info_add_allocation_placement(
                        state->mem, state->debug_info,
                        instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]));
                } else {
                    res = kefir_opt_instruction_next_control(state->code, instr_ref, &instr_ref);
                }
                break;

            default:
                res = kefir_opt_instruction_next_control(state->code, instr_ref, &instr_ref);
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t free_link_phi_frame(struct kefir_mem *mem, struct kefir_list *list,
                                          struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct mem2reg_link_frame *, frame, entry->value);
    REQUIRE(frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR phi link frame"));

    REQUIRE_OK(kefir_hashtable_free(mem, &frame->content));
    KEFIR_FREE(mem, frame);
    return KEFIR_OK;
}

static kefir_result_t mem2reg_link(struct mem2reg_state *state) {
    REQUIRE_OK(kefir_list_clear(state->mem, &state->block_queue));
    REQUIRE_OK(kefir_list_on_remove(&state->block_queue, free_link_phi_frame, NULL));

    REQUIRE_OK(mem2reg_push_link_frame(state, state->code->entry_point, NULL));
    for (struct kefir_list_entry *iter = kefir_list_head(&state->block_queue); iter != NULL;
         iter = kefir_list_head(&state->block_queue)) {
        ASSIGN_DECL_CAST(struct mem2reg_link_frame *, frame, iter->value);

        if (!frame->unfolded) {
            kefir_result_t res;
            struct kefir_hashset_iterator iter2;
            kefir_hashset_key_t entry2;
            for (res = kefir_hashset_iter(state->candidates, &iter2, &entry2); res == KEFIR_OK;
                 res = kefir_hashset_next(&iter2, &entry2)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, original_instr_ref, entry2);

                kefir_uint64_t key = (((kefir_uint64_t) original_instr_ref) << 32) | (kefir_uint32_t) frame->block_ref;
                kefir_hashtable_value_t table_value;
                res = kefir_hashtable_at(&state->inserted_phis, key, &table_value);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    REQUIRE_OK(kefir_hashtable_insert(state->mem, &frame->content,
                                                      (kefir_hashtable_key_t) original_instr_ref,
                                                      (kefir_opt_instruction_ref_t) table_value));
                }
            }

            REQUIRE_OK(mem2reg_assign(state, frame));
            REQUIRE_OK(mem2reg_link_successor_phis(state, frame, frame->block_ref));

            struct kefir_opt_control_flow_dominator_tree_iterator iter;
            kefir_opt_block_id_t dominated_block_ref;
            for (res = kefir_opt_control_flow_dominator_tree_iter(state->control_flow, &iter, frame->block_ref,
                                                                  &dominated_block_ref);
                 res == KEFIR_OK; res = kefir_opt_control_flow_dominator_tree_next(&iter, &dominated_block_ref)) {
                REQUIRE_OK(mem2reg_push_link_frame(state, dominated_block_ref, frame));
            }

            frame->unfolded = true;
        } else {
            REQUIRE_OK(kefir_list_pop(state->mem, &state->block_queue, iter));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_scan_candidate(struct mem2reg_state *state, kefir_opt_instruction_ref_t instr_ref) {
    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(state->code, instr_ref, &use_iter); res == KEFIR_OK;
         res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, use_iter.use_instr_ref, &use_instr));

        kefir_uint64_t type = 0;
        kefir_result_t res = kefir_opt_code_util_mem2reg_classify_opcode(use_instr, &type);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);
            kefir_hashtable_value_t *table_value_ptr;
            kefir_result_t res =
                kefir_hashtable_at_mut(&state->candidate_types, (kefir_hashtable_key_t) instr_ref, &table_value_ptr);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                if (type == CANDIDATE_INT || *table_value_ptr == CANDIDATE_INT) {
                    *table_value_ptr = CANDIDATE_INT;
                } else {
                    REQUIRE(type == *table_value_ptr,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Conflicting mem2reg candidate types"));
                }
            } else {
                REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->candidate_types,
                                                  (kefir_hashtable_key_t) instr_ref, (kefir_hashtable_value_t) type));
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_do(struct mem2reg_state *state) {
    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t entry;
    for (res = kefir_hashset_iter(state->candidates, &iter, &entry); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);

        REQUIRE_OK(mem2reg_scan_candidate(state, instr_ref));
        REQUIRE_OK(kefir_hashset_clear(state->mem, &state->visited_blocks));
        REQUIRE_OK(kefir_list_clear(state->mem, &state->block_queue));
        REQUIRE_OK(mem2reg_collect_def_blocks(state, instr_ref));
        REQUIRE_OK(mem2reg_insert_phis(state, instr_ref));
    }

    REQUIRE_OK(mem2reg_link(state));

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_util_mem2reg_apply(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 struct kefir_opt_code_debug_info *debug_info,
                                                 const struct kefir_opt_code_control_flow *control_flow,
                                                 const struct kefir_hashset *candidates) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug info"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer control flow"));
    REQUIRE(candidates != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid mem2reg candidate set"));

    struct mem2reg_state state = {
        .mem = mem, .code = code, .debug_info = debug_info, .control_flow = control_flow, .candidates = candidates};
    REQUIRE_OK(kefir_hashset_init(&state.visited_blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&state.block_queue));
    REQUIRE_OK(kefir_hashtable_init(&state.inserted_phis, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.candidate_types, &kefir_hashtable_uint_ops));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, mem2reg_do(&state));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.candidate_types);
        kefir_hashtable_free(mem, &state.inserted_phis);
        kefir_list_free(mem, &state.block_queue);
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.candidate_types);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.inserted_phis);
        kefir_list_free(mem, &state.block_queue);
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.inserted_phis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.block_queue);
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    res = kefir_list_free(mem, &state.block_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.visited_blocks);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &state.visited_blocks));
    return KEFIR_OK;
}
