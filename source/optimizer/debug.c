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

#include "kefir/optimizer/debug.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_allocation(struct kefir_mem *mem, struct kefir_hashtable *table,
                                               kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_allocation_placement *, placement, value);
    if (placement != NULL) {
        REQUIRE_OK(kefir_hashset_free(mem, &placement->placement));
        KEFIR_FREE(mem, placement);
    }
    return KEFIR_OK;
}

static kefir_result_t free_local_variable(struct kefir_mem *mem, struct kefir_hashtable *table,
                                               kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_local_variable *, local_variable, value);
    REQUIRE_OK(kefir_hashset_free(mem, &local_variable->allocations));
    KEFIR_FREE(mem, local_variable);
    return KEFIR_OK;
}

static kefir_result_t free_code_ref_instrs(struct kefir_mem *mem, struct kefir_hashtable *table,
                                               kefir_hashtable_key_t key, kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_code_reference *, code_ref, value);
    REQUIRE_OK(kefir_hashset_free(mem, &code_ref->instructions));
    KEFIR_FREE(mem, code_ref);
    return KEFIR_OK;
}

static kefir_result_t on_new_instruction(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                         kefir_opt_instruction_ref_t instr_ref, void *payload) {
    UNUSED(code);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info *, debug_info, payload);
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    if (debug_info->next_instruction_code_ref != KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE) {
        REQUIRE_OK(kefir_hashtable_insert(mem, &debug_info->instruction_code_refs, (kefir_hashtable_key_t) instr_ref,
                                         (kefir_hashtable_value_t) debug_info->next_instruction_code_ref));

        kefir_hashtable_value_t table_value;
        kefir_result_t res = kefir_hashtable_at(&debug_info->code_ref_instructions, (kefir_hashtable_key_t) debug_info->next_instruction_code_ref, &table_value);
        struct kefir_opt_code_debug_info_code_reference *code_ref = NULL;
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            code_ref = (struct kefir_opt_code_debug_info_code_reference *) table_value;
        } else {
            code_ref = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_debug_info_code_reference));
            REQUIRE(code_ref != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate debug information code reference"));
            code_ref->code_ref = debug_info->next_instruction_code_ref;
            res = kefir_hashset_init(&code_ref->instructions, &kefir_hashtable_uint_ops);
            REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &debug_info->code_ref_instructions, (kefir_hashtable_key_t) debug_info->next_instruction_code_ref, (kefir_hashtable_value_t) code_ref));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, code_ref);
                return res;
            });
        }

        REQUIRE_OK(kefir_hashset_add(mem, &code_ref->instructions, (kefir_hashset_key_t) instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t kefir_opt_code_debug_info_replace_reference(struct kefir_mem *, struct kefir_opt_code_debug_info *,
                                                                kefir_opt_instruction_ref_t,
                                                                kefir_opt_instruction_ref_t);

static kefir_result_t on_replace_instruction(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                         kefir_opt_instruction_ref_t to_instr_ref, kefir_opt_instruction_ref_t from_instr_ref, void *payload) {
    UNUSED(code);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(to_instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    REQUIRE(from_instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info *, debug_info, payload);
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    REQUIRE_OK(kefir_opt_code_debug_info_replace_reference(mem, debug_info, from_instr_ref, to_instr_ref));
    return KEFIR_OK;
}

static kefir_result_t on_drop_instruction(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                         kefir_opt_instruction_ref_t instr_ref, void *payload) {
    UNUSED(code);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info *, debug_info, payload);
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    REQUIRE_OK(kefir_opt_code_debug_info_replace_reference(mem, debug_info, instr_ref, KEFIR_ID_NONE));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&debug_info->instruction_code_refs, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(kefir_opt_code_debug_info_code_ref_t, code_ref, table_value);
        res = kefir_hashtable_at(&debug_info->code_ref_instructions, (kefir_hashtable_key_t) code_ref, &table_value);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_code_reference *, code_reference, table_value);
            REQUIRE_OK(kefir_hashset_delete(&code_reference->instructions, (kefir_hashset_key_t) instr_ref));
        }
        REQUIRE_OK(kefir_hashtable_delete(mem, &debug_info->instruction_code_refs, (kefir_hashtable_key_t) instr_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_init(struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(debug_info != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer debug information"));

    debug_info->next_instruction_code_ref = KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE;
    debug_info->listener.on_new_instruction = on_new_instruction;
    debug_info->listener.on_replace_instruction = on_replace_instruction;
    debug_info->listener.on_drop_instruction = on_drop_instruction;
    debug_info->listener.payload = debug_info;
    debug_info->record_debug_info = true;
    REQUIRE_OK(kefir_hashtable_init(&debug_info->instruction_code_refs, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&debug_info->code_ref_instructions, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&debug_info->code_ref_instructions, free_code_ref_instrs, NULL));
    REQUIRE_OK(kefir_hashtable_init(&debug_info->local_variables, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&debug_info->local_variables, free_local_variable, NULL));
    REQUIRE_OK(kefir_hashtable_init(&debug_info->allocations, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&debug_info->allocations, free_allocation, NULL));
    REQUIRE_OK(kefir_hashset_init(&debug_info->active_refs, &kefir_hashtable_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_free(struct kefir_mem *mem, struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    REQUIRE_OK(kefir_hashset_free(mem, &debug_info->active_refs));
    REQUIRE_OK(kefir_hashtable_free(mem, &debug_info->local_variables));
    REQUIRE_OK(kefir_hashtable_free(mem, &debug_info->allocations));
    REQUIRE_OK(kefir_hashtable_free(mem, &debug_info->code_ref_instructions));
    REQUIRE_OK(kefir_hashtable_free(mem, &debug_info->instruction_code_refs));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_next_instruction_code_reference(struct kefir_opt_code_debug_info *debug_info,
                                                                         kefir_size_t code_ref) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    debug_info->next_instruction_code_ref = code_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_next_instruction_code_reference_of(
    struct kefir_opt_code_debug_info *debug_info, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(debug_info->record_debug_info, KEFIR_OK);

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&debug_info->instruction_code_refs, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        debug_info->next_instruction_code_ref = (kefir_opt_code_debug_info_local_variable_ref_t) table_value;
    } else {
        debug_info->next_instruction_code_ref = KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_instruction_code_reference(const struct kefir_opt_code_debug_info *debug_info,
                                                              kefir_opt_instruction_ref_t instr_ref,
                                                              kefir_opt_code_debug_info_code_ref_t *code_ref_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    REQUIRE(code_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to instruction location"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&debug_info->instruction_code_refs, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *code_ref_ptr = (kefir_opt_code_debug_info_local_variable_ref_t) table_value;
    } else {
        *code_ref_ptr = KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE;
    }
    return KEFIR_OK;
}


kefir_result_t kefir_opt_code_debug_info_code_reference(const struct kefir_opt_code_debug_info *debug_info,
                                                              kefir_opt_code_debug_info_code_ref_t code_ref, const struct kefir_opt_code_debug_info_code_reference **code_reference_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(code_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information code reference"));
    REQUIRE(code_reference_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to code reference information"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&debug_info->code_ref_instructions, (kefir_hashtable_key_t) code_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find optimizer debug information code reference instructions");
    }
    REQUIRE_OK(res);
    *code_reference_ptr = (const struct kefir_opt_code_debug_info_code_reference *) table_value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_add_local_variable_allocation(struct kefir_mem *mem, struct kefir_opt_code_debug_info *debug_info,
    kefir_opt_code_debug_info_local_variable_ref_t variable_ref, kefir_opt_instruction_ref_t allocation_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(debug_info->record_debug_info, KEFIR_OK);
    
    struct kefir_opt_code_debug_info_local_variable *local_variable = NULL;

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&debug_info->local_variables, (kefir_hashtable_key_t) variable_ref, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        local_variable = (struct kefir_opt_code_debug_info_local_variable *) table_value;
    } else {
        local_variable = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_debug_info_local_variable));
        REQUIRE(local_variable != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate local variable debug information"));
        local_variable->variable_ref = variable_ref;
        res = kefir_hashset_init(&local_variable->allocations, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &debug_info->local_variables, (kefir_hashtable_key_t) variable_ref, (kefir_hashtable_value_t) local_variable));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, local_variable);
            return res;
        });
    }
    
    REQUIRE_OK(kefir_hashset_add(mem, &local_variable->allocations, (kefir_hashset_key_t) allocation_ref));
    REQUIRE_OK(kefir_hashset_add(mem, &debug_info->active_refs, (kefir_hashset_key_t) allocation_ref));
    return KEFIR_OK;    
}

kefir_result_t kefir_opt_code_debug_info_add_allocation_placement(struct kefir_mem *mem, struct kefir_opt_code_debug_info *debug_info,
    kefir_opt_instruction_ref_t allocation_ref, kefir_opt_instruction_ref_t placement_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(debug_info->record_debug_info, KEFIR_OK);

    struct kefir_opt_code_debug_info_allocation_placement *placement = NULL;

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&debug_info->allocations, (kefir_hashtable_key_t) allocation_ref, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        placement = (struct kefir_opt_code_debug_info_allocation_placement *) table_value;
    } else {
        placement = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_debug_info_allocation_placement));
        REQUIRE(placement != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate allocation placement debug information"));
        placement->allocation_ref = allocation_ref;
        res = kefir_hashset_init(&placement->placement, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &debug_info->allocations, (kefir_hashtable_key_t) allocation_ref, (kefir_hashtable_value_t) placement));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, placement);
            return res;
        });
    }
    
    REQUIRE_OK(kefir_hashset_add(mem, &placement->placement, (kefir_hashset_key_t) placement_ref));
    REQUIRE_OK(kefir_hashset_add(mem, &debug_info->active_refs, (kefir_hashset_key_t) placement_ref));
    return KEFIR_OK;    
}

