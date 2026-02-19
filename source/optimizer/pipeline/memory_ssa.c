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

#include "kefir/core/basic-types.h"
#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/memory_ssa.h"
#include "kefir/optimizer/sequencing.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/alias.h"
#include "kefir/optimizer/escape.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t classify_memory_access(const struct kefir_opt_instruction *instr,
                                             kefir_opt_instruction_ref_t *location_ptr, kefir_size_t *size_ptr,
                                             kefir_int64_t *offset_ptr) {
    *offset_ptr = 0;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_LOAD:
        case KEFIR_OPT_OPCODE_INT8_STORE:
            *location_ptr = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF];
            *size_ptr = 1;
            break;

        case KEFIR_OPT_OPCODE_INT16_LOAD:
        case KEFIR_OPT_OPCODE_INT16_STORE:
            *location_ptr = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF];
            *size_ptr = 2;
            break;

        case KEFIR_OPT_OPCODE_INT32_LOAD:
        case KEFIR_OPT_OPCODE_INT32_STORE:
        case KEFIR_OPT_OPCODE_FLOAT32_LOAD:
        case KEFIR_OPT_OPCODE_FLOAT32_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            *location_ptr = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF];
            *size_ptr = 4;
            break;

        case KEFIR_OPT_OPCODE_INT64_LOAD:
        case KEFIR_OPT_OPCODE_INT64_STORE:
        case KEFIR_OPT_OPCODE_FLOAT64_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            *location_ptr = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF];
            *size_ptr = 8;
            break;

        case KEFIR_OPT_OPCODE_INT128_LOAD:
        case KEFIR_OPT_OPCODE_INT128_STORE:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            *location_ptr = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF];
            *size_ptr = 16;
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            *location_ptr = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF];
            *size_ptr = 32;
            break;

        case KEFIR_OPT_OPCODE_BITINT_LOAD:
        case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
        case KEFIR_OPT_OPCODE_BITINT_STORE:
        case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
            *location_ptr = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF];
            if (instr->operation.parameters.bitwidth <= 8) {
                *size_ptr = 1;
            } else if (instr->operation.parameters.bitwidth <= 16) {
                *size_ptr = 2;
            } else if (instr->operation.parameters.bitwidth <= 32) {
                *size_ptr = 4;
            } else if (instr->operation.parameters.bitwidth <= 64) {
                *size_ptr = 8;
            } else {
                *size_ptr = (instr->operation.parameters.bitwidth + 63) / 64 * 8;
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to classify memory access instruction");
    }
    return KEFIR_OK;
}

static kefir_result_t check_clobber(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                    const struct kefir_opt_code_escape_analysis *escapes,
                                    kefir_opt_instruction_ref_t instr_ref1, kefir_opt_instruction_ref_t instr_ref2,
                                    kefir_bool_t *do_alias) {
    UNUSED(mem);
    *do_alias = true;

    const struct kefir_opt_instruction *instr1, *instr2;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref1, &instr1));
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref2, &instr2));

    kefir_opt_instruction_ref_t location1_ref = KEFIR_ID_NONE, location2_ref = KEFIR_ID_NONE;
    kefir_size_t size1 = 0, size2 = 0;
    kefir_int64_t offset1 = 0, offset2 = 0;

    kefir_result_t res = classify_memory_access(instr1, &location1_ref, &size1, &offset1);
    if (res == KEFIR_NO_MATCH) {
        location1_ref = instr_ref1;
        size1 = 0;
        offset1 = 0;
        res = KEFIR_OK;
    }
    REQUIRE_CHAIN(&res, classify_memory_access(instr2, &location2_ref, &size2, &offset2));
    if (res == KEFIR_NO_MATCH) {
        location2_ref = instr_ref2;
        size2 = 0;
        offset2 = 0;
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_opt_code_may_alias(code, escapes, location1_ref, size1, offset1, location2_ref, size2, offset2,
                                        do_alias));
    return KEFIR_OK;
}

