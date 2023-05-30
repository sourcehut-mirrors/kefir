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

#include "kefir/codegen/opt-common/linear_register_allocator.h"
#include "kefir/optimizer/liveness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_allocation(struct kefir_mem *mem, struct kefir_graph *graph, kefir_graph_node_id_t key,
                                      kefir_graph_node_value_t value, void *payload) {
    UNUSED(graph);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, allocation, value);
    REQUIRE(allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linear register allocation"));

    memset(allocation, 0, sizeof(struct kefir_codegen_opt_linear_register_allocation));
    KEFIR_FREE(mem, allocation);
    return KEFIR_OK;
}

static kefir_result_t allocator_init_impl(struct kefir_mem *mem,
                                          struct kefir_codegen_opt_linear_register_allocator *allocator,
                                          struct kefir_list *alive) {
    for (kefir_size_t instr_idx = 0; instr_idx < allocator->analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            allocator->analysis->linearization[instr_idx];

        struct kefir_codegen_opt_linear_register_allocation *instr_allocation =
            KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_opt_linear_register_allocation));
        REQUIRE(instr_allocation != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate linear register allocation entry"));

        instr_allocation->done = false;
        instr_allocation->constraint.type = KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_GENERAL_PURPOSE;
        instr_allocation->constraint.register_hint.present = false;
        instr_allocation->constraint.alias_hint.present = false;

        kefir_result_t res =
            kefir_graph_new_node(mem, &allocator->allocation, (kefir_graph_node_id_t) instr_props->instr_ref,
                                 (kefir_graph_node_value_t) instr_allocation);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, instr_allocation);
            return res;
        });

        for (const struct kefir_list_entry *iter = kefir_list_head(alive); iter != NULL;) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);

            const struct kefir_opt_instruction_liveness_interval *liveness =
                &allocator->analysis->liveness.intervals[allocator->analysis->instructions[instr_ref].linear_position];
            if (liveness->range.end <= instr_idx) {
                const struct kefir_list_entry *next_iter = iter->next;
                REQUIRE_OK(kefir_list_pop(mem, alive, (struct kefir_list_entry *) iter));
                iter = next_iter;
            } else {
                REQUIRE_OK(kefir_graph_new_edge(mem, &allocator->allocation, (kefir_graph_node_id_t) instr_ref,
                                                (kefir_graph_node_id_t) instr_props->instr_ref));
                REQUIRE_OK(kefir_graph_new_edge(mem, &allocator->allocation,
                                                (kefir_graph_node_id_t) instr_props->instr_ref,
                                                (kefir_graph_node_id_t) instr_ref));
                kefir_list_next(&iter);
            }
        }

        REQUIRE_OK(kefir_list_insert_after(mem, alive, kefir_list_tail(alive),
                                           (void *) (kefir_uptr_t) instr_props->instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t allocator_init(struct kefir_mem *mem,
                                     struct kefir_codegen_opt_linear_register_allocator *allocator) {
    struct kefir_list alive;
    REQUIRE_OK(kefir_list_init(&alive));

    kefir_result_t res = allocator_init_impl(mem, allocator, &alive);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &alive);
        return res;
    });

    REQUIRE_OK(kefir_list_free(mem, &alive));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_linear_register_allocator_init(
    struct kefir_mem *mem, struct kefir_codegen_opt_linear_register_allocator *allocator,
    const struct kefir_opt_code_analysis *analysis, kefir_size_t general_purpose_registers,
    kefir_size_t floating_point_registers) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to linear register allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    allocator->analysis = analysis;
    REQUIRE_OK(kefir_graph_init(&allocator->allocation, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_graph_on_removal(&allocator->allocation, free_allocation, NULL));
    REQUIRE_OK(
        kefir_codegen_opt_virtual_register_allocator_init(mem, &allocator->general_purpose, general_purpose_registers));

    kefir_result_t res =
        kefir_codegen_opt_virtual_register_allocator_init(mem, &allocator->floating_point, floating_point_registers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_virtual_register_allocator_free(mem, &allocator->general_purpose);
        return res;
    });

    res = allocator_init(mem, allocator);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_linear_register_allocator_free(mem, allocator);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_linear_register_allocator_free(
    struct kefir_mem *mem, struct kefir_codegen_opt_linear_register_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linear register allocator"));

    REQUIRE_OK(kefir_codegen_opt_virtual_register_allocator_free(mem, &allocator->general_purpose));
    REQUIRE_OK(kefir_codegen_opt_virtual_register_allocator_free(mem, &allocator->floating_point));
    REQUIRE_OK(kefir_graph_free(mem, &allocator->allocation));
    memset(allocator, 0, sizeof(struct kefir_codegen_opt_linear_register_allocator));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_linear_register_allocator_set_type(
    const struct kefir_codegen_opt_linear_register_allocator *allocator, kefir_opt_instruction_ref_t instr_ref,
    kefir_codegen_opt_linear_register_allocator_constraint_type_t type) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linear register allocator"));

    struct kefir_graph_node *node = NULL;
    kefir_result_t res = kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find a linear register allocation entry");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, allocation, node->value);
    allocation->constraint.type = type;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_linear_register_allocator_hint_register(
    const struct kefir_codegen_opt_linear_register_allocator *allocator, kefir_opt_instruction_ref_t instr_ref,
    kefir_codegen_opt_virtual_register_t hint) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linear register allocator"));

    struct kefir_graph_node *node = NULL;
    kefir_result_t res = kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find a linear register allocation entry");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, allocation, node->value);
    allocation->constraint.register_hint.present = true;
    allocation->constraint.register_hint.vreg = hint;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_linear_register_allocator_hint_alias(
    const struct kefir_codegen_opt_linear_register_allocator *allocator, kefir_opt_instruction_ref_t instr_ref,
    kefir_opt_instruction_ref_t alias) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linear register allocator"));

    struct kefir_graph_node *node = NULL;
    kefir_result_t res = kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find a linear register allocation entry");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, allocation, node->value);
    allocation->constraint.alias_hint.present = true;
    allocation->constraint.alias_hint.alias = alias;
    return KEFIR_OK;
}

