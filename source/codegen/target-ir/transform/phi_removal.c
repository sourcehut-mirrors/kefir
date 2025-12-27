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

#include "kefir/codegen/target-ir/transform.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct phi_scc {
    struct kefir_hashset phi_refs;
    struct kefir_hashset outside_inputs;
};

struct phi_scc_traversal {
    kefir_size_t current_index;
    struct kefir_hashtable indices;
    struct kefir_list stack;
    struct kefir_hashset on_stack;
    struct kefir_list scc_list;
};

static kefir_result_t phi_scc_tarjan_strongconnect(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct phi_scc_traversal *traversal, kefir_codegen_target_ir_instruction_ref_t instr_ref) {
    UNUSED(code);

    kefir_uint64_t index_pair = (((kefir_uint64_t) traversal->current_index) << 32) | (kefir_uint32_t) traversal->current_index;
    REQUIRE_OK(kefir_hashtable_insert(mem, &traversal->indices, (kefir_hashtable_key_t) instr_ref, (kefir_hashtable_value_t) index_pair));
    traversal->current_index++;

    REQUIRE_OK(kefir_list_insert_after(mem, &traversal->stack, NULL, (void *) (kefir_uptr_t) instr_ref));
    REQUIRE_OK(kefir_hashset_add(mem, &traversal->on_stack, (kefir_hashset_key_t) instr_ref));

    kefir_result_t res;
    kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
    struct kefir_codegen_target_ir_use_iterator use_iter;
    for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, instr_ref, &use_instr_ref, NULL);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, NULL)) {
        const struct kefir_codegen_target_ir_instruction *use_instr;
        REQUIRE_OK(kefir_codegen_target_ir_code_instruction(code, use_instr_ref, &use_instr));
        if (use_instr->operation.opcode != code->klass->phi_opcode) {
            continue;
        }

        kefir_hashtable_value_t table_value;
        res = kefir_hashtable_at(&traversal->indices, (kefir_hashtable_key_t) use_instr_ref, &table_value);
        if (res == KEFIR_NOT_FOUND) {
            REQUIRE_OK(phi_scc_tarjan_strongconnect(mem, code, traversal, use_instr_ref));
            REQUIRE_OK(kefir_hashtable_at(&traversal->indices, (kefir_hashtable_key_t) use_instr_ref, &table_value));
            kefir_uint32_t use_lowlink = ((kefir_uint64_t) table_value) >> 32;

            kefir_hashtable_value_t *table_value_ptr;
            REQUIRE_OK(kefir_hashtable_at_mut(&traversal->indices, (kefir_hashtable_key_t) instr_ref, &table_value_ptr));
            kefir_uint32_t instr_lowlink = ((kefir_uint64_t) *table_value_ptr) >> 32;
            kefir_uint32_t instr_index = (kefir_uint32_t) *table_value_ptr;

            instr_lowlink = MIN(instr_lowlink, use_lowlink);
            *table_value_ptr = (((kefir_uint64_t) instr_lowlink) << 32) | (kefir_uint32_t) instr_index;
        } else {
            REQUIRE_OK(res);
            if (kefir_hashset_has(&traversal->on_stack, (kefir_hashset_key_t) use_instr_ref)) {
                REQUIRE_OK(kefir_hashtable_at(&traversal->indices, (kefir_hashtable_key_t) use_instr_ref, &table_value));
                kefir_uint32_t use_index = (kefir_uint32_t) table_value;

                kefir_hashtable_value_t *table_value_ptr;
                REQUIRE_OK(kefir_hashtable_at_mut(&traversal->indices, (kefir_hashtable_key_t) instr_ref, &table_value_ptr));
                kefir_uint32_t instr_lowlink = ((kefir_uint64_t) *table_value_ptr) >> 32;
                kefir_uint32_t instr_index = (kefir_uint32_t) *table_value_ptr;

                instr_lowlink = MIN(instr_lowlink, use_index);
                *table_value_ptr = (((kefir_uint64_t) instr_lowlink) << 32) | (kefir_uint32_t) instr_index;
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_at(&traversal->indices, (kefir_hashtable_key_t) instr_ref, &table_value));
    kefir_uint32_t instr_lowlink = ((kefir_uint64_t) table_value) >> 32;
    kefir_uint32_t instr_index = (kefir_uint32_t) table_value;
    if (instr_lowlink == instr_index) {
        struct phi_scc *scc = KEFIR_MALLOC(mem, sizeof(struct phi_scc));
        REQUIRE(scc != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate SCC"));
        res = kefir_hashset_init(&scc->phi_refs, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashset_init(&scc->outside_inputs, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_list_insert_after(mem, &traversal->scc_list, NULL, (void *) scc));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, scc);
            return res;
        });

        kefir_codegen_target_ir_instruction_ref_t stack_instr_ref = KEFIR_ID_NONE;
        do {
            struct kefir_list_entry *iter = kefir_list_head(&traversal->stack);
            REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected non-empty stack in SCC"));
            stack_instr_ref = (kefir_codegen_target_ir_instruction_ref_t) (kefir_uptr_t) iter->value;
            REQUIRE_OK(kefir_list_pop(mem, &traversal->stack, iter));
            REQUIRE_OK(kefir_hashset_delete(&traversal->on_stack, (kefir_hashset_key_t) stack_instr_ref));
            REQUIRE_OK(kefir_hashset_add(mem, &scc->phi_refs, (kefir_hashset_key_t) stack_instr_ref));
        } while (stack_instr_ref != instr_ref);
    }
    return KEFIR_OK;
}

