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
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/mem2reg_util.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/escape.h"
#include "kefir/optimizer/alias.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct sroa_state {
    struct kefir_opt_code_container *code;
    struct kefir_opt_code_debug_info *debug_info;
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_escape_analysis escapes;

    struct kefir_hashtable candidate_accesses;
    struct kefir_hashtable candidate_locations;
    struct kefir_hashset candidates;
};

kefir_result_t kefir_opt_code_util_classify_memory_access(const struct kefir_opt_instruction *,
                                                          kefir_opt_instruction_ref_t *, kefir_size_t *,
                                                          kefir_int64_t *);

static kefir_result_t sroa_scan_candidate(struct kefir_mem *mem, struct sroa_state *state,
                                          kefir_opt_instruction_ref_t candidate_ref) {
    const struct kefir_opt_instruction *candidate_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(state->code, candidate_ref, &candidate_instr));

    kefir_opt_instruction_ref_t location1_ref = KEFIR_ID_NONE, location2_ref = KEFIR_ID_NONE;
    kefir_size_t size1 = 0, size2 = 0;
    kefir_int64_t offset1 = 0, offset2 = 0;

    kefir_result_t res = kefir_opt_code_util_classify_memory_access(candidate_instr, &location1_ref, &size1, &offset1);
    if (res == KEFIR_NO_MATCH) {
        location1_ref = candidate_ref;
        size1 = 0;
        offset1 = 0;
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);

    kefir_uint64_t candidate_type;
    REQUIRE_OK(kefir_opt_code_util_mem2reg_classify_opcode(candidate_instr, &candidate_type));

    kefir_bool_t valid_candidate = true;

    struct kefir_hashtable_iterator iter;
    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    for (res = kefir_hashtable_iter(&state->candidate_accesses, &iter, &table_key, &table_value); res == KEFIR_OK;
         res = kefir_hashtable_next(&iter, &table_key, &table_value)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, current_candidate_ref, table_key);

        const struct kefir_opt_instruction *current_candidate_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, current_candidate_ref, &current_candidate_instr));

        kefir_uint64_t current_candidate_type;
        REQUIRE_OK(kefir_opt_code_util_mem2reg_classify_opcode(current_candidate_instr, &current_candidate_type));

        res = kefir_opt_code_util_classify_memory_access(current_candidate_instr, &location2_ref, &size2, &offset2);
        if (res == KEFIR_NO_MATCH) {
            location2_ref = current_candidate_ref;
            size2 = 0;
            offset2 = 0;
            res = KEFIR_OK;
        }
        REQUIRE_OK(res);

        kefir_bool_t may_alias, must_alias;
        REQUIRE_OK(kefir_opt_code_may_alias(state->code, &state->escapes, location1_ref, size1, offset1, location2_ref,
                                            size2, offset2, &may_alias));
        REQUIRE_OK(kefir_opt_code_must_alias(state->code, location1_ref, size1, offset1, location2_ref, size2, offset2,
                                             &must_alias));

        if (may_alias && (!table_value || !must_alias || size1 != size2 || candidate_type != current_candidate_type)) {
            valid_candidate = false;

            kefir_hashtree_value_t *value_ptr;
            REQUIRE_OK(kefir_hashtable_at_mut(&state->candidate_accesses, table_key, &value_ptr));
            *value_ptr = false;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE_OK(kefir_hashtable_insert(mem, &state->candidate_accesses, (kefir_hashtable_key_t) candidate_ref,
                                      (kefir_hashtable_value_t) valid_candidate));
    return KEFIR_OK;
}

