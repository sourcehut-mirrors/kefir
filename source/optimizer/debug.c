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

static kefir_result_t free_local_variable_refs(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                               kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_local_variable_refset *, refs, value);
    if (refs != NULL) {
        REQUIRE_OK(kefir_hashtreeset_free(mem, &refs->refs));
        memset(refs, 0, sizeof(struct kefir_opt_code_debug_info_local_variable_refset));
        KEFIR_FREE(mem, refs);
    }
    return KEFIR_OK;
}

static kefir_result_t on_new_instruction(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                         kefir_opt_instruction_ref_t instr_ref, void *payload) {
    UNUSED(code);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info *, debug_info, payload);
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    if (debug_info->instruction_location_cursor != KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &debug_info->instruction_locations, (kefir_hashtree_key_t) instr_ref,
                                         (kefir_hashtree_value_t) debug_info->instruction_location_cursor));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_init(struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(debug_info != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer debug information"));

    debug_info->instruction_location_cursor = KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE;
    debug_info->listener.on_new_instruction = on_new_instruction;
    debug_info->listener.payload = debug_info;
    REQUIRE_OK(kefir_hashtree_init(&debug_info->instruction_locations, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&debug_info->local_variable_allocs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&debug_info->local_variable_allocs, free_local_variable_refs, NULL));
    REQUIRE_OK(kefir_hashtree_init(&debug_info->local_variable_refs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&debug_info->local_variable_refs, free_local_variable_refs, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_free(struct kefir_mem *mem, struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    REQUIRE_OK(kefir_hashtree_free(mem, &debug_info->local_variable_refs));
    REQUIRE_OK(kefir_hashtree_free(mem, &debug_info->local_variable_allocs));
    REQUIRE_OK(kefir_hashtree_free(mem, &debug_info->instruction_locations));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_set_instruction_location_cursor(struct kefir_opt_code_debug_info *debug_info,
                                                                         kefir_size_t cursor) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    debug_info->instruction_location_cursor = cursor;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_set_instruction_location_cursor_of(
    struct kefir_opt_code_debug_info *debug_info, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&debug_info->instruction_locations, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        debug_info->instruction_location_cursor = (kefir_size_t) node->value;
    } else {
        debug_info->instruction_location_cursor = KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_instruction_location(const struct kefir_opt_code_debug_info *debug_info,
                                                              kefir_opt_instruction_ref_t instr_ref,
                                                              kefir_size_t *location_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    REQUIRE(location_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to instruction location"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&debug_info->instruction_locations, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *location_ptr = (kefir_size_t) node->value;
    } else {
        *location_ptr = KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_register_local_variable_allocation(
    struct kefir_mem *mem, struct kefir_opt_code_debug_info *debug_info, kefir_opt_instruction_ref_t alloc_instr_ref,
    kefir_uint64_t local_variable_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    ASSIGN_DECL_CAST(const kefir_hashtree_key_t, key, local_variable_id);

    struct kefir_opt_code_debug_info_local_variable_refset *refs = NULL;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&debug_info->local_variable_allocs, key, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        refs = (struct kefir_opt_code_debug_info_local_variable_refset *) node->value;
    } else {
        refs = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_debug_info_local_variable_refset));
        REQUIRE(refs != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer local variable references"));

        res = kefir_hashtreeset_init(&refs->refs, &kefir_hashtree_uint_ops);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, refs);
            return res;
        });

        res = kefir_hashtree_insert(mem, &debug_info->local_variable_allocs, key, (kefir_hashtree_value_t) refs);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtreeset_free(mem, &refs->refs);
            KEFIR_FREE(mem, refs);
            return res;
        });
    }

    REQUIRE_OK(kefir_hashtreeset_add(mem, &refs->refs, (kefir_hashtreeset_entry_t) alloc_instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_replace_local_variable(struct kefir_mem *mem,
                                                                struct kefir_opt_code_debug_info *debug_info,
                                                                kefir_opt_instruction_ref_t instr_ref,
                                                                kefir_opt_instruction_ref_t replacement_instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&debug_info->local_variable_allocs, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_local_variable_refset *, refs, node->value);
        if (kefir_hashtreeset_has(&refs->refs, (kefir_hashtreeset_entry_t) instr_ref)) {
            REQUIRE_OK(kefir_hashtreeset_delete(mem, &refs->refs, (kefir_hashtreeset_entry_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(mem, &refs->refs, (kefir_hashtreeset_entry_t) replacement_instr_ref));
        }
    }

    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&debug_info->local_variable_refs, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_local_variable_refset *, refs, node->value);
        if (kefir_hashtreeset_has(&refs->refs, (kefir_hashtreeset_entry_t) instr_ref)) {
            REQUIRE_OK(kefir_hashtreeset_delete(mem, &refs->refs, (kefir_hashtreeset_entry_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(mem, &refs->refs, (kefir_hashtreeset_entry_t) replacement_instr_ref));
        }
    }

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&debug_info->local_variable_refs, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_local_variable_refset *, refs, node->value);

        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&refs->refs, &iter); res == KEFIR_OK; res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, ref, iter.entry);
            REQUIRE_OK(kefir_opt_code_debug_info_add_local_variable_ref(mem, debug_info, replacement_instr_ref, ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_hashtree_delete(mem, &debug_info->local_variable_refs, (kefir_hashtree_key_t) instr_ref));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_allocation_of(
    const struct kefir_opt_code_debug_info *debug_info, kefir_uint64_t variable_id,
    const struct kefir_opt_code_debug_info_local_variable_refset **alloc_instr_refs) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(alloc_instr_refs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction reference set"));

    ASSIGN_DECL_CAST(const kefir_hashtree_key_t, key, variable_id);
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&debug_info->local_variable_allocs, key, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find local variable allocation");
    }
    REQUIRE_OK(res);

    *alloc_instr_refs = (const struct kefir_opt_code_debug_info_local_variable_refset *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_add_local_variable_ref(struct kefir_mem *mem,
                                                                struct kefir_opt_code_debug_info *debug_info,
                                                                kefir_opt_instruction_ref_t alloc_instr_ref,
                                                                kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(alloc_instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));

    struct kefir_opt_code_debug_info_local_variable_refset *refs = NULL;
    struct kefir_hashtree_node *node;
    kefir_result_t res =
        kefir_hashtree_at(&debug_info->local_variable_refs, (kefir_hashtree_key_t) alloc_instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        refs = (struct kefir_opt_code_debug_info_local_variable_refset *) node->value;
    } else {
        refs = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_debug_info_local_variable_refset));
        REQUIRE(refs != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer local variable references"));

        res = kefir_hashtreeset_init(&refs->refs, &kefir_hashtree_uint_ops);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, refs);
            return res;
        });

        res = kefir_hashtree_insert(mem, &debug_info->local_variable_refs, (kefir_hashtree_key_t) alloc_instr_ref,
                                    (kefir_hashtree_value_t) refs);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtreeset_free(mem, &refs->refs);
            KEFIR_FREE(mem, refs);
            return res;
        });
    }

    REQUIRE_OK(kefir_hashtreeset_add(mem, &refs->refs, (kefir_hashtreeset_entry_t) instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_has_refs(const struct kefir_opt_code_debug_info *debug_info,
                                                                 kefir_opt_instruction_ref_t instr_ref,
                                                                 kefir_bool_t *has_refs_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(has_refs_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *has_refs_ptr = kefir_hashtree_has(&debug_info->local_variable_refs, (kefir_hashtree_key_t) instr_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_allocation_iter(
    const struct kefir_opt_code_debug_info *debug_info, struct kefir_opt_code_debug_info_local_variable_iterator *iter,
    kefir_uint64_t *variable_id_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information local variable iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_iter(&debug_info->local_variable_allocs, &iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(variable_id_ptr, (kefir_uint64_t) node->key);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_allocation_next(
    struct kefir_opt_code_debug_info_local_variable_iterator *iter, kefir_uint64_t *variable_id_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information local variable iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(variable_id_ptr, (kefir_uint64_t) node->key);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_iter(
    const struct kefir_opt_code_debug_info *debug_info, struct kefir_opt_code_debug_info_local_variable_iterator *iter,
    kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information local variable iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_iter(&debug_info->local_variable_refs, &iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) node->key);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_next(
    struct kefir_opt_code_debug_info_local_variable_iterator *iter, kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information local variable iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) node->key);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_ref_iter(
    const struct kefir_opt_code_debug_info *debug_info,
    struct kefir_opt_code_debug_info_local_variable_ref_iterator *iter, kefir_opt_instruction_ref_t alloc_instr_ref,
    kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer debug information local variable ref iterator"));

    struct kefir_hashtree_node *node;
    kefir_result_t res =
        kefir_hashtree_at(&debug_info->local_variable_refs, (kefir_hashtree_key_t) alloc_instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find request instruction reference debug information");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_opt_code_debug_info_local_variable_refset *, refs, node->value);

    REQUIRE_OK(kefir_hashtreeset_iter(&refs->refs, &iter->iter));
    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) iter->iter.entry);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_local_variable_ref_next(
    struct kefir_opt_code_debug_info_local_variable_ref_iterator *iter, kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid optimizer debug information local variable ref iterator"));

    REQUIRE_OK(kefir_hashtreeset_next(&iter->iter));
    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) iter->iter.entry);
    return KEFIR_OK;
}