static kefir_result_t get_output_for(const struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_instruction_ref_t instr_ref, kefir_codegen_target_ir_value_ref_t *value_ref_ptr) {
    struct kefir_codegen_target_ir_value_iterator iter;
    REQUIRE_OK(kefir_codegen_target_ir_code_value_iter(code, &iter, instr_ref, value_ref_ptr, NULL));
    return KEFIR_OK;
}

static kefir_result_t scan_scc_uses(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct phi_scc_traversal *traversal, struct kefir_hashset *removal_set) {
    for (struct kefir_list_entry *scc_iter = kefir_list_head(&traversal->scc_list);
        scc_iter != NULL;) {
        ASSIGN_DECL_CAST(struct phi_scc *, scc, scc_iter->value);

        kefir_bool_t no_external_uses = true;

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&scc->phi_refs, &iter, &key); res == KEFIR_OK && no_external_uses;
            res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_instruction_ref_t, phi_ref, key);
            
            struct kefir_codegen_target_ir_use_iterator use_iter;
            kefir_codegen_target_ir_instruction_ref_t use_instr_ref;
            for (res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, phi_ref, &use_instr_ref, NULL);
                res == KEFIR_OK && no_external_uses;
                res = kefir_codegen_target_ir_code_use_next(&use_iter, &use_instr_ref, NULL)) {
                if (!kefir_hashset_has(&scc->phi_refs, (kefir_hashset_key_t) use_instr_ref)) {
                    no_external_uses = false;
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (!no_external_uses) {
            scc_iter = scc_iter->next;
            continue;
        }

        for (res = kefir_hashset_iter(&scc->phi_refs, &iter, &key); res == KEFIR_OK && no_external_uses;
            res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_instruction_ref_t, phi_ref, key);
            for (;;) {
                struct kefir_codegen_target_ir_value_phi_link_iterator link_iter;
                kefir_codegen_target_ir_block_ref_t link_block_ref;
                res = kefir_codegen_target_ir_code_phi_link_iter(code, &link_iter, phi_ref, &link_block_ref, NULL);
                if (res == KEFIR_ITERATOR_END) {
                    break;
                }
                REQUIRE_OK(res);
                REQUIRE_OK(kefir_codegen_target_ir_code_phi_drop(mem, code, phi_ref, link_block_ref));
            }

            REQUIRE_OK(kefir_hashset_add(mem, removal_set, (kefir_hashset_key_t) phi_ref));
        }
        
        struct kefir_list_entry *next_iter = scc_iter->next;
        REQUIRE_OK(kefir_list_pop(mem, &traversal->scc_list, scc_iter));
        scc_iter = next_iter;
    }
    return KEFIR_OK;
}

