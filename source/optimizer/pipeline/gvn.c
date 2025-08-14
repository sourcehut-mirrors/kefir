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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct gvn_state {
    struct kefir_mem *mem;
    struct kefir_opt_function *func;
    struct kefir_opt_code_structure structure;
    struct kefir_list queue;
    struct kefir_hashtreeset queued_instr;
    struct kefir_hashtreeset processed_instr;
    struct kefir_hashtree instr_hashes;

    kefir_bool_t all_inputs_processed;
};

struct gvn_instr_list {
    struct kefir_list instr_refs;
};

#define MAGIC1 0x9e3779b97f4a7c15ull
#define MAGIC2 0xbf58476d1ce4e5b9ull
#define MAGIC3 0x94d049bb133111ebull
static kefir_uint64_t splitmix64(kefir_uint64_t value) {
    value += MAGIC1;
    value = (value ^ (value >> 30)) * MAGIC2;
    value = (value ^ (value >> 27)) * MAGIC3;
    return value ^ (value >> 31);
}

static kefir_uint64_t hash_instruction_impl(const struct kefir_opt_instruction *instr) {
    kefir_uint64_t hash = 0;

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_ADD:
        case KEFIR_OPT_OPCODE_INT16_ADD:
        case KEFIR_OPT_OPCODE_INT32_ADD:
        case KEFIR_OPT_OPCODE_INT64_ADD:
        case KEFIR_OPT_OPCODE_INT8_MUL:
        case KEFIR_OPT_OPCODE_INT16_MUL:
        case KEFIR_OPT_OPCODE_INT32_MUL:
        case KEFIR_OPT_OPCODE_INT64_MUL:
        case KEFIR_OPT_OPCODE_UINT8_MUL:
        case KEFIR_OPT_OPCODE_UINT16_MUL:
        case KEFIR_OPT_OPCODE_UINT32_MUL:
        case KEFIR_OPT_OPCODE_UINT64_MUL:
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
        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
            hash += splitmix64(instr->operation.opcode);
            hash ^= splitmix64(MIN(instr->operation.parameters.refs[0], instr->operation.parameters.refs[1]) + MAGIC1);
            hash ^= splitmix64(MAX(instr->operation.parameters.refs[0], instr->operation.parameters.refs[1]) + MAGIC2);
            break;

        case KEFIR_OPT_OPCODE_INT8_SUB:
        case KEFIR_OPT_OPCODE_INT16_SUB:
        case KEFIR_OPT_OPCODE_INT32_SUB:
        case KEFIR_OPT_OPCODE_INT64_SUB:
        case KEFIR_OPT_OPCODE_INT8_DIV:
        case KEFIR_OPT_OPCODE_INT16_DIV:
        case KEFIR_OPT_OPCODE_INT32_DIV:
        case KEFIR_OPT_OPCODE_INT64_DIV:
        case KEFIR_OPT_OPCODE_INT8_MOD:
        case KEFIR_OPT_OPCODE_INT16_MOD:
        case KEFIR_OPT_OPCODE_INT32_MOD:
        case KEFIR_OPT_OPCODE_INT64_MOD:
        case KEFIR_OPT_OPCODE_UINT8_DIV:
        case KEFIR_OPT_OPCODE_UINT16_DIV:
        case KEFIR_OPT_OPCODE_UINT32_DIV:
        case KEFIR_OPT_OPCODE_UINT64_DIV:
        case KEFIR_OPT_OPCODE_UINT8_MOD:
        case KEFIR_OPT_OPCODE_UINT16_MOD:
        case KEFIR_OPT_OPCODE_UINT32_MOD:
        case KEFIR_OPT_OPCODE_UINT64_MOD:
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
            hash += splitmix64(instr->operation.opcode);
            hash ^= splitmix64(instr->operation.parameters.refs[0] + MAGIC1);
            hash ^= splitmix64(instr->operation.parameters.refs[1] + MAGIC2);
            break;

        case KEFIR_OPT_OPCODE_INT8_NOT:
        case KEFIR_OPT_OPCODE_INT16_NOT:
        case KEFIR_OPT_OPCODE_INT32_NOT:
        case KEFIR_OPT_OPCODE_INT64_NOT:
        case KEFIR_OPT_OPCODE_INT8_NEG:
        case KEFIR_OPT_OPCODE_INT16_NEG:
        case KEFIR_OPT_OPCODE_INT32_NEG:
        case KEFIR_OPT_OPCODE_INT64_NEG:
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
            hash += splitmix64(instr->operation.opcode);
            hash ^= splitmix64(instr->operation.parameters.refs[0] + MAGIC1);
            break;

        default:
            hash += splitmix64(instr->id);
            break;
    }

    return hash;
}