static kefir_result_t find_upstream_clobber_impl(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                                 const struct kefir_opt_code_escape_analysis *escapes,
                                                 const struct kefir_opt_code_memssa *memssa,
                                                 kefir_opt_code_memssa_node_ref_t node_ref,
                                                 kefir_opt_code_memssa_node_ref_t *clobber_ref_ptr,
                                                 struct kefir_list *queue, struct kefir_hashset *visited) {
    const struct kefir_opt_code_memssa_node *node;
    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));
    REQUIRE(node->type == KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE || node->type == KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE ||
                node->type == KEFIR_OPT_CODE_MEMSSA_PRODUCE_CONSUME_NODE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find clobber memory ssa node"));

    *clobber_ref_ptr = KEFIR_ID_NONE;
    REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) node_ref));
    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL; iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_opt_code_memssa_node_ref_t, iter_node_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));
        if (kefir_hashset_has(visited, (kefir_hashset_key_t) iter_node_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(mem, visited, (kefir_hashset_key_t) iter_node_ref));

        const struct kefir_opt_code_memssa_node *iter_node;
        REQUIRE_OK(kefir_opt_code_memssa_node(memssa, iter_node_ref, &iter_node));
        switch (iter_node->type) {
            case KEFIR_OPT_CODE_MEMSSA_ROOT_NODE:
                if (*clobber_ref_ptr == KEFIR_ID_NONE || *clobber_ref_ptr == iter_node_ref) {
                    *clobber_ref_ptr = iter_node_ref;
                } else {
                    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find clobber memory ssa node");
                }
                break;

            case KEFIR_OPT_CODE_MEMSSA_PHI_NODE:
                for (kefir_size_t i = 0; i < iter_node->phi.link_count; i++) {
                    REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL,
                                                       (void *) (kefir_uptr_t) iter_node->phi.links[i].node_ref));
                }
                break;

            case KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE:
                REQUIRE_OK(
                    kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) iter_node->predecessor_ref));
                break;

            case KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE:
            case KEFIR_OPT_CODE_MEMSSA_PRODUCE_CONSUME_NODE: {
                kefir_bool_t trace = true;
                if (iter_node_ref != node_ref) {
                    kefir_bool_t node_alias = true;
                    REQUIRE_OK(check_clobber(mem, code, escapes, node->instr_ref, iter_node->instr_ref, &node_alias));
                    if (node_alias) {
                        if (*clobber_ref_ptr == KEFIR_ID_NONE || *clobber_ref_ptr == iter_node_ref) {
                            *clobber_ref_ptr = iter_node_ref;
                            trace = false;
                        } else {
                            return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find clobber memory ssa node");
                        }
                    }
                }
                if (trace) {
                    REQUIRE_OK(
                        kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) iter_node->predecessor_ref));
                }
            } break;

            case KEFIR_OPT_CODE_MEMSSA_TERMINATE_NODE:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected memory ssa node type");
        }
    }

    REQUIRE(*clobber_ref_ptr != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find clobber memory ssa node"));
    return KEFIR_OK;
}