static kefir_result_t scan_scc_inputs(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct phi_scc_traversal *traversal, struct kefir_hashset *removal_set) {
    for (struct kefir_list_entry *scc_iter = kefir_list_head(&traversal->scc_list);
        scc_iter != NULL;) {
        ASSIGN_DECL_CAST(struct phi_scc *, scc, scc_iter->value);

        REQUIRE_OK(kefir_hashset_clear(mem, &scc->outside_inputs));
        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&scc->phi_refs, &iter, &key); res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_instruction_ref_t, phi_ref, key);

            struct kefir_codegen_target_ir_value_phi_link_iterator iter;
            kefir_codegen_target_ir_block_ref_t link_block_ref;
            struct kefir_codegen_target_ir_value_ref link_value_ref, first_link_value_ref = {
                .instr_ref = KEFIR_ID_NONE
            };
            kefir_bool_t do_drop = true;
            for (res = kefir_codegen_target_ir_code_phi_link_iter(code, &iter, phi_ref, &link_block_ref, &link_value_ref);
                res == KEFIR_OK;
                res = kefir_codegen_target_ir_code_phi_link_next(&iter, &link_block_ref, &link_value_ref)) {
                if (!kefir_hashset_has(&scc->phi_refs, (kefir_hashset_key_t) link_value_ref.instr_ref)) {
                    REQUIRE_OK(kefir_hashset_add(mem, &scc->outside_inputs, (kefir_hashset_key_t) KEFIR_CODEGEN_TARGET_IR_VALUE_REF_INTO(&link_value_ref)));
                }

                if (link_value_ref.instr_ref != phi_ref) {
                    if (first_link_value_ref.instr_ref == KEFIR_ID_NONE) {
                        first_link_value_ref = link_value_ref;
                    } else if (first_link_value_ref.instr_ref != link_value_ref.instr_ref || first_link_value_ref.aspect != link_value_ref.aspect) {
                        do_drop = false;
                    }
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            if (do_drop) {
                kefir_codegen_target_ir_value_ref_t phi_value_ref;
                REQUIRE_OK(get_output_for(code, phi_ref, &phi_value_ref));
                REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, first_link_value_ref, phi_value_ref));
                REQUIRE_OK(kefir_hashset_add(mem, removal_set, (kefir_hashset_key_t) phi_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_hashset_subtract(&scc->phi_refs, removal_set));

        if (kefir_hashset_size(&scc->outside_inputs) == 1) {
            REQUIRE_OK(kefir_hashset_iter(&scc->outside_inputs, &iter, &key));
            kefir_codegen_target_ir_value_ref_t value_ref = KEFIR_CODEGEN_TARGET_IR_VALUE_REF_FROM(key);

            for (res = kefir_hashset_iter(&scc->phi_refs, &iter, &key); res == KEFIR_OK;
                res = kefir_hashset_next(&iter, &key)) {
                ASSIGN_DECL_CAST(kefir_codegen_target_ir_instruction_ref_t, phi_ref, key);
                kefir_codegen_target_ir_value_ref_t phi_value_ref;
                REQUIRE_OK(get_output_for(code, phi_ref, &phi_value_ref));
                REQUIRE_OK(kefir_codegen_target_ir_code_replace_value(mem, code, value_ref, phi_value_ref));
                REQUIRE_OK(kefir_hashset_add(mem, removal_set, (kefir_hashset_key_t) phi_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            struct kefir_list_entry *next_iter = scc_iter->next;
            REQUIRE_OK(kefir_list_pop(mem, &traversal->scc_list, scc_iter));
            scc_iter = next_iter;
        } else {
            scc_iter = scc_iter->next;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t remove_unused_phis_from_block(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_codegen_target_ir_block_ref_t block_ref, struct kefir_hashset *removal_set) {
    REQUIRE_OK(kefir_hashset_clear(mem, removal_set));

    kefir_result_t res;
    struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
    kefir_codegen_target_ir_instruction_ref_t phi_ref;
    for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_node_iter, block_ref, &phi_ref);
        res == KEFIR_OK;
        res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
        struct kefir_codegen_target_ir_use_iterator use_iter;
        res = kefir_codegen_target_ir_code_use_iter(code, &use_iter, phi_ref, NULL, NULL);
        if (res == KEFIR_ITERATOR_END) {
            REQUIRE_OK(kefir_hashset_add(mem, removal_set, (kefir_hashset_key_t) phi_ref));
        } else {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_hashset_iterator removal_iter;
    kefir_hashset_key_t removal_key;
    for (res = kefir_hashset_iter(removal_set, &removal_iter, &removal_key); res == KEFIR_OK;
         res = kefir_hashset_next(&removal_iter, &removal_key)) {
        REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, (kefir_codegen_target_ir_instruction_ref_t) removal_key));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t phi_scc_tarjan_impl(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct phi_scc_traversal *traversal, struct kefir_hashset *removal_set, const struct kefir_codegen_target_ir_control_flow *control_flow, kefir_bool_t coalesce_phis) {
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_OK(remove_unused_phis_from_block(mem, code, block_ref, removal_set));
    }

    REQUIRE_OK(kefir_hashtable_clear(&traversal->indices));
    REQUIRE_OK(kefir_hashset_clear(mem, &traversal->on_stack));
    REQUIRE_OK(kefir_list_clear(mem, &traversal->stack));
    REQUIRE_OK(kefir_list_clear(mem, &traversal->scc_list));
    traversal->current_index = 0;
    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        if (!kefir_codegen_target_ir_control_flow_is_reachable(control_flow, block_ref)) {
            continue;
        }
        kefir_result_t res;
        struct kefir_codegen_target_ir_value_phi_node_iterator phi_node_iter;
        kefir_codegen_target_ir_instruction_ref_t phi_ref;
        for (res = kefir_codegen_target_ir_code_phi_node_iter(code, &phi_node_iter, block_ref, &phi_ref);
            res == KEFIR_OK;
            res = kefir_codegen_target_ir_code_phi_node_next(&phi_node_iter, &phi_ref)) {
            if (!kefir_hashtable_has(&traversal->indices, (kefir_hashtable_key_t) phi_ref)) {
                REQUIRE_OK(phi_scc_tarjan_strongconnect(mem, code, traversal, phi_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    kefir_bool_t fixpoint_reached = false;
    for (; !fixpoint_reached;) {
        REQUIRE_OK(kefir_hashset_clear(mem, removal_set));
        REQUIRE_OK(scan_scc_uses(mem, code, traversal, removal_set));
        if (coalesce_phis) {
            REQUIRE_OK(scan_scc_inputs(mem, code, traversal, removal_set));
        }
        fixpoint_reached = kefir_hashset_size(removal_set) == 0;

        struct kefir_hashset_iterator removal_iter;
        kefir_hashset_key_t removal_key;
        kefir_result_t res;
        for (res = kefir_hashset_iter(removal_set, &removal_iter, &removal_key); res == KEFIR_OK;
            res = kefir_hashset_next(&removal_iter, &removal_key)) {
            REQUIRE_OK(kefir_codegen_target_ir_code_drop_instruction(mem, code, (kefir_codegen_target_ir_instruction_ref_t) removal_key));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    for (kefir_size_t i = 0; i < kefir_codegen_target_ir_code_block_count(code); i++) {
        kefir_codegen_target_ir_block_ref_t block_ref = kefir_codegen_target_ir_code_block_by_index(code, i);
        REQUIRE_OK(remove_unused_phis_from_block(mem, code, block_ref, removal_set));
    }
    return KEFIR_OK;
}

static kefir_result_t free_scc(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct phi_scc *, scc,
        entry->value);
    REQUIRE(scc != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Expected valid strongly connected component"));

    REQUIRE_OK(kefir_hashset_free(mem, &scc->phi_refs));
    REQUIRE_OK(kefir_hashset_free(mem, &scc->outside_inputs));
    KEFIR_FREE(mem, scc);
    return KEFIR_OK;
}

static kefir_result_t phi_scc_tarjan(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashset *removal_set, const struct kefir_codegen_target_ir_control_flow *control_flow, kefir_bool_t coealsce_phis) {
    struct phi_scc_traversal traversal = {
        .current_index = 0
    };
    REQUIRE_OK(kefir_hashtable_init(&traversal.indices, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&traversal.stack));
    REQUIRE_OK(kefir_hashset_init(&traversal.on_stack, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&traversal.scc_list));
    REQUIRE_OK(kefir_list_on_remove(&traversal.scc_list, free_scc, NULL));

    kefir_result_t res = phi_scc_tarjan_impl(mem, code, &traversal, removal_set, control_flow, coealsce_phis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &traversal.scc_list);
        kefir_hashset_free(mem, &traversal.on_stack);
        kefir_list_free(mem, &traversal.stack);
        kefir_hashtable_free(mem, &traversal.indices);
        return res;
    });
    res = kefir_list_free(mem, &traversal.scc_list);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &traversal.on_stack);
        kefir_list_free(mem, &traversal.stack);
        kefir_hashtable_free(mem, &traversal.indices);
        return res;
    });
    res = kefir_hashset_free(mem, &traversal.on_stack);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &traversal.stack);
        kefir_hashtable_free(mem, &traversal.indices);
        return res;
    });
    res = kefir_list_free(mem, &traversal.stack);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &traversal.indices);
        return res;
    });
    REQUIRE_OK(kefir_hashtable_free(mem, &traversal.indices));
    return KEFIR_OK;
}

static kefir_result_t remove_phis(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, struct kefir_hashset *removal_set, struct kefir_codegen_target_ir_control_flow *control_flow, kefir_bool_t coalesce_phis) {
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_build(mem, control_flow));
    REQUIRE_OK(phi_scc_tarjan(mem, code, removal_set, control_flow, coalesce_phis));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_transform_phi_removal(struct kefir_mem *mem, struct kefir_codegen_target_ir_code *code, kefir_bool_t coalesce_phis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    struct kefir_hashset removal_set;
    REQUIRE_OK(kefir_hashset_init(&removal_set, &kefir_hashtable_uint_ops));

    struct kefir_codegen_target_ir_control_flow control_flow;
    REQUIRE_OK(kefir_codegen_target_ir_control_flow_init(&control_flow, code));

    kefir_result_t res = remove_phis(mem, code, &removal_set, &control_flow, coalesce_phis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
        kefir_hashset_free(mem, &removal_set);
        return res;
    });
    res = kefir_codegen_target_ir_control_flow_free(mem, &control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &removal_set);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &removal_set));
    return KEFIR_OK;
}