static kefir_bool_t compare_instructions_impl(const struct kefir_opt_instruction *instr1,
                                              const struct kefir_opt_instruction *instr2) {
    if (instr1->id == instr2->id) {
        return true;
    }

    if (instr1->operation.opcode != instr2->operation.opcode) {
        return false;
    }

    switch (instr1->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_ADD:
        case KEFIR_OPT_OPCODE_INT16_ADD:
        case KEFIR_OPT_OPCODE_INT32_ADD:
        case KEFIR_OPT_OPCODE_INT64_ADD:
        case KEFIR_OPT_OPCODE_INT8_MUL:
        case KEFIR_OPT_OPCODE_INT16_MUL:
        case KEFIR_OPT_OPCODE_INT32_MUL:
        case KEFIR_OPT_OPCODE_INT64_MUL:
        case KEFIR_OPT_OPCODE_UINT8_MUL:
        case KEFIR_OPT_OPCODE_UINT16_MUL:
        case KEFIR_OPT_OPCODE_UINT32_MUL:
        case KEFIR_OPT_OPCODE_UINT64_MUL:
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
        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
            return MIN(instr1->operation.parameters.refs[0], instr1->operation.parameters.refs[1]) ==
                       MIN(instr2->operation.parameters.refs[0], instr2->operation.parameters.refs[1]) &&
                   MAX(instr1->operation.parameters.refs[0], instr1->operation.parameters.refs[1]) ==
                       MAX(instr2->operation.parameters.refs[0], instr2->operation.parameters.refs[1]);

        case KEFIR_OPT_OPCODE_INT8_SUB:
        case KEFIR_OPT_OPCODE_INT16_SUB:
        case KEFIR_OPT_OPCODE_INT32_SUB:
        case KEFIR_OPT_OPCODE_INT64_SUB:
        case KEFIR_OPT_OPCODE_INT8_DIV:
        case KEFIR_OPT_OPCODE_INT16_DIV:
        case KEFIR_OPT_OPCODE_INT32_DIV:
        case KEFIR_OPT_OPCODE_INT64_DIV:
        case KEFIR_OPT_OPCODE_INT8_MOD:
        case KEFIR_OPT_OPCODE_INT16_MOD:
        case KEFIR_OPT_OPCODE_INT32_MOD:
        case KEFIR_OPT_OPCODE_INT64_MOD:
        case KEFIR_OPT_OPCODE_UINT8_DIV:
        case KEFIR_OPT_OPCODE_UINT16_DIV:
        case KEFIR_OPT_OPCODE_UINT32_DIV:
        case KEFIR_OPT_OPCODE_UINT64_DIV:
        case KEFIR_OPT_OPCODE_UINT8_MOD:
        case KEFIR_OPT_OPCODE_UINT16_MOD:
        case KEFIR_OPT_OPCODE_UINT32_MOD:
        case KEFIR_OPT_OPCODE_UINT64_MOD:
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
            return instr1->operation.parameters.refs[0] == instr2->operation.parameters.refs[0] &&
                   instr1->operation.parameters.refs[1] == instr2->operation.parameters.refs[1];

        case KEFIR_OPT_OPCODE_INT8_NOT:
        case KEFIR_OPT_OPCODE_INT16_NOT:
        case KEFIR_OPT_OPCODE_INT32_NOT:
        case KEFIR_OPT_OPCODE_INT64_NOT:
        case KEFIR_OPT_OPCODE_INT8_NEG:
        case KEFIR_OPT_OPCODE_INT16_NEG:
        case KEFIR_OPT_OPCODE_INT32_NEG:
        case KEFIR_OPT_OPCODE_INT64_NEG:
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
            return instr1->operation.parameters.refs[0] == instr2->operation.parameters.refs[0];

        default:
            // Intentionally left blank
            break;
    }

    return false;
}

enum gvn_replacement_policy { GVN_REPLACEMENT_SKIP, GVN_REPLACEMENT_GLOBAL, GVN_REPLACEMENT_LOCAL };