static kefir_result_t sroa_scan(struct kefir_mem *mem, struct sroa_state *state,
                                kefir_opt_instruction_ref_t alloc_instr_ref, kefir_opt_instruction_ref_t instr_ref) {
    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    kefir_bool_t is_control_flow;
    kefir_uint64_t current_type;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(state->code, instr_ref, &use_iter); res == KEFIR_OK;
         res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, use_iter.use_instr_ref, &use_instr));
        switch (use_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_REF_LOCAL:
                REQUIRE_OK(sroa_scan(mem, state, alloc_instr_ref, use_iter.use_instr_ref));
                break;

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
            case KEFIR_OPT_OPCODE_BITINT_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
            case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
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
            case KEFIR_OPT_OPCODE_BITINT_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
            case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
                REQUIRE_OK(
                    kefir_opt_code_instruction_is_control_flow(state->code, use_iter.use_instr_ref, &is_control_flow));
                REQUIRE_OK(kefir_opt_code_util_mem2reg_classify_opcode(use_instr, &current_type));
                REQUIRE(is_control_flow && !use_instr->operation.parameters.memory_access.flags.volatile_access,
                        KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match sroa candidate"));
                REQUIRE_OK(sroa_scan_candidate(mem, state, use_iter.use_instr_ref));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match sroa candidate");
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t sroa_prepare_locations(struct kefir_mem *mem, struct sroa_state *state) {
    kefir_result_t res;
    struct kefir_hashtable_iterator iter;
    kefir_hashtable_key_t table_key;
    kefir_hashtable_value_t table_value;
    for (res = kefir_hashtable_iter(&state->candidate_accesses, &iter, &table_key, &table_value); res == KEFIR_OK;
         res = kefir_hashtable_next(&iter, &table_key, &table_value)) {
        if (!table_value) {
            continue;
        }
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, candidate_ref, table_key);

        const struct kefir_opt_instruction *candidate_instr, *location_instr, *alloc_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, candidate_ref, &candidate_instr));
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, candidate_instr->operation.parameters.refs[0],
                                                  &location_instr));

        if (location_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
            REQUIRE_OK(kefir_hashset_add(mem, &state->candidates, (kefir_hashset_key_t) location_instr->id));
        } else if (location_instr->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL &&
                   location_instr->operation.parameters.offset >= KEFIR_INT32_MIN &&
                   location_instr->operation.parameters.offset <= KEFIR_INT32_MAX) {
            kefir_uint64_t key = (((kefir_uint64_t) location_instr->operation.parameters.refs[0]) << 32) |
                                 (kefir_uint32_t) location_instr->operation.parameters.offset;

            res = kefir_hashtable_at(&state->candidate_locations, (kefir_hashtable_key_t) key, &table_value);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                REQUIRE_OK(kefir_opt_code_container_replace_references(
                    mem, state->code, (kefir_opt_instruction_ref_t) table_value, location_instr->id));
            } else {
                kefir_opt_instruction_ref_t canonical_ref, location_ref = location_instr->id;
                if (location_instr->operation.parameters.offset != 0) {
                    REQUIRE_OK(kefir_opt_code_container_instr(state->code, location_instr->operation.parameters.refs[0],
                                                              &alloc_instr));
                    REQUIRE_OK(kefir_opt_code_builder_ref_local(
                        mem, state->code, alloc_instr->block_id, location_instr->operation.parameters.refs[0],
                        location_instr->operation.parameters.offset, &canonical_ref));
                } else {
                    canonical_ref = location_instr->operation.parameters.refs[0];
                }
                REQUIRE_OK(kefir_opt_code_container_replace_references(
                    mem, state->code, (kefir_opt_instruction_ref_t) canonical_ref, location_ref));
                REQUIRE_OK(kefir_hashtable_insert(mem, &state->candidate_locations, (kefir_hashtable_key_t) key,
                                                  (kefir_hashtable_value_t) canonical_ref));
                REQUIRE_OK(kefir_hashset_add(mem, &state->candidates, (kefir_hashset_key_t) canonical_ref));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t sroa_impl(struct kefir_mem *mem, struct sroa_state *state) {
    REQUIRE_OK(kefir_opt_code_control_flow_build(mem, &state->control_flow, state->code));
    REQUIRE_OK(kefir_opt_code_escape_analysis_build(mem, &state->escapes, state->code));

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(state->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {
        kefir_bool_t is_reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(&state->control_flow, block->id, &is_reachable));
        if (!is_reachable) {
            continue;
        }

        kefir_opt_instruction_ref_t instr_ref;
        for (kefir_opt_code_block_instr_head(state->code, block->id, &instr_ref); instr_ref != KEFIR_ID_NONE;
             kefir_opt_instruction_next_sibling(state->code, instr_ref, &instr_ref)) {

            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
                !kefir_opt_code_escape_analysis_has_escapes(&state->escapes, instr_ref)) {
                REQUIRE_OK(kefir_hashtable_clear(mem, &state->candidate_accesses));
                REQUIRE_OK(kefir_hashtable_clear(mem, &state->candidate_locations));
                kefir_result_t res = sroa_scan(mem, state, instr_ref, instr_ref);
                if (res != KEFIR_NO_MATCH) {
                    REQUIRE_OK(res);
                }
                REQUIRE_OK(sroa_prepare_locations(mem, state));
            }
        }
    }

    REQUIRE_OK(kefir_opt_code_util_mem2reg_apply(mem, state->code, state->debug_info, &state->control_flow,
                                                 &state->candidates));
    return KEFIR_OK;
}

static kefir_result_t sroa_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                 struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                 const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct sroa_state state = {.code = &func->code, .debug_info = &func->debug_info};
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_opt_code_escape_analysis_init(&state.escapes));
    REQUIRE_OK(kefir_hashtable_init(&state.candidate_accesses, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.candidate_locations, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.candidates, &kefir_hashtable_uint_ops));

    kefir_result_t res = sroa_impl(mem, &state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.candidates);
        kefir_hashtable_free(mem, &state.candidate_locations);
        kefir_hashtable_free(mem, &state.candidate_accesses);
        kefir_opt_code_escape_analysis_free(mem, &state.escapes);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_hashset_free(mem, &state.candidates);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.candidate_locations);
        kefir_hashtable_free(mem, &state.candidate_accesses);
        kefir_opt_code_escape_analysis_free(mem, &state.escapes);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.candidate_locations);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.candidate_accesses);
        kefir_opt_code_escape_analysis_free(mem, &state.escapes);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.candidate_accesses);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_escape_analysis_free(mem, &state.escapes);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_opt_code_escape_analysis_free(mem, &state.escapes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &state.control_flow));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassSROA = {.name = "sroa", .apply = sroa_apply, .payload = NULL};
