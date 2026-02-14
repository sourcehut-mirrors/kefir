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
        case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
        case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            *location_ptr = instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF];
            *size_ptr = 4;
            break;

        case KEFIR_OPT_OPCODE_INT64_LOAD:
        case KEFIR_OPT_OPCODE_INT64_STORE:
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
    REQUIRE_CHAIN(&res, classify_memory_access(instr2, &location2_ref, &size2, &offset2));
    if (res != KEFIR_NO_MATCH) {
        REQUIRE_OK(res);
        REQUIRE_OK(
            kefir_opt_code_may_alias(code, location1_ref, size1, offset1, location2_ref, size2, offset2, do_alias));
    }
    return KEFIR_OK;
}

static kefir_result_t find_clobber_impl(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                        const struct kefir_opt_code_memssa *memssa,
                                        kefir_opt_code_memssa_node_ref_t node_ref,
                                        kefir_opt_code_memssa_node_ref_t *clobber_ref_ptr, struct kefir_list *queue,
                                        struct kefir_hashset *visited) {
    const struct kefir_opt_code_memssa_node *node;
    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));
    REQUIRE(node->type == KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE || node->type == KEFIR_OPT_CODE_MEMSSA_PRODUCE_NODE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find clobber memory ssa node"));

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
                *clobber_ref_ptr = iter_node_ref;
                return KEFIR_OK;

            case KEFIR_OPT_CODE_MEMSSA_JOIN_NODE:
                for (kefir_size_t i = 0; i < iter_node->join.input_length; i++) {
                    REQUIRE_OK(
                        kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) iter_node->join.inputs[i]));
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
                if (iter_node_ref != node_ref) {
                    kefir_bool_t node_alias = true;
                    REQUIRE_OK(check_clobber(mem, code, node->instr_ref, iter_node->instr_ref, &node_alias));
                    if (node_alias) {
                        *clobber_ref_ptr = iter_node_ref;
                        return KEFIR_OK;
                    }
                }
                REQUIRE_OK(
                    kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) iter_node->predecessor_ref));
                break;
        }
    }
    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find clobber memory ssa node");
}

static kefir_result_t find_clobber(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                   const struct kefir_opt_code_memssa *memssa,
                                   kefir_opt_code_memssa_node_ref_t node_ref,
                                   kefir_opt_code_memssa_node_ref_t *clobber_ref_ptr) {
    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    kefir_result_t res = find_clobber_impl(mem, code, memssa, node_ref, clobber_ref_ptr, &queue, &visited);
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

static kefir_result_t do_optimize_nonvolatile_load(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                   struct kefir_opt_function *func,
                                                   const struct kefir_opt_code_control_flow *control_flow,
                                                   struct kefir_opt_code_sequencing *sequencing,
                                                   const struct kefir_opt_code_memssa *memssa,
                                                   const struct kefir_opt_instruction *instr) {
    UNUSED(control_flow);
    kefir_opt_instruction_ref_t instr_ref = instr->id;

    kefir_opt_code_memssa_node_ref_t node_ref, clobber_ref = KEFIR_ID_NONE;
    kefir_result_t res = kefir_opt_code_memssa_instruction_binding(memssa, instr_ref, &node_ref);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    const struct kefir_opt_code_memssa_node *node, *clobber_node;
    REQUIRE_OK(kefir_opt_code_memssa_node(memssa, node_ref, &node));
    REQUIRE(node->type == KEFIR_OPT_CODE_MEMSSA_CONSUME_NODE, KEFIR_OK);

    res = find_clobber(mem, &func->code, memssa, node_ref, &clobber_ref);
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

    kefir_opt_instruction_ref_t replacement_ref =
        clobber_instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF];
    if ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD &&
         (clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_STORE)) ||
        (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD &&
         (clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_STORE)) ||
        (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD &&
         (clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_STORE ||
          clobber_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_STORE)) ||
        (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_LOAD &&
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
    return KEFIR_OK;
}

static kefir_result_t do_optimize(struct kefir_mem *mem, struct kefir_opt_module *module,
                                  struct kefir_opt_function *func,
                                  const struct kefir_opt_code_control_flow *control_flow,
                                  struct kefir_opt_code_sequencing *sequencing,
                                  const struct kefir_opt_code_memssa *memssa) {
    for (kefir_opt_block_id_t block_id = 0; block_id < kefir_opt_code_container_block_count(&func->code); block_id++) {
        kefir_bool_t is_reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, block_id, &is_reachable));
        if (!is_reachable) {
            continue;
        }

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_control_head(&func->code, block_id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_control(&func->code, instr_ref, &instr_ref)) {
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));

            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT8_LOAD:
                case KEFIR_OPT_OPCODE_INT16_LOAD:
                case KEFIR_OPT_OPCODE_INT32_LOAD:
                case KEFIR_OPT_OPCODE_INT64_LOAD:
                case KEFIR_OPT_OPCODE_INT128_LOAD:
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
                        REQUIRE_OK(
                            do_optimize_nonvolatile_load(mem, module, func, control_flow, sequencing, memssa, instr));
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
    REQUIRE_OK(kefir_opt_code_control_flow_init(&control_flow));
    REQUIRE_OK(kefir_opt_code_sequencing_init(&sequencing));
    REQUIRE_OK(kefir_opt_code_liveness_init(&liveness));
    REQUIRE_OK(kefir_opt_code_memssa_init(&memssa));

    kefir_result_t res = kefir_opt_code_control_flow_build(mem, &control_flow, &func->code);
    REQUIRE_CHAIN(&res, kefir_opt_code_liveness_build(mem, &liveness, &control_flow));
    REQUIRE_CHAIN(&res, kefir_opt_code_memssa_construct(mem, &memssa, &func->code, &control_flow, &liveness));
    REQUIRE_CHAIN(&res, do_optimize(mem, module, func, &control_flow, &sequencing, &memssa));
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