static kefir_result_t instr_replacement_policy(struct gvn_state *state, const struct kefir_opt_instruction *instr,
                                               enum gvn_replacement_policy *policy) {
    kefir_bool_t is_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&state->func->code, instr->id, &is_control_flow));
    if (is_control_flow) {
        *policy = GVN_REPLACEMENT_SKIP;
        return KEFIR_OK;
    }

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_ADD:
        case KEFIR_OPT_OPCODE_INT16_ADD:
        case KEFIR_OPT_OPCODE_INT32_ADD:
        case KEFIR_OPT_OPCODE_INT64_ADD:
        case KEFIR_OPT_OPCODE_INT8_MUL:
        case KEFIR_OPT_OPCODE_INT16_MUL:
        case KEFIR_OPT_OPCODE_INT32_MUL:
        case KEFIR_OPT_OPCODE_INT64_MUL:
        case KEFIR_OPT_OPCODE_UINT8_MUL:
        case KEFIR_OPT_OPCODE_UINT16_MUL:
        case KEFIR_OPT_OPCODE_UINT32_MUL:
        case KEFIR_OPT_OPCODE_UINT64_MUL:
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
        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT8_SUB:
        case KEFIR_OPT_OPCODE_INT16_SUB:
        case KEFIR_OPT_OPCODE_INT32_SUB:
        case KEFIR_OPT_OPCODE_INT64_SUB:
        case KEFIR_OPT_OPCODE_INT8_DIV:
        case KEFIR_OPT_OPCODE_INT16_DIV:
        case KEFIR_OPT_OPCODE_INT32_DIV:
        case KEFIR_OPT_OPCODE_INT64_DIV:
        case KEFIR_OPT_OPCODE_INT8_MOD:
        case KEFIR_OPT_OPCODE_INT16_MOD:
        case KEFIR_OPT_OPCODE_INT32_MOD:
        case KEFIR_OPT_OPCODE_INT64_MOD:
        case KEFIR_OPT_OPCODE_UINT8_DIV:
        case KEFIR_OPT_OPCODE_UINT16_DIV:
        case KEFIR_OPT_OPCODE_UINT32_DIV:
        case KEFIR_OPT_OPCODE_UINT64_DIV:
        case KEFIR_OPT_OPCODE_UINT8_MOD:
        case KEFIR_OPT_OPCODE_UINT16_MOD:
        case KEFIR_OPT_OPCODE_UINT32_MOD:
        case KEFIR_OPT_OPCODE_UINT64_MOD:
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
        case KEFIR_OPT_OPCODE_INT8_NOT:
        case KEFIR_OPT_OPCODE_INT16_NOT:
        case KEFIR_OPT_OPCODE_INT32_NOT:
        case KEFIR_OPT_OPCODE_INT64_NOT:
        case KEFIR_OPT_OPCODE_INT8_NEG:
        case KEFIR_OPT_OPCODE_INT16_NEG:
        case KEFIR_OPT_OPCODE_INT32_NEG:
        case KEFIR_OPT_OPCODE_INT64_NEG:
        case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
            *policy = GVN_REPLACEMENT_GLOBAL;
            break;

        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
            *policy = GVN_REPLACEMENT_LOCAL;
            break;

        default:
            *policy = GVN_REPLACEMENT_SKIP;
            break;
    }
    return KEFIR_OK;
}

static kefir_hashtree_hash_t hash_instr(kefir_hashtree_key_t key, void *payload) {
    ASSIGN_DECL_CAST(struct gvn_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GVN state"));
    ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, key);

    const struct kefir_opt_instruction *instr = NULL;
    kefir_result_t res = kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr);
    if (res != KEFIR_OK) {
        return ~0ull;
    } else {
        return hash_instruction_impl(instr);
    }
}

static kefir_int_t compare_instr(kefir_hashtree_key_t key1, kefir_hashtree_key_t key2, void *payload) {
    ASSIGN_DECL_CAST(struct gvn_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GVN state"));
    ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref1, key1);
    ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref2, key2);

    const struct kefir_opt_instruction *instr1 = NULL, *instr2 = NULL;
    kefir_result_t res = kefir_opt_code_container_instr(&state->func->code, instr_ref1, &instr1);
    if (res != KEFIR_OK) {
        return -1;
    }
    res = kefir_opt_code_container_instr(&state->func->code, instr_ref2, &instr2);
    if (res != KEFIR_OK) {
        return 1;
    }

    if (compare_instructions_impl(instr1, instr2)) {
        return 0;
    } else if (instr1->id < instr2->id) {
        return -1;
    } else {
        return 1;
    }
}

