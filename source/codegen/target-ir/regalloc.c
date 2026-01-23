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

#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/codegen/target-ir/hotness.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct regalloc_state {
    struct kefir_codegen_target_ir_regalloc *regalloc;
    const struct kefir_codegen_target_ir_control_flow *control_flow;
    const struct kefir_codegen_target_ir_liveness *liveness;
    const struct kefir_codegen_target_ir_interference *interference;
    const struct kefir_codegen_target_ir_coalesce *coalesce;
    struct kefir_codegen_target_ir_hotness hotness;
    struct kefir_list block_queue;
    struct kefir_list value_queue;
    struct kefir_codegen_target_ir_regalloc_state regalloc_state;
    const struct kefir_codegen_target_ir_stack_frame *stack_frame;
};

kefir_result_t kefir_codegen_target_ir_regalloc_init(struct kefir_codegen_target_ir_regalloc *regalloc, const struct kefir_codegen_target_ir_regalloc_class *klass) {
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocator"));
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator class"));

    REQUIRE_OK(kefir_hashtable_init(&regalloc->allocation, &kefir_hashtable_uint_ops));
    regalloc->klass = klass;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_regalloc_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_regalloc *regalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));

    REQUIRE_OK(kefir_hashtable_free(mem, &regalloc->allocation));
    return KEFIR_OK;
}

static kefir_result_t build_constraints_and_hints(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_value_ref_t value_ref) {
    REQUIRE_OK(state->regalloc_state.reset(mem, state->regalloc_state.payload));
    
    kefir_result_t res;
    struct kefir_codegen_target_ir_interference_iterator iter;
    kefir_codegen_target_ir_value_ref_t conflict_value_ref;
    for (res = kefir_codegen_target_ir_interference_iter(state->interference, &iter, value_ref, &conflict_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_interference_next(&iter, &conflict_value_ref)) {
        if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(conflict_value_ref.aspect) ||
            KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(conflict_value_ref.aspect)) {
            continue;
        }
        const struct kefir_codegen_target_ir_value_type *conflict_value_type = NULL;
        res = kefir_codegen_target_ir_code_value_props(state->control_flow->code, conflict_value_ref, &conflict_value_type);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        if (conflict_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
            kefir_codegen_target_ir_regalloc_allocation_t allocation;
            REQUIRE_OK(state->regalloc->klass->register_allocation(conflict_value_type->constraint.physical_register, &allocation, state->regalloc->klass->payload));
            REQUIRE_OK(state->regalloc_state.add_conflict(mem, allocation, state->regalloc_state.payload));
            continue;
        }

        kefir_hashtable_value_t table_value;
        res = kefir_hashtable_at(&state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&conflict_value_ref), &table_value);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        REQUIRE_OK(state->regalloc_state.add_conflict(mem, (kefir_codegen_target_ir_regalloc_allocation_t) table_value, state->regalloc_state.payload));        
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (state->coalesce != NULL) {
        struct kefir_codegen_target_ir_coalesce_iterator iter;
        kefir_codegen_target_ir_value_ref_t coalesce_value_ref;
        for (res = kefir_codegen_target_ir_coalesce_iter(state->coalesce, &iter, value_ref, &coalesce_value_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_coalesce_next(&iter, &coalesce_value_ref)) {
            const struct kefir_codegen_target_ir_value_type *coalesce_value_type = NULL;
            res = kefir_codegen_target_ir_code_value_props(state->control_flow->code, coalesce_value_ref, &coalesce_value_type);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            
            if (coalesce_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT) {
                kefir_codegen_target_ir_regalloc_allocation_t allocation;
                REQUIRE_OK(state->regalloc->klass->register_allocation(coalesce_value_type->constraint.physical_register, &allocation, state->regalloc->klass->payload));
                REQUIRE_OK(state->regalloc_state.add_allocation_hint(mem, allocation, state->regalloc_state.payload));
                continue;
            }

            kefir_hashtable_value_t table_value;
            res = kefir_hashtable_at(&state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&coalesce_value_ref), &table_value);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);
            REQUIRE_OK(state->regalloc_state.add_allocation_hint(mem, (kefir_codegen_target_ir_regalloc_allocation_t) table_value, state->regalloc_state.payload));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}

static kefir_result_t get_value_hotness(struct regalloc_state *state, kefir_codegen_target_ir_value_ref_t value_ref, struct kefir_codegen_target_ir_value_hotness_fragment *hotness_fragment) {
    kefir_result_t res = kefir_codegen_target_ir_hotness_get_global(&state->hotness, value_ref, hotness_fragment);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        
        kefir_bool_t is_rematerializable;
        REQUIRE_OK(state->regalloc->klass->is_rematerializable(state->control_flow->code, state->liveness, value_ref, &is_rematerializable, state->regalloc->klass->payload));
        if (is_rematerializable) {
            hotness_fragment->fragment_length <<= 1;
        }
    } else {
        *hotness_fragment = KEFIR_CODEGEN_TARGET_IR_HOTNESS_MAX;
    }
    return KEFIR_OK;
}