static kefir_result_t allocate_vreg(struct kefir_codegen_opt_linear_register_allocator *allocator,
                                    struct kefir_codegen_opt_linear_register_allocation *allocation,
                                    kefir_codegen_opt_virtual_register_t vreg, kefir_bool_t *success) {
    REQUIRE(!allocation->done,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected linear register allocation entry to be empty"));

    switch (allocation->constraint.type) {
        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_GENERAL_PURPOSE:
            REQUIRE_OK(
                kefir_codegen_opt_virtual_register_allocator_allocate(&allocator->general_purpose, vreg, success));
            break;

        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_FLOATING_POINT:
            REQUIRE_OK(
                kefir_codegen_opt_virtual_register_allocator_allocate(&allocator->floating_point, vreg, success));
            break;

        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected linear register allocation type");
    }

    if (*success) {
        allocation->done = true;
        allocation->allocation = vreg;
    }
    return KEFIR_OK;
}

static kefir_result_t deallocate(struct kefir_codegen_opt_linear_register_allocator *allocator,
                                 struct kefir_codegen_opt_linear_register_allocation *allocation) {
    REQUIRE(allocation->done,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected linear register allocation entry to be filled"));

    switch (allocation->constraint.type) {
        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected linear register entry allocation type");

        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_GENERAL_PURPOSE:
            REQUIRE_OK(kefir_codegen_opt_virtual_register_allocator_deallocate(&allocator->general_purpose,
                                                                               allocation->allocation));
            break;

        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_FLOATING_POINT:
            REQUIRE_OK(kefir_codegen_opt_virtual_register_allocator_deallocate(&allocator->floating_point,
                                                                               allocation->allocation));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t propagate_hints(struct kefir_codegen_opt_linear_register_allocator *allocator) {
    for (kefir_size_t instr_rev_idx = 0; instr_rev_idx < allocator->analysis->linearization_length; instr_rev_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            allocator->analysis->linearization[allocator->analysis->linearization_length - instr_rev_idx - 1];

        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_props->instr_ref, &node));
        ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, allocation, node->value);

        if (allocation->constraint.register_hint.present && allocation->constraint.alias_hint.present &&
            allocation->constraint.type != KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION) {
            const struct kefir_opt_code_analysis_instruction_properties *alias_instr_props =
                &allocator->analysis->instructions[allocation->constraint.alias_hint.alias];
            REQUIRE_OK(kefir_graph_node(&allocator->allocation,
                                        (kefir_graph_node_id_t) allocation->constraint.alias_hint.alias, &node));
            ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, alias_allocation, node->value);
            if (alias_instr_props->linear_position < instr_props->linear_position &&
                alias_allocation->constraint.type !=
                    KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION &&
                !alias_allocation->constraint.register_hint.present) {

                alias_allocation->constraint.register_hint.present = true;
                alias_allocation->constraint.register_hint.vreg = allocation->constraint.register_hint.vreg;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t conflict_filter(kefir_codegen_opt_virtual_register_t vreg, kefir_bool_t *fits, void *payload) {
    REQUIRE(fits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(const struct kefir_hashtreeset *, conflict_hints, payload);
    REQUIRE(conflict_hints != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linear register allocation conflictng hint set"));

    *fits = kefir_hashtreeset_has(conflict_hints, (kefir_hashtreeset_entry_t) vreg);
    return KEFIR_OK;
}

static kefir_result_t deallocate_dead(struct kefir_mem *mem,
                                      struct kefir_codegen_opt_linear_register_allocator *allocator,
                                      struct kefir_list *alive, kefir_size_t instr_idx) {
    struct kefir_graph_node *node = NULL;
    for (const struct kefir_list_entry *iter = kefir_list_head(alive); iter != NULL;) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, prev_instr_ref, (kefir_uptr_t) iter->value);

        const struct kefir_opt_instruction_liveness_interval *prev_liveness =
            &allocator->analysis->liveness.intervals[allocator->analysis->instructions[prev_instr_ref].linear_position];
        const struct kefir_list_entry *next_iter = iter->next;
        if (prev_liveness->range.end <= instr_idx + 1) {
            REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) prev_instr_ref, &node));
            ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, prev_allocation, node->value);
            REQUIRE_OK(deallocate(allocator, prev_allocation));
            REQUIRE_OK(kefir_list_pop(mem, alive, (struct kefir_list_entry *) iter));
        }
        iter = next_iter;
    }
    return KEFIR_OK;
}