kefir_result_t kefir_opt_code_debug_info_local_variable(const struct kefir_opt_code_debug_info *debug_info, kefir_opt_code_debug_info_local_variable_ref_t variable_ref, const struct kefir_opt_code_debug_info_local_variable **local_variable_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&debug_info->local_variables, (kefir_hashtree_key_t) variable_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Uable to find local variable debug information");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_local_variable *, local_variable, table_value);

    ASSIGN_PTR(local_variable_ptr, local_variable);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_allocation_placement(const struct kefir_opt_code_debug_info *debug_info, kefir_opt_instruction_ref_t instr_ref, const struct kefir_opt_code_debug_info_allocation_placement **placement_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&debug_info->allocations, (kefir_hashtree_key_t) instr_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Uable to find allocatio placement debug information");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_allocation_placement *, placement, table_value);

    ASSIGN_PTR(placement_ptr, placement);
    return KEFIR_OK;
}

static kefir_result_t kefir_opt_code_debug_info_replace_reference(struct kefir_mem *mem,
                                                                struct kefir_opt_code_debug_info *debug_info,
                                                                kefir_opt_instruction_ref_t instr_ref,
                                                                kefir_opt_instruction_ref_t replacement_instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(kefir_hashset_has(&debug_info->active_refs, (kefir_hashset_key_t) instr_ref), KEFIR_OK);

    kefir_result_t res;
    struct kefir_hashtable_iterator iter;
    kefir_hashtable_value_t table_value;
    for (res = kefir_hashtable_iter(&debug_info->local_variables, &iter, NULL, &table_value);
        res == KEFIR_OK; res = kefir_hashtable_next(&iter, NULL, &table_value)) {
        ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_local_variable *, local_variable, table_value);
        if (kefir_hashset_has(&local_variable->allocations, (kefir_hashset_key_t) instr_ref)) {
            REQUIRE_OK(kefir_hashset_delete(&local_variable->allocations, (kefir_hashset_key_t) instr_ref));
            if (replacement_instr_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_hashset_add(mem, &local_variable->allocations, (kefir_hashset_key_t) replacement_instr_ref));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_hashtable_iter(&debug_info->allocations, &iter, NULL, &table_value);
        res == KEFIR_OK; res = kefir_hashtable_next(&iter, NULL, &table_value)) {
        ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_allocation_placement *, placement, table_value);
        if (kefir_hashset_has(&placement->placement, (kefir_hashset_key_t) instr_ref)) {
            REQUIRE_OK(kefir_hashset_delete(&placement->placement, (kefir_hashset_key_t) instr_ref));
            if (replacement_instr_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_hashset_add(mem, &placement->placement, (kefir_hashset_key_t) replacement_instr_ref));
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_hashtable_value_t *table_value_ptr;
    res = kefir_hashtable_at_mut(&debug_info->allocations, (kefir_hashtable_key_t) instr_ref, &table_value_ptr);
    if (res != KEFIR_NOT_FOUND) {
        if (replacement_instr_ref != KEFIR_ID_NONE) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_allocation_placement *, placement, *table_value_ptr);
            *table_value_ptr = (kefir_hashtable_value_t) NULL;
            placement->allocation_ref = replacement_instr_ref;
            res = kefir_hashtable_delete(mem, &debug_info->allocations, (kefir_hashtable_key_t) instr_ref);
            REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &debug_info->allocations, (kefir_hashtable_key_t) replacement_instr_ref, (kefir_hashtable_value_t) placement));
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_hashset_free(mem, &placement->placement);
                KEFIR_FREE(mem, placement);
                return res;
            });
        } else {
            REQUIRE_OK(kefir_hashtable_delete(mem, &debug_info->allocations, (kefir_hashtable_key_t) instr_ref));
        }
    }

    REQUIRE_OK(kefir_hashset_delete(&debug_info->active_refs, (kefir_hashset_key_t) instr_ref));
    if (replacement_instr_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_hashset_add(mem, &debug_info->active_refs, (kefir_hashset_key_t) replacement_instr_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_iter(
    const struct kefir_opt_code_debug_info *debug_info, struct kefir_opt_code_debug_info_local_variable_iterator *iter,
    const struct kefir_opt_code_debug_info_local_variable **local_variable_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information local variable iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_iter(&debug_info->local_variables, &iter->iter, NULL, &table_value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of local variable debug information iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_PTR(local_variable_ptr, (const struct kefir_opt_code_debug_info_local_variable *) table_value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_next(
    struct kefir_opt_code_debug_info_local_variable_iterator *iter, const struct kefir_opt_code_debug_info_local_variable **local_variable_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information local variable iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_next(&iter->iter, NULL, &table_value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of local variable debug information iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_PTR(local_variable_ptr, (const struct kefir_opt_code_debug_info_local_variable *) table_value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_allocation_placement_iter(
    const struct kefir_opt_code_debug_info *debug_info, struct kefir_opt_code_debug_info_allocation_placement_iterator *iter,
    const struct kefir_opt_code_debug_info_allocation_placement **placement_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information allocation placement iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_iter(&debug_info->allocations, &iter->iter, NULL, &table_value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of allocation placement debug information iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_PTR(placement_ptr, (const struct kefir_opt_code_debug_info_allocation_placement *) table_value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_allocation_placement_next(
    struct kefir_opt_code_debug_info_allocation_placement_iterator *iter, const struct kefir_opt_code_debug_info_allocation_placement **placement_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information allocation placement iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_next(&iter->iter, NULL, &table_value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of allocation placement debug information iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_PTR(placement_ptr, (const struct kefir_opt_code_debug_info_allocation_placement *) table_value);
    return KEFIR_OK;
}