static kefir_result_t try_evict_neighbor(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_value_ref_t value_ref) {
    struct kefir_codegen_target_ir_value_hotness_fragment hotness_fragment;
    REQUIRE_OK(get_value_hotness(state, value_ref, &hotness_fragment));
    REQUIRE(!KEFIR_CODEGEN_TARGET_IR_HOTNESS_IS_MAX(&hotness_fragment), KEFIR_OK);
    kefir_codegen_target_ir_value_ref_t evict_value = value_ref;
    kefir_codegen_target_ir_regalloc_allocation_t evict_allocation = 0;

    kefir_result_t res;
    struct kefir_codegen_target_ir_interference_iterator iter;
    kefir_codegen_target_ir_value_ref_t conflict_value_ref;
    for (res = kefir_codegen_target_ir_interference_iter(state->interference, &iter, value_ref, &conflict_value_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_interference_next(&iter, &conflict_value_ref)) {
        const struct kefir_codegen_target_ir_value_type *conflict_value_type = NULL;
        res = kefir_codegen_target_ir_code_value_props(state->control_flow->code, conflict_value_ref, &conflict_value_type);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        if (conflict_value_type->constraint.type == KEFIR_CODEGEN_TARGET_IR_ALLOCATION_REQUIREMENT || 
            (conflict_value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_GENERAL_PURPOSE &&
            conflict_value_type->kind != KEFIR_CODEGEN_TARGET_IR_VALUE_TYPE_FLOATING_POINT)) {
            continue;
        }

        kefir_hashtable_value_t table_value;
        res = kefir_hashtable_at(&state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&conflict_value_ref), &table_value);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        
        kefir_bool_t evictable;
        REQUIRE_OK(state->regalloc->klass->is_evictable(table_value, &evictable, state->regalloc->klass->payload));
        if (!evictable) {
            continue;
        }

        struct kefir_codegen_target_ir_value_hotness_fragment conflict_hotness_fragment;
        REQUIRE_OK(get_value_hotness(state, conflict_value_ref, &conflict_hotness_fragment));
        if (kefir_codegen_target_ir_value_hotness_compare(conflict_hotness_fragment, hotness_fragment) < 0) {
            hotness_fragment = conflict_hotness_fragment;
            evict_value = conflict_value_ref;
            evict_allocation = table_value;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&evict_value) != KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)) {
        REQUIRE_OK(kefir_hashtable_delete(mem, &state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&evict_value)));
        REQUIRE_OK(kefir_list_insert_after(mem, &state->value_queue, kefir_list_tail(&state->value_queue), (void *) (kefir_uptr_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&evict_value)));
        REQUIRE_OK(state->regalloc_state.remove_conflict(mem, evict_allocation, state->regalloc_state.payload));
    }

    return KEFIR_OK;
}