static kefir_result_t collect_conflict_hints(struct kefir_mem *mem,
                                             struct kefir_codegen_opt_linear_register_allocator *allocator,
                                             struct kefir_hashtreeset *conflict_hints,
                                             const struct kefir_opt_code_analysis_instruction_properties *instr_props,
                                             struct kefir_codegen_opt_linear_register_allocation *current_allocation) {
    kefir_result_t res;
    REQUIRE_OK(kefir_hashtreeset_clean(mem, conflict_hints));
    kefir_graph_node_id_t conflict_edge;
    struct kefir_graph_edge_iterator edge_iter;
    for (res = kefir_graph_edge_iter(&allocator->allocation, &edge_iter, (kefir_graph_node_id_t) instr_props->instr_ref,
                                     &conflict_edge);
         res == KEFIR_OK; res = kefir_graph_edge_next(&edge_iter, &conflict_edge)) {

        struct kefir_opt_code_analysis_instruction_properties *conflict_instr_props =
            &allocator->analysis->instructions[conflict_edge];
        if (conflict_instr_props->linear_position < instr_props->linear_position) {
            continue;
        }

        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) conflict_edge, &node));
        ASSIGN_DECL_CAST(const struct kefir_codegen_opt_linear_register_allocation *, conflict_allocation, node->value);

        if (conflict_allocation->constraint.type != current_allocation->constraint.type) {
            continue;
        }

        if (conflict_allocation->constraint.register_hint.present) {
            REQUIRE_OK(kefir_hashtreeset_add(
                mem, conflict_hints, (kefir_hashtreeset_entry_t) conflict_allocation->constraint.register_hint.vreg));
        } else if (conflict_allocation->constraint.alias_hint.present) {
            struct kefir_graph_node *node = NULL;
            REQUIRE_OK(kefir_graph_node(&allocator->allocation,
                                        (kefir_graph_node_id_t) conflict_allocation->constraint.alias_hint.alias,
                                        &node));
            ASSIGN_DECL_CAST(const struct kefir_codegen_opt_linear_register_allocation *, ref_allocation, node->value);
            if (ref_allocation->done) {
                REQUIRE_OK(
                    kefir_hashtreeset_add(mem, conflict_hints, (kefir_hashtreeset_entry_t) ref_allocation->allocation));
            }
        }
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    return KEFIR_OK;
}