static kefir_result_t find_upstream_clobber(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                            const struct kefir_opt_code_escape_analysis *escapes,
                                            const struct kefir_opt_code_memssa *memssa,
                                            kefir_opt_code_memssa_node_ref_t node_ref,
                                            kefir_opt_code_memssa_node_ref_t *clobber_ref_ptr) {
    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    kefir_result_t res =
        find_upstream_clobber_impl(mem, code, escapes, memssa, node_ref, clobber_ref_ptr, &queue, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &visited);
        kefir_list_free(mem, &queue);
        return res;
    });
    res = kefir_hashset_free(mem, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_util_extend_load_value(struct kefir_mem *, struct kefir_opt_code_container *,
                                                     const struct kefir_ir_module *,
                                                     const struct kefir_opt_instruction *, kefir_opt_instruction_ref_t,
                                                     kefir_opt_instruction_ref_t *);

static kefir_result_t do_optimize_nonvolatile_load(
    struct kefir_mem *mem, struct kefir_opt_module *module, struct kefir_opt_function *func,
    const struct kefir_opt_code_control_flow *control_flow, struct kefir_opt_code_sequencing *sequencing,
    struct kefir_opt_code_memssa *memssa, const struct kefir_opt_code_escape_analysis *escapes,
    const struct kefir_opt_instruction *instr, kefir_bool_t *did_replace) {
    UNUSED(control_flow);
    *did_replace = false;
    kefir_opt_instruction_ref_t instr_ref = instr->id;

    kefir_opt_code_memssa_node_ref_t node_ref, clobber_ref = KEFIR_ID_NONE;
    kefir_result_t res = kefir_opt_code_memssa_instruction_binding(memssa, instr_ref, &node_ref);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    const struct kefir_opt_code_memssa_node *node, *clobber_node;
    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));
    REQUIRE(node->type == KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE, KEFIR_OK);

    res = find_upstream_clobber(mem, &func->code, escapes, memssa, node_ref, &clobber_ref);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, clobber_ref, &clobber_node));
    REQUIRE(clobber_node->type == KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE ||
                clobber_node->type == KEFIR_OPT_CODE_MEMSSA_PRODUCE_CONSUME_NODE,
            KEFIR_OK);

    const struct kefir_opt_instruction *clobber_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, clobber_node->instr_ref, &clobber_instr));

    kefir_bool_t is_sequenced_before;
    REQUIRE_OK(kefir_opt_code_is_sequenced_before(mem, control_flow, sequencing, clobber_node->instr_ref, instr_ref,
                                                  &is_sequenced_before));
    REQUIRE(is_sequenced_before, KEFIR_OK);

    kefir_opt_instruction_ref_t location1_ref, location2_ref;
    kefir_size_t size1, size2;
    kefir_int64_t offset1, offset2;
    res = classify_memory_access(instr, &location1_ref, &size1, &offset1);
    if (res == KEFIR_NO_MATCH) {
        location1_ref = instr->id;
        size1 = 0;
        offset1 = 0;
        res = KEFIR_OK;
    }
    REQUIRE(res != KEFIR_NO_MATCH, KEFIR_OK);
    REQUIRE_OK(res);

    res = classify_memory_access(clobber_instr, &location2_ref, &size2, &offset2);
    if (res == KEFIR_NO_MATCH) {
        location2_ref = clobber_instr->id;
        size2 = 0;
        offset2 = 0;
        res = KEFIR_OK;
    }
    REQUIRE(res != KEFIR_NO_MATCH, KEFIR_OK);
    REQUIRE_OK(res);

    kefir_bool_t must_alias;
    REQUIRE_OK(kefir_opt_code_must_alias(&func->code, location2_ref, size2, offset2, location1_ref, size1, offset1,
                                         &must_alias));
    REQUIRE(must_alias, KEFIR_OK);

    kefir_opt_instruction_ref_t replacement_ref =
        clobber_instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF];
    if ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD &&
         (clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_STORE)) ||
        (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD &&
         (clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_STORE)) ||
        ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_LOAD) &&
         (clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_STORE)) ||
        ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_LOAD ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_LOAD) &&
         clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_STORE)) {

        REQUIRE_OK(kefir_opt_code_builder_to_int(mem, &func->code, instr->block_id, replacement_ref, &replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
        REQUIRE_OK(kefir_opt_code_util_extend_load_value(mem, &func->code, module->ir_module, instr, replacement_ref,
                                                         &replacement_ref));
    } else {
        REQUIRE((instr->operation.opcode == KEFIR_OPT_OPCODE_INT128_LOAD &&
                 clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT128_STORE) ||
                    (instr->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD &&
                     clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE) ||
                    (instr->operation.opcode == KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD &&
                     clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE) ||
                    (instr->operation.opcode == KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD &&
                     clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE) ||
                    (instr->operation.opcode == KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD &&
                     clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE) ||
                    (instr->operation.opcode == KEFIR_OPT_OPCODE_DECIMAL32_LOAD &&
                     clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_DECIMAL32_STORE) ||
                    (instr->operation.opcode == KEFIR_OPT_OPCODE_DECIMAL64_LOAD &&
                     clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_DECIMAL64_STORE) ||
                    (instr->operation.opcode == KEFIR_OPT_OPCODE_DECIMAL128_LOAD &&
                     clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_DECIMAL128_STORE),
                KEFIR_OK);
    }
    REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, instr_ref));
    REQUIRE_OK(kefir_opt_code_sequencing_drop_cache(mem, sequencing));
    REQUIRE_OK(kefir_opt_code_memssa_replace(mem, memssa, node->predecessor_ref, node_ref));
    REQUIRE_OK(kefir_opt_code_memssa_unbind(mem, memssa, node_ref));
    *did_replace = true;
    return KEFIR_OK;
}