static kefir_result_t do_regalloc(struct kefir_mem *mem, struct regalloc_state *state) {
    for (struct kefir_list_entry *head = kefir_list_head(&state->value_queue);
        head != NULL;
        head = kefir_list_head(&state->value_queue)) {
        kefir_codegen_target_ir_value_ref_t value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM((kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, &state->value_queue, head));

        kefir_result_t res;
        if (state->coalesce != NULL) {
            struct kefir_codegen_target_ir_coalesce_iterator iter;
            kefir_codegen_target_ir_value_ref_t coalesce_value_ref;
            for (res = kefir_codegen_target_ir_coalesce_iter(state->coalesce, &iter, value_ref, &coalesce_value_ref);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_coalesce_next(&iter, &coalesce_value_ref)) {
                kefir_hashtable_value_t table_value;
                res = kefir_hashtable_at(&state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&coalesce_value_ref), &table_value);
                if (res == KEFIR_NOT_FOUND) {
                if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(coalesce_value_ref.aspect) ||
                    KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(coalesce_value_ref.aspect)) {
                    continue;
                }
                    REQUIRE_OK(kefir_list_insert_after(mem, &state->value_queue, NULL, (void *) (kefir_uptr_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&coalesce_value_ref)));
                } else {
                    REQUIRE_OK(res);
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }

        if (kefir_hashtable_has(&state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref))) {
            continue;
        }

        const struct kefir_codegen_target_ir_value_type *value_type = NULL;
        REQUIRE_OK(kefir_codegen_target_ir_code_value_props(state->control_flow->code, value_ref, &value_type));

        REQUIRE_OK(build_constraints_and_hints(mem, state, value_ref));

        kefir_bool_t coalesce_spill = true;
        struct kefir_codegen_target_ir_use_iterator use_iter;
        kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
        kefir_codegen_target_ir_value_ref_t use_value_ref;
        for (res = kefir_codegen_target_ir_code_use_iter(state->control_flow->code, &use_iter, value_ref.instr_ref, &use_instr_ref, &use_value_ref);
            res == KEFIR_OK && coalesce_spill;
            res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, &use_value_ref)) {
            if (use_value_ref.aspect != value_ref.aspect) {
                continue;
            }

            const struct kefir_codegen_target_ir_instruction *user_instr;
            REQUIRE_OK(kefir_codegen_target_ir_code_instruction(state->control_flow->code, use_instr_ref, &user_instr));
            if (user_instr->operation.opcode != state->control_flow->code->klass->upsilon_opcode &&
                user_instr->operation.opcode != state->control_flow->code->klass->assign_opcode &&
                user_instr->operation.opcode != state->control_flow->code->klass->phi_opcode) {
                coalesce_spill = false;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        kefir_codegen_target_ir_regalloc_allocation_t allocation;
        res = state->regalloc->klass->do_allocate(mem, value_type, state->stack_frame, state->regalloc_state.payload, true, coalesce_spill, &allocation, state->regalloc->klass->payload);
        if (res == KEFIR_OUT_OF_SPACE) {
            REQUIRE_OK(try_evict_neighbor(mem, state, value_ref));
            REQUIRE_OK(state->regalloc->klass->do_allocate(mem, value_type, state->stack_frame, state->regalloc_state.payload, false, coalesce_spill, &allocation, state->regalloc->klass->payload));
        } else {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_hashtable_insert(mem, &state->regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), (kefir_hashtable_value_t) allocation));   
    }
    return KEFIR_OK;
}

static kefir_result_t do_regalloc_block(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_block_ref_t block_ref) {
    for (kefir_codegen_target_ir_instruction_ref_t instr_ref = kefir_codegen_target_ir_code_block_control_head(state->control_flow->code, block_ref);
        instr_ref != KEFIR_ID_NONE;
        instr_ref = kefir_codegen_target_ir_code_control_next(state->control_flow->code, instr_ref)) {
        struct kefir_codegen_target_ir_value_iterator value_iter;
        struct kefir_codegen_target_ir_value_ref value_ref;
        kefir_result_t res;
        for (res = kefir_codegen_target_ir_code_value_iter(state->control_flow->code, &value_iter, instr_ref, &value_ref, NULL);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_value_next(&value_iter, &value_ref, NULL)) {
            if (KEFIR_CODEGEN_TARGET_IR_VALUE_IS_INDIRECT_OUTPUT(value_ref.aspect) ||
                KEFIR_CODEGEN_TARGET_IR_VALUE_IS_RESOURCE(value_ref.aspect)) {
                continue;
            }
            REQUIRE_OK(kefir_list_insert_after(mem, &state->value_queue, NULL, (void *) (kefir_uptr_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref)));
            REQUIRE_OK(do_regalloc(mem, state));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }   
    }

    return KEFIR_OK;
}