static kefir_result_t gvn_scan_control_flow(struct gvn_state *state) {
    kefir_result_t res;
    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&state->func->code, &total_blocks));

    for (kefir_opt_block_id_t block_id = 0; block_id < total_blocks; block_id++) {
        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&state->func->code, block_id, &block));

        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_control_head(&state->func->code, block, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_control(&state->func->code, instr_ref, &instr_ref)) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, kefir_list_tail(&state->queue),
                                               (void *) (kefir_uptr_t) instr_ref));
        }
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t ensure_instruction_inputs(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct gvn_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GVN state"));

    if (!kefir_hashtreeset_has(&state->processed_instr, (kefir_hashtreeset_entry_t) instr_ref)) {
        if (!kefir_hashtreeset_has(&state->queued_instr, (kefir_hashtreeset_entry_t) instr_ref)) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, kefir_list_tail(&state->queue),
                                               (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->queued_instr, (kefir_hashtreeset_entry_t) instr_ref));
        }
        state->all_inputs_processed = false;
    }
    return KEFIR_OK;
}

static kefir_result_t try_replace_instr(struct gvn_state *state, kefir_opt_instruction_ref_t instr_ref,
                                        kefir_opt_instruction_ref_t replacement_ref, kefir_bool_t only_local,
                                        kefir_bool_t *success_ptr) {
    const struct kefir_opt_instruction *instr, *replacement_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, replacement_ref, &replacement_instr));

    kefir_bool_t can_replace = false;
    if (replacement_instr->block_id == instr->block_id) {
        kefir_bool_t sequenced_before = false;
        REQUIRE_OK(kefir_opt_code_structure_is_sequenced_before(state->mem, &state->structure, instr_ref,
                                                                replacement_ref, &sequenced_before));
        can_replace = !sequenced_before;
    } else if (!only_local) {
        REQUIRE_OK(kefir_opt_code_structure_is_dominator(&state->structure, instr->block_id,
                                                         replacement_instr->block_id, &can_replace));
    }

    if (can_replace) {
        REQUIRE_OK(
            kefir_opt_code_container_replace_references(state->mem, &state->func->code, replacement_ref, instr_ref));
    }
    *success_ptr = can_replace;

    return KEFIR_OK;
}

#define IS_BLOCK_REACHABLE(_structure, _block_id)      \
    ((_block_id) == (_structure)->code->entry_point || \
     (_structure)->blocks[(_block_id)].immediate_dominator != KEFIR_ID_NONE)