static kefir_result_t do_deduplicate_nonvolatile_load(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                      struct kefir_opt_function *func,
                                                      const struct kefir_opt_code_control_flow *control_flow,
                                                      struct kefir_opt_code_sequencing *sequencing,
                                                      struct kefir_opt_code_memssa *memssa,
                                                      const struct kefir_opt_code_escape_analysis *escapes,
                                                      const struct kefir_opt_instruction *instr) {
    UNUSED(control_flow);
    UNUSED(module);
    kefir_opt_instruction_ref_t instr_ref = instr->id;

    kefir_opt_code_memssa_node_ref_t node_ref;
    kefir_result_t res = kefir_opt_code_memssa_instruction_binding(memssa, instr_ref, &node_ref);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    const struct kefir_opt_code_memssa_node *node, *iter_node;
    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));
    REQUIRE(node->type == KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE, KEFIR_OK);

    kefir_opt_instruction_ref_t location1_ref, location2_ref;
    kefir_size_t size1, size2;
    kefir_int64_t offset1, offset2;
    res = classify_memory_access(instr, &location1_ref, &size1, &offset1);
    if (res == KEFIR_NO_MATCH) {
        location1_ref = instr_ref;
        size1 = 0;
        offset1 = 0;
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);

    for (kefir_opt_code_memssa_node_ref_t iter_node_ref = node->predecessor_ref; iter_node_ref != KEFIR_ID_NONE;) {
        REQUIRE_OK(kefir_opt_code_memssa_node(memssa, iter_node_ref, &iter_node));

        kefir_bool_t terminate = false;
        switch (iter_node->type) {
            case KEFIR_OPT_CODE_MEMSSA_ROOT_NODE:
            case KEFIR_OPT_CODE_MEMSSA_PHI_NODE:
                terminate = true;
                break;

            case KEFIR_OPT_CODE_MEMSSA_TERMINATE_NODE:
            case KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected memory ssa node type");

            case KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE:
            case KEFIR_OPT_CODE_MEMSSA_PRODUCE_CONSUME_NODE: {
                const struct kefir_opt_instruction *iter_instr;
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, iter_node->instr_ref, &iter_instr));

                res = classify_memory_access(iter_instr, &location2_ref, &size2, &offset2);
                if (res == KEFIR_NO_MATCH) {
                    location2_ref = iter_node->instr_ref;
                    size2 = 0;
                    offset2 = 0;
                    res = KEFIR_OK;
                }
                REQUIRE_OK(res);

                REQUIRE_OK(kefir_opt_code_may_alias(&func->code, escapes, location1_ref, size1, offset1, location2_ref,
                                                    size2, offset2, &terminate));
            } break;
        }

        struct kefir_opt_code_memssa_use_iterator use_iter;
        kefir_opt_code_memssa_node_ref_t use_node_ref;
        for (res = kefir_opt_code_memssa_use_iter(memssa, &use_iter, iter_node_ref, &use_node_ref); res == KEFIR_OK;
             res = kefir_opt_code_memssa_use_next(&use_iter, &use_node_ref)) {
            const struct kefir_opt_code_memssa_node *use_node;
            REQUIRE_OK(kefir_opt_code_memssa_node(memssa, use_node_ref, &use_node));
            if (use_node->type != KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE || use_node->instr_ref == instr_ref) {
                continue;
            }

            const struct kefir_opt_instruction *use_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, use_node->instr_ref, &use_instr));
            if (use_instr->operation.opcode != instr->operation.opcode ||
                use_instr->operation.parameters.memory_access.flags.load_extension !=
                    instr->operation.parameters.memory_access.flags.load_extension) {
                continue;
            }

            res = classify_memory_access(use_instr, &location2_ref, &size2, &offset2);
            if (res == KEFIR_NO_MATCH) {
                continue;
            }
            REQUIRE_OK(res);

            kefir_bool_t must_alias;
            REQUIRE_OK(kefir_opt_code_must_alias(&func->code, location1_ref, size1, offset1, location2_ref, size2,
                                                 offset2, &must_alias));
            if (must_alias && size1 == size2) {
                kefir_bool_t is_sequenced_before;
                REQUIRE_OK(kefir_opt_code_is_sequenced_before(mem, control_flow, sequencing, use_node->instr_ref,
                                                              instr_ref, &is_sequenced_before));
                if (!is_sequenced_before) {
                    continue;
                }

                REQUIRE_OK(
                    kefir_opt_code_container_replace_references(mem, &func->code, use_node->instr_ref, instr_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
                REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, instr_ref));
                REQUIRE_OK(kefir_opt_code_sequencing_drop_cache(mem, sequencing));
                REQUIRE_OK(kefir_opt_code_memssa_replace(mem, memssa, node->predecessor_ref, node_ref));
                REQUIRE_OK(kefir_opt_code_memssa_unbind(mem, memssa, node_ref));
                return KEFIR_OK;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (terminate) {
            break;
        } else {
            iter_node_ref = iter_node->predecessor_ref;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t has_downstream_clobbers_impl(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                                   const struct kefir_opt_code_escape_analysis *escapes,
                                                   const struct kefir_opt_code_memssa *memssa,
                                                   kefir_opt_code_memssa_node_ref_t node_ref,
                                                   kefir_bool_t *has_clobbers, struct kefir_list *queue,
                                                   struct kefir_hashset *visited, struct kefir_hashset *consumers) {
    kefir_opt_instruction_ref_t location1_ref, location2_ref;
    kefir_size_t size1, size2;
    kefir_int64_t offset1, offset2;

    const struct kefir_opt_code_memssa_node *node;
    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, node->instr_ref, &instr));
    kefir_result_t res = classify_memory_access(instr, &location1_ref, &size1, &offset1);
    if (res == KEFIR_NO_MATCH) {
        location1_ref = node->instr_ref;
        size1 = 0;
        offset1 = 0;
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);

    *has_clobbers = true;
    kefir_bool_t first_iter = true;
    REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) node_ref));
    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL;
         iter = kefir_list_head(queue), first_iter = false) {
        ASSIGN_DECL_CAST(kefir_opt_code_memssa_node_ref_t, iter_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));
        if (kefir_hashset_has(visited, (kefir_hashset_key_t) iter_ref)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(mem, visited, (kefir_hashset_key_t) iter_ref));

        const struct kefir_opt_code_memssa_node *iter_node;
        REQUIRE_OK(kefir_opt_code_memssa_node(memssa, iter_ref, &iter_node));

        kefir_bool_t continue_scan = true;
        if (!first_iter) {
            switch (iter_node->type) {
                case KEFIR_OPT_CODE_MEMSSA_ROOT_NODE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected memory ssa node type");

                case KEFIR_OPT_CODE_MEMSSA_TERMINATE_NODE:
                    *has_clobbers = false;
                    return KEFIR_OK;

                case KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE: {
                    const struct kefir_opt_instruction *iter_instr;
                    res = kefir_opt_code_container_instr(code, iter_node->instr_ref, &iter_instr);
                    if (res != KEFIR_NOT_FOUND) {
                        REQUIRE_OK(res);
                        res = classify_memory_access(iter_instr, &location2_ref, &size2, &offset2);
                        if (res == KEFIR_NO_MATCH) {
                            location2_ref = iter_node->instr_ref;
                            size2 = 0;
                            offset2 = 0;
                            res = KEFIR_OK;
                        }
                        REQUIRE_OK(res);

                        kefir_bool_t must_alias;
                        REQUIRE_OK(kefir_opt_code_must_alias(code, location1_ref, size1, offset1, location2_ref, size2,
                                                             offset2, &must_alias));
                        if (must_alias && size2 <= size1) {
                            REQUIRE_OK(kefir_hashset_add(mem, consumers, (kefir_hashset_key_t) iter_node->instr_ref));
                        } else {
                            kefir_bool_t may_alias;
                            REQUIRE_OK(kefir_opt_code_may_alias(code, escapes, location1_ref, size1, offset1,
                                                                location2_ref, size2, offset2, &may_alias));
                            if (may_alias) {
                                *has_clobbers = false;
                                return KEFIR_OK;
                            }
                        }
                    }
                } break;

                case KEFIR_OPT_CODE_MEMSSA_PRODUCE_CONSUME_NODE: {
                    const struct kefir_opt_instruction *iter_instr;
                    res = kefir_opt_code_container_instr(code, iter_node->instr_ref, &iter_instr);
                    if (res != KEFIR_NOT_FOUND) {
                        REQUIRE_OK(res);
                        res = classify_memory_access(iter_instr, &location2_ref, &size2, &offset2);
                        if (res == KEFIR_NO_MATCH) {
                            location2_ref = iter_node->instr_ref;
                            size2 = 0;
                            offset2 = 0;
                            res = KEFIR_OK;
                        }
                        REQUIRE_OK(res);

                        kefir_bool_t must_alias;
                        REQUIRE_OK(kefir_opt_code_must_alias(code, location1_ref, size1, offset1, location2_ref, size2,
                                                             offset2, &must_alias));

                        if (must_alias && size1 == size2) {
                            REQUIRE_OK(kefir_hashset_add(mem, consumers, (kefir_hashset_key_t) iter_node->instr_ref));
                            continue_scan = false;
                        } else {
                            kefir_bool_t may_alias;
                            REQUIRE_OK(kefir_opt_code_may_alias(code, escapes, location1_ref, size1, offset1,
                                                                location2_ref, size2, offset2, &may_alias));
                            if (may_alias) {
                                *has_clobbers = false;
                                return KEFIR_OK;
                            }
                        }
                    }
                } break;

                case KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE: {
                    const struct kefir_opt_instruction *iter_instr;
                    res = kefir_opt_code_container_instr(code, iter_node->instr_ref, &iter_instr);
                    if (res != KEFIR_NOT_FOUND) {
                        REQUIRE_OK(res);
                        res = classify_memory_access(iter_instr, &location2_ref, &size2, &offset2);
                        if (res == KEFIR_NO_MATCH) {
                            location2_ref = iter_node->instr_ref;
                            size2 = 0;
                            offset2 = 0;
                            res = KEFIR_OK;
                        }
                        REQUIRE_OK(res);

                        kefir_bool_t must_alias;
                        REQUIRE_OK(kefir_opt_code_must_alias(code, location1_ref, size1, offset1, location2_ref, size2,
                                                             offset2, &must_alias));
                        if (must_alias && size2 >= size1) {
                            continue_scan = false;
                        } else {
                            kefir_bool_t may_alias;
                            REQUIRE_OK(kefir_opt_code_may_alias(code, escapes, location1_ref, size1, offset1,
                                                                location2_ref, size2, offset2, &may_alias));
                            if (may_alias) {
                                *has_clobbers = false;
                                return KEFIR_OK;
                            }
                        }
                    }
                } break;

                case KEFIR_OPT_CODE_MEMSSA_PHI_NODE:
                    // Intentionally left blank
                    break;
            }
        }

        struct kefir_opt_code_memssa_use_iterator use_iter;
        kefir_opt_code_memssa_node_ref_t use_node_ref;
        for (res = kefir_opt_code_memssa_use_iter(memssa, &use_iter, iter_ref, &use_node_ref);
             continue_scan && res == KEFIR_OK; res = kefir_opt_code_memssa_use_next(&use_iter, &use_node_ref)) {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) use_node_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t has_downstream_clobbers(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_code_escape_analysis *escapes,
                                              const struct kefir_opt_code_memssa *memssa,
                                              kefir_opt_code_memssa_node_ref_t node_ref, kefir_bool_t *has_clobbers,
                                              struct kefir_hashset *consumers) {
    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    kefir_result_t res =
        has_downstream_clobbers_impl(mem, code, escapes, memssa, node_ref, has_clobbers, &queue, &visited, consumers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &visited);
        kefir_list_free(mem, &queue);
        return res;
    });
    res = kefir_hashset_free(mem, &visited);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