static kefir_result_t attempt_hint_allocation(struct kefir_codegen_opt_linear_register_allocator *allocator,
                                              struct kefir_codegen_opt_linear_register_allocation *current_allocation,
                                              kefir_bool_t *success) {
    *success = false;
    if (current_allocation->constraint.register_hint.present) {
        REQUIRE_OK(
            allocate_vreg(allocator, current_allocation, current_allocation->constraint.register_hint.vreg, success));

        if (*success) {
            return KEFIR_OK;
        }
    }
    if (current_allocation->constraint.alias_hint.present) {
        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation,
                                    (kefir_graph_node_id_t) current_allocation->constraint.alias_hint.alias, &node));
        ASSIGN_DECL_CAST(const struct kefir_codegen_opt_linear_register_allocation *, ref_allocation, node->value);
        if (ref_allocation->done && ref_allocation->constraint.type == current_allocation->constraint.type) {
            REQUIRE_OK(allocate_vreg(allocator, current_allocation, ref_allocation->allocation, success));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t do_allocation(struct kefir_mem *mem,
                                    struct kefir_codegen_opt_linear_register_allocator *allocator,
                                    struct kefir_codegen_opt_linear_register_allocation *current_allocation,
                                    struct kefir_hashtreeset *conflict_hints) {
    struct kefir_codegen_opt_virtual_register_allocator *vreg_allocator = NULL;
    switch (current_allocation->constraint.type) {
        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_GENERAL_PURPOSE:
            vreg_allocator = &allocator->general_purpose;
            break;

        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_FLOATING_POINT:
            vreg_allocator = &allocator->floating_point;
            break;

        case KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected linear register allocation type constraint");
    }
    kefir_result_t res = kefir_codegen_opt_virtual_register_allocator_allocate_register(
        vreg_allocator, &current_allocation->allocation, conflict_filter, conflict_hints);
    if (res == KEFIR_OUT_OF_SPACE) {
        // TODO Implement register eviction and spilling
        REQUIRE_OK(kefir_codegen_opt_virtual_register_allocator_allocate_any(mem, vreg_allocator,
                                                                             &current_allocation->allocation));
        current_allocation->done = true;
    } else {
        REQUIRE_OK(res);
        current_allocation->done = true;
    }
    return KEFIR_OK;
}

static kefir_result_t allocator_run_impl(struct kefir_mem *mem,
                                         struct kefir_codegen_opt_linear_register_allocator *allocator,
                                         struct kefir_list *alive, struct kefir_hashtreeset *conflict_hints) {
    REQUIRE_OK(propagate_hints(allocator));
    for (kefir_size_t instr_idx = 0; instr_idx < allocator->analysis->linearization_length; instr_idx++) {
        const struct kefir_opt_code_analysis_instruction_properties *instr_props =
            allocator->analysis->linearization[instr_idx];

        struct kefir_graph_node *node = NULL;
        REQUIRE_OK(kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_props->instr_ref, &node));
        ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, current_allocation, node->value);

        REQUIRE_OK(deallocate_dead(mem, allocator, alive, instr_idx));

        if (current_allocation->constraint.type !=
            KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION) {
            REQUIRE_OK(collect_conflict_hints(mem, allocator, conflict_hints, instr_props, current_allocation));
            kefir_bool_t success;
            REQUIRE_OK(attempt_hint_allocation(allocator, current_allocation, &success));
            if (!success) {
                REQUIRE_OK(do_allocation(mem, allocator, current_allocation, conflict_hints));
            }
            REQUIRE_OK(kefir_list_insert_after(mem, alive, kefir_list_tail(alive),
                                               (void *) (kefir_uptr_t) instr_props->instr_ref));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_linear_register_allocator_run(
    struct kefir_mem *mem, struct kefir_codegen_opt_linear_register_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linear register allocator"));

    struct kefir_list alive;
    struct kefir_hashtreeset conflict_hints;
    REQUIRE_OK(kefir_list_init(&alive));
    REQUIRE_OK(kefir_hashtreeset_init(&conflict_hints, &kefir_hashtree_uint_ops));

    kefir_result_t res = allocator_run_impl(mem, allocator, &alive, &conflict_hints);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &alive);
        kefir_hashtreeset_free(mem, &conflict_hints);
        return res;
    });

    res = kefir_list_free(mem, &alive);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &conflict_hints);
        return res;
    });

    REQUIRE_OK(kefir_hashtreeset_free(mem, &conflict_hints));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_linear_register_allocator_allocation_of(
    const struct kefir_codegen_opt_linear_register_allocator *allocator, kefir_opt_instruction_ref_t instr_ref,
    const struct kefir_codegen_opt_linear_register_allocation **allocation_ptr) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linear register allocator"));
    REQUIRE(allocation_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to linear register allocation"));

    struct kefir_graph_node *node = NULL;
    kefir_result_t res = kefir_graph_node(&allocator->allocation, (kefir_graph_node_id_t) instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find a linear register allocation entry");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_codegen_opt_linear_register_allocation *, allocation, node->value);
    *allocation_ptr = allocation;
    return KEFIR_OK;
}