static kefir_result_t gvn_impl(struct gvn_state *state) {
    REQUIRE_OK(kefir_opt_code_structure_build(state->mem, &state->structure, &state->func->code));
    REQUIRE_OK(gvn_scan_control_flow(state));

    for (struct kefir_list_entry *iter = kefir_list_head(&state->queue); iter != NULL;
         iter = kefir_list_head(&state->queue)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(state->mem, &state->queue, iter));
        REQUIRE_OK(kefir_hashtreeset_delete(state->mem, &state->queued_instr, (kefir_hashtreeset_entry_t) instr_ref));

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));
        if (!IS_BLOCK_REACHABLE(&state->structure, instr->block_id)) {
            continue;
        }
        enum gvn_replacement_policy replacement_policy = GVN_REPLACEMENT_SKIP;
        REQUIRE_OK(instr_replacement_policy(state, instr, &replacement_policy));

        state->all_inputs_processed = true;
        REQUIRE_OK(
            kefir_opt_instruction_extract_inputs(&state->func->code, instr, false, ensure_instruction_inputs, state));
        if (replacement_policy == GVN_REPLACEMENT_SKIP) {
            REQUIRE_OK(
                kefir_hashtreeset_add(state->mem, &state->processed_instr, (kefir_hashtreeset_entry_t) instr_ref));
            continue;
        }
        if (!state->all_inputs_processed) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, kefir_list_tail(&state->queue),
                                               (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->queued_instr, (kefir_hashtreeset_entry_t) instr_ref));
            continue;
        }

        struct gvn_instr_list *list = NULL;
        struct kefir_hashtree_node *node;
        kefir_result_t res = kefir_hashtree_at(&state->instr_hashes, (kefir_hashtree_key_t) instr_ref, &node);
        if (res == KEFIR_NOT_FOUND) {
            list = KEFIR_MALLOC(state->mem, sizeof(struct gvn_instr_list));
            REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate GVN instruction list"));

            res = kefir_list_init(&list->instr_refs);
            REQUIRE_CHAIN(&res, kefir_hashtree_insert(state->mem, &state->instr_hashes,
                                                      (kefir_hashtree_key_t) instr_ref, (kefir_hashtree_value_t) list));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(state->mem, list);
                return res;
            });
        } else {
            REQUIRE_OK(res);
            list = (struct gvn_instr_list *) node->value;
            kefir_bool_t replaced = false;
            for (struct kefir_list_entry *candidate_iter = kefir_list_head(&list->instr_refs);
                 candidate_iter != NULL && !replaced; candidate_iter = candidate_iter->next) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, candidate_instr_ref,
                                 (kefir_uptr_t) candidate_iter->value);

                REQUIRE_OK(try_replace_instr(state, instr_ref, candidate_instr_ref,
                                             replacement_policy == GVN_REPLACEMENT_LOCAL, &replaced));
                if (!replaced) {
                    REQUIRE_OK(try_replace_instr(state, candidate_instr_ref, instr_ref,
                                                 replacement_policy == GVN_REPLACEMENT_LOCAL, &replaced));
                    if (replaced) {
                        candidate_iter->value = (void *) (kefir_uptr_t) instr_ref;
                    }
                }

                if (!replaced && replacement_policy == GVN_REPLACEMENT_GLOBAL) {
                    const struct kefir_opt_instruction *candidate_instr;
                    REQUIRE_OK(
                        kefir_opt_code_container_instr(&state->func->code, candidate_instr_ref, &candidate_instr));

                    kefir_opt_block_id_t closest_common_dominator_block_id;
                    REQUIRE_OK(kefir_opt_find_closest_common_dominator(&state->structure, instr->block_id,
                                                                       candidate_instr->block_id,
                                                                       &closest_common_dominator_block_id));

                    kefir_bool_t can_hoist;
                    REQUIRE_OK(kefir_opt_can_hoist_instruction(&state->structure, instr_ref,
                                                               closest_common_dominator_block_id, &can_hoist));

                    if (can_hoist) {
                        REQUIRE_OK(kefir_opt_move_instruction(state->mem, &state->func->code, &state->func->debug_info,
                                                              instr_ref, closest_common_dominator_block_id,
                                                              &instr_ref));
                        REQUIRE_OK(kefir_opt_code_container_replace_references(state->mem, &state->func->code,
                                                                               instr_ref, candidate_instr_ref));
                        candidate_iter->value = (void *) (kefir_uptr_t) instr_ref;
                        replaced = true;
                    }
                }
            }
            if (replaced) {
                list = NULL;
            }
        }

        if (list != NULL) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &list->instr_refs, kefir_list_tail(&list->instr_refs),
                                               (void *) (kefir_uptr_t) instr_ref));
        }
        REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->processed_instr, (kefir_hashtreeset_entry_t) instr_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t free_instr_refs(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                      kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct gvn_instr_list *, instr_list, value);

    REQUIRE_OK(kefir_list_free(mem, &instr_list->instr_refs));
    KEFIR_FREE(mem, instr_list);
    return KEFIR_OK;
}

static kefir_result_t global_value_numbering_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                   struct kefir_opt_function *func,
                                                   const struct kefir_optimizer_pass *pass,
                                                   const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct gvn_state state = {.mem = mem, .func = func};
    const struct kefir_hashtree_ops instr_ops = {.hash = hash_instr, .compare = compare_instr, .data = &state};
    REQUIRE_OK(kefir_list_init(&state.queue));
    REQUIRE_OK(kefir_hashtreeset_init(&state.queued_instr, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.processed_instr, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&state.instr_hashes, &instr_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state.instr_hashes, free_instr_refs, NULL));
    REQUIRE_OK(kefir_opt_code_structure_init(&state.structure));

    kefir_result_t res = gvn_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, &state.structure);
        kefir_list_free(mem, &state.queue);
        kefir_hashtreeset_free(mem, &state.processed_instr);
        kefir_hashtreeset_free(mem, &state.queued_instr);
        kefir_hashtree_free(mem, &state.instr_hashes);
        return res;
    });
    res = kefir_opt_code_structure_free(mem, &state.structure);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.queue);
        kefir_hashtreeset_free(mem, &state.processed_instr);
        kefir_hashtreeset_free(mem, &state.queued_instr);
        kefir_hashtree_free(mem, &state.instr_hashes);
        return res;
    });
    res = kefir_list_free(mem, &state.queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.processed_instr);
        kefir_hashtreeset_free(mem, &state.queued_instr);
        kefir_hashtree_free(mem, &state.instr_hashes);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &state.processed_instr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.queued_instr);
        kefir_hashtree_free(mem, &state.instr_hashes);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &state.queued_instr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &state.instr_hashes);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &state.instr_hashes));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassGlobalValueNumbering = {
    .name = "gvn", .apply = global_value_numbering_apply, .payload = NULL};