static kefir_result_t uses_terminate_at(const struct kefir_opt_code_container *code,
                                        kefir_opt_instruction_ref_t source_ref, kefir_opt_instruction_ref_t target_ref,
                                        kefir_bool_t permit_control_flow, kefir_size_t depth,
                                        kefir_bool_t *terminates_at) {
    REQUIRE(source_ref != target_ref, KEFIR_OK);
    if (depth == 0) {
        *terminates_at = false;
        return KEFIR_OK;
    }

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, source_ref, &instr));

    kefir_bool_t is_control_flow, is_side_effect_free;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, source_ref, &is_control_flow));
    REQUIRE_OK(kefir_opt_instruction_is_side_effect_free(instr, &is_side_effect_free));

    if (!permit_control_flow && (is_control_flow || !is_side_effect_free)) {
        *terminates_at = false;
        return KEFIR_OK;
    }

    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(code, source_ref, &use_iter);
         res == KEFIR_OK && *terminates_at; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        REQUIRE_OK(uses_terminate_at(code, use_iter.use_instr_ref, target_ref, false, depth - 1, terminates_at));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

#define MAX_USE_CHAIN 16
static kefir_result_t all_uses_terminate_at(const struct kefir_opt_code_container *code,
                                            const struct kefir_hashset *sources, kefir_opt_instruction_ref_t target_ref,
                                            kefir_bool_t *all_terminate_at) {
    *all_terminate_at = true;

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(sources, &iter, &key); res == KEFIR_OK && *all_terminate_at;
         res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, source_ref, key);

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, source_ref, &instr));
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
            case KEFIR_OPT_OPCODE_BITINT_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
            case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
                REQUIRE_OK(uses_terminate_at(code, source_ref, target_ref, true, MAX_USE_CHAIN, all_terminate_at));
                break;

            default:
                all_terminate_at = false;
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t do_optimize_nonvolatile_store(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                    struct kefir_opt_function *func,
                                                    const struct kefir_opt_code_control_flow *control_flow,
                                                    struct kefir_opt_code_sequencing *sequencing,
                                                    struct kefir_opt_code_memssa *memssa,
                                                    const struct kefir_opt_code_escape_analysis *escapes,
                                                    const struct kefir_opt_instruction *instr) {
    UNUSED(control_flow);
    UNUSED(module);
    UNUSED(has_downstream_clobbers);
    kefir_opt_instruction_ref_t instr_ref = instr->id;

    kefir_opt_code_memssa_node_ref_t node_ref, clobber_ref = KEFIR_ID_NONE;
    kefir_result_t res = kefir_opt_code_memssa_instruction_binding(memssa, instr_ref, &node_ref);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    const struct kefir_opt_code_memssa_node *node, *clobber_node;
    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));
    REQUIRE(node->type == KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE, KEFIR_OK);

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_STORE:
        case KEFIR_OPT_OPCODE_INT16_STORE:
        case KEFIR_OPT_OPCODE_INT32_STORE:
        case KEFIR_OPT_OPCODE_INT64_STORE:
        case KEFIR_OPT_OPCODE_INT128_STORE:
        case KEFIR_OPT_OPCODE_FLOAT32_STORE:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
        case KEFIR_OPT_OPCODE_BITINT_STORE:
        case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
        case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
        case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            // Intentionally left blank
            break;

        default:
            return KEFIR_OK;
    }

    kefir_bool_t downstream_clobbers = false, all_consumers_terminate_at = false;
    struct kefir_hashset consumers;
    REQUIRE_OK(kefir_hashset_init(&consumers, &kefir_hashtable_uint_ops));
    res = has_downstream_clobbers(mem, &func->code, escapes, memssa, node_ref, &downstream_clobbers, &consumers);
    REQUIRE_CHAIN(&res, all_uses_terminate_at(&func->code, &consumers, instr_ref, &all_consumers_terminate_at));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &consumers);
        return res;
    });
    if (downstream_clobbers && all_consumers_terminate_at) {
        REQUIRE_OK(kefir_hashset_free(mem, &consumers));
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, instr_ref));
        REQUIRE_OK(kefir_opt_code_sequencing_drop_cache(mem, sequencing));
        REQUIRE_OK(kefir_opt_code_memssa_replace(mem, memssa, node->predecessor_ref, node_ref));
        REQUIRE_OK(kefir_opt_code_memssa_unbind(mem, memssa, node_ref));
        return KEFIR_OK;
    } else {
        REQUIRE_OK(kefir_hashset_free(mem, &consumers));
    }

    res = find_upstream_clobber(mem, &func->code, escapes, memssa, node_ref, &clobber_ref);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, clobber_ref, &clobber_node));
    REQUIRE(clobber_node->type == KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE, KEFIR_OK);

    const struct kefir_opt_instruction *clobber_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, clobber_node->instr_ref, &clobber_instr));

    kefir_bool_t is_sequenced_before;
    REQUIRE_OK(kefir_opt_code_is_sequenced_before(mem, control_flow, sequencing, clobber_node->instr_ref, instr_ref,
                                                  &is_sequenced_before));
    REQUIRE(is_sequenced_before, KEFIR_OK);

    kefir_opt_instruction_ref_t location1_ref, location2_ref;
    kefir_size_t size1, size2;
    kefir_int64_t offset1, offset2;
    res = classify_memory_access(instr, &location1_ref, &size1, &offset1);
    REQUIRE_CHAIN(&res, classify_memory_access(clobber_instr, &location2_ref, &size2, &offset2));
    REQUIRE(res != KEFIR_NO_MATCH, KEFIR_OK);
    REQUIRE_OK(res);

    kefir_bool_t must_alias;
    REQUIRE_OK(kefir_opt_code_must_alias(&func->code, location2_ref, size2, offset2, location1_ref, size1, offset1,
                                         &must_alias));
    REQUIRE(must_alias, KEFIR_OK);

    if (instr->operation.opcode == clobber_instr->operation.opcode &&
        instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF] ==
            clobber_instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]) {
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, instr_ref));
        REQUIRE_OK(kefir_opt_code_sequencing_drop_cache(mem, sequencing));
        REQUIRE_OK(kefir_opt_code_memssa_replace(mem, memssa, node->predecessor_ref, node_ref));
        REQUIRE_OK(kefir_opt_code_memssa_unbind(mem, memssa, node_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t do_optimize(struct kefir_mem *mem, struct kefir_opt_module *module,
                                  struct kefir_opt_function *func,
                                  const struct kefir_opt_code_control_flow *control_flow,
                                  struct kefir_opt_code_sequencing *sequencing, struct kefir_opt_code_memssa *memssa,
                                  const struct kefir_opt_code_escape_analysis *escapes) {
    for (kefir_opt_block_id_t block_id = 0; block_id < kefir_opt_code_container_block_count(&func->code); block_id++) {
        kefir_bool_t is_reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, block_id, &is_reachable));
        if (!is_reachable) {
            continue;
        }

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref, next_control_ref;
        for (res = kefir_opt_code_block_instr_control_head(&func->code, block_id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE; instr_ref = next_control_ref) {
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
            REQUIRE_OK(kefir_opt_instruction_next_control(&func->code, instr_ref, &next_control_ref));

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
                case KEFIR_OPT_OPCODE_BITINT_LOAD:
                case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
                case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
                case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
                case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
                    if (!instr->operation.parameters.memory_access.flags.volatile_access) {
                        kefir_bool_t did_replace = false;
                        REQUIRE_OK(do_optimize_nonvolatile_load(mem, module, func, control_flow, sequencing, memssa,
                                                                escapes, instr, &did_replace));
                        if (!did_replace) {
                            REQUIRE_OK(do_deduplicate_nonvolatile_load(mem, module, func, control_flow, sequencing,
                                                                       memssa, escapes, instr));
                        }
                    }
                    break;

                case KEFIR_OPT_OPCODE_INT8_STORE:
                case KEFIR_OPT_OPCODE_INT16_STORE:
                case KEFIR_OPT_OPCODE_INT32_STORE:
                case KEFIR_OPT_OPCODE_INT64_STORE:
                case KEFIR_OPT_OPCODE_INT128_STORE:
                case KEFIR_OPT_OPCODE_FLOAT32_STORE:
                case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
                case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
                case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
                case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
                case KEFIR_OPT_OPCODE_BITINT_STORE:
                case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
                case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
                case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
                case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
                    if (!instr->operation.parameters.memory_access.flags.volatile_access) {
                        REQUIRE_OK(do_optimize_nonvolatile_store(mem, module, func, control_flow, sequencing, memssa,
                                                                 escapes, instr));
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
    }
    return KEFIR_OK;
}

static kefir_result_t memory_ssa_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                       struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                       const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_sequencing sequencing;
    struct kefir_opt_code_liveness liveness;
    struct kefir_opt_code_memssa memssa;
    struct kefir_opt_code_escape_analysis escapes;
    REQUIRE_OK(kefir_opt_code_control_flow_init(&control_flow));
    REQUIRE_OK(kefir_opt_code_sequencing_init(&sequencing));
    REQUIRE_OK(kefir_opt_code_liveness_init(&liveness));
    REQUIRE_OK(kefir_opt_code_memssa_init(&memssa));
    REQUIRE_OK(kefir_opt_code_escape_analysis_init(&escapes));

    kefir_result_t res = kefir_opt_code_control_flow_build(mem, &control_flow, &func->code);
    REQUIRE_CHAIN(&res, kefir_opt_code_liveness_build(mem, &liveness, &control_flow));
    REQUIRE_CHAIN(&res, kefir_opt_code_memssa_construct(mem, &memssa, &func->code, &control_flow, &liveness));
    REQUIRE_CHAIN(&res, kefir_opt_code_escape_analysis_build(mem, &escapes, &func->code));
    REQUIRE_CHAIN(&res, do_optimize(mem, module, func, &control_flow, &sequencing, &memssa, &escapes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_escape_analysis_free(mem, &escapes);
        kefir_opt_code_memssa_free(mem, &memssa);
        kefir_opt_code_liveness_free(mem, &liveness);
        kefir_opt_code_sequencing_free(mem, &sequencing);
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_opt_code_escape_analysis_free(mem, &escapes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_memssa_free(mem, &memssa);
        kefir_opt_code_liveness_free(mem, &liveness);
        kefir_opt_code_sequencing_free(mem, &sequencing);
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_opt_code_memssa_free(mem, &memssa);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_liveness_free(mem, &liveness);
        kefir_opt_code_sequencing_free(mem, &sequencing);
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_opt_code_liveness_free(mem, &liveness);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_sequencing_free(mem, &sequencing);
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_opt_code_sequencing_free(mem, &sequencing);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &control_flow));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassMemorySSA = {
    .name = "memory-ssa", .apply = memory_ssa_apply, .payload = NULL};