static kefir_result_t regalloc_run_impl(struct kefir_mem *mem, struct regalloc_state *state, kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE_OK(kefir_codegen_target_ir_hotness_build(mem, &state->hotness, state->control_flow, state->liveness));

    REQUIRE_OK(kefir_list_insert_after(mem, &state->block_queue, kefir_list_tail(&state->block_queue), (void *) (kefir_uptr_t) block_ref));
    for (struct kefir_list_entry *head = kefir_list_head(&state->block_queue);
        head != NULL;
        head = kefir_list_head(&state->block_queue)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, block_ref, (kefir_uptr_t) head->value);
        REQUIRE_OK(kefir_list_pop(mem, &state->block_queue, head));
        REQUIRE_OK(do_regalloc_block(mem, state, block_ref));

        kefir_result_t res;
        struct kefir_codegen_target_ir_control_flow_dominator_tree_iterator iter;
        kefir_codegen_target_ir_block_ref_t dominated_block_ref;
        for (res = kefir_codegen_target_ir_control_flow_dominator_tree_iter(state->control_flow, &iter, block_ref, &dominated_block_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_control_flow_dominator_tree_next(&iter, &dominated_block_ref)) {
            REQUIRE_OK(kefir_list_insert_after(mem, &state->block_queue, kefir_list_tail(&state->block_queue), (void *) (kefir_uptr_t) dominated_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_size_t kefir_codegen_target_ir_regalloc_num_of_allocations(const struct kefir_codegen_target_ir_regalloc *regalloc) {
    REQUIRE(regalloc != NULL, 0);

    return regalloc->allocation.occupied;
}

kefir_result_t kefir_codegen_target_ir_regalloc_forget(struct kefir_mem *mem, struct kefir_codegen_target_ir_regalloc *regalloc, kefir_codegen_target_ir_value_ref_t value_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));

    kefir_result_t res = kefir_hashtable_delete(mem, &regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref));
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_regalloc_reset(struct kefir_mem *mem, struct kefir_codegen_target_ir_regalloc *regalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));

    REQUIRE_OK(kefir_hashtable_clear(mem, &regalloc->allocation));
    REQUIRE_OK(kefir_hashtable_trim(mem, &regalloc->allocation));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_regalloc_run(struct kefir_mem *mem, struct kefir_codegen_target_ir_regalloc *regalloc,
    const struct kefir_codegen_target_ir_control_flow *control_flow,
    const struct kefir_codegen_target_ir_liveness *liveness,
    const struct kefir_codegen_target_ir_interference *interference,
    const struct kefir_codegen_target_ir_coalesce *coalesce,
    const struct kefir_codegen_target_ir_stack_frame *stack_frame) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR liveness"));

    struct regalloc_state state = {
        .regalloc = regalloc,
        .control_flow = control_flow,
        .liveness = liveness,
        .interference = interference,
        .coalesce = coalesce,
        .stack_frame = stack_frame
    };
    REQUIRE_OK(kefir_list_init(&state.block_queue));
    REQUIRE_OK(kefir_list_init(&state.value_queue));
    REQUIRE_OK(kefir_codegen_target_ir_hotness_init(&state.hotness));
    REQUIRE_OK(regalloc->klass->new_state(mem, &state.regalloc_state, regalloc->klass->payload));
    kefir_result_t res = regalloc_run_impl(mem, &state, control_flow->code->entry_block);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_hotness_free(mem, &state.hotness);
        kefir_list_free(mem, &state.block_queue);
        kefir_list_free(mem, &state.value_queue);
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    res = kefir_codegen_target_ir_hotness_free(mem, &state.hotness);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.block_queue);
        kefir_list_free(mem, &state.value_queue);
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    res = kefir_list_free(mem, &state.block_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.value_queue);
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    res = kefir_list_free(mem, &state.value_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        state.regalloc_state.free_state(mem, state.regalloc_state.payload);
        return res;
    });
    REQUIRE_OK(state.regalloc_state.free_state(mem, state.regalloc_state.payload));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_regalloc_get(const struct kefir_codegen_target_ir_regalloc *regalloc,
    kefir_codegen_target_ir_value_ref_t value_ref,
    kefir_codegen_target_ir_regalloc_allocation_t *allocation_ptr) {
    REQUIRE(regalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR register allocator"));
    REQUIRE(allocation_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR register allocation"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&regalloc->allocation, (kefir_hashtable_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&value_ref), &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find register allocation for requested target IR value");
    }
    REQUIRE_OK(res);
    *allocation_ptr = (kefir_codegen_target_ir_regalloc_allocation_t) table_value;
    return KEFIR_OK;
}
