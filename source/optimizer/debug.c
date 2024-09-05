/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

static kefir_hashtree_hash_t source_location_hash(kefir_hashtree_key_t key, void *data) {
    UNUSED(data);
    ASSIGN_DECL_CAST(const struct kefir_source_location *, source_location, key);
    REQUIRE(source_location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));
    return kefir_hashtree_str_ops.hash((kefir_hashtree_key_t) source_location->source, kefir_hashtree_str_ops.data) *
               61 +
           source_location->line * 37 + source_location->column;
}

static kefir_int_t source_location_compare(kefir_hashtree_key_t key1, kefir_hashtree_key_t key2, void *data) {
    UNUSED(data);
    ASSIGN_DECL_CAST(const struct kefir_source_location *, source_location1, key1);
    ASSIGN_DECL_CAST(const struct kefir_source_location *, source_location2, key2);
    REQUIRE(source_location1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));
    REQUIRE(source_location2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));

    int source_cmp = strcmp(source_location1->source, source_location2->source);
    if (source_cmp < 0) {
        return -1;
    } else if (source_cmp > 0) {
        return 1;
    } else if (source_location1->line < source_location2->line) {
        return -1;
    } else if (source_location1->line > source_location2->line) {
        return 1;
    } else if (source_location1->column < source_location2->column) {
        return -1;
    } else if (source_location1->column > source_location2->column) {
        return 1;
    } else {
        return 0;
    }
}

const struct kefir_hashtree_ops kefir_hashtree_source_location_ops = {
    .hash = source_location_hash, .compare = source_location_compare, .data = NULL};

static kefir_result_t free_source_location(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                           kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(value);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_source_location *, source_location, key);
    REQUIRE(source_location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));

    KEFIR_FREE(mem, (char *) source_location->source);
    memset(source_location, 0, sizeof(struct kefir_source_location));
    KEFIR_FREE(mem, source_location);
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

    if (debug_info->source_location_cursor != NULL) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &debug_info->instruction_locations, (kefir_hashtree_key_t) instr_ref,
                                         (kefir_hashtree_value_t) debug_info->source_location_cursor));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_init(struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(debug_info != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer debug information"));

    debug_info->source_location_cursor = NULL;
    debug_info->listener.on_new_instruction = on_new_instruction;
    debug_info->listener.payload = debug_info;
    REQUIRE_OK(kefir_hashtree_init(&debug_info->source_locations, &kefir_hashtree_source_location_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&debug_info->source_locations, free_source_location, NULL));
    REQUIRE_OK(kefir_hashtree_init(&debug_info->instruction_locations, &kefir_hashtree_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_free(struct kefir_mem *mem, struct kefir_opt_code_debug_info *debug_info) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    REQUIRE_OK(kefir_hashtree_free(mem, &debug_info->instruction_locations));
    REQUIRE_OK(kefir_hashtree_free(mem, &debug_info->source_locations));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_set_source_location_cursor(
    struct kefir_mem *mem, struct kefir_opt_code_debug_info *debug_info,
    const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    if (source_location != NULL) {
        struct kefir_hashtree_node *node;
        kefir_result_t res =
            kefir_hashtree_at(&debug_info->source_locations, (kefir_hashtree_key_t) source_location, &node);
        if (res == KEFIR_NOT_FOUND) {
            struct kefir_source_location *location = KEFIR_MALLOC(mem, sizeof(struct kefir_source_location));
            REQUIRE(location != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate source location"));

            const kefir_size_t source_id_length = strlen(source_location->source);
            char *source_id = KEFIR_MALLOC(mem, sizeof(char) * (source_id_length + 1));
            REQUIRE_ELSE(source_id != NULL, {
                KEFIR_FREE(mem, location);
                return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate source location identifier");
            });

            strcpy(source_id, source_location->source);
            location->source = source_id;
            location->line = source_location->line;
            location->column = source_location->column;

            kefir_result_t res = kefir_hashtree_insert(mem, &debug_info->source_locations,
                                                       (kefir_hashtree_key_t) location, (kefir_hashtree_value_t) 0);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, (char *) location->source);
                KEFIR_FREE(mem, location);
                return res;
            });

            debug_info->source_location_cursor = location;
        } else {
            REQUIRE_OK(res);
            debug_info->source_location_cursor = (const struct kefir_source_location *) node->key;
        }
    } else {
        debug_info->source_location_cursor = NULL;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_set_source_location_cursor_of(struct kefir_opt_code_debug_info *debug_info,
                                                                       kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&debug_info->instruction_locations, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        debug_info->source_location_cursor = (const struct kefir_source_location *) node->value;
    } else {
        debug_info->source_location_cursor = NULL;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_debug_info_source_location_of(const struct kefir_opt_code_debug_info *debug_info,
                                                            kefir_opt_instruction_ref_t instr_ref,
                                                            const struct kefir_source_location **source_location_ptr) {
    REQUIRE(debug_info != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer debug information"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction reference"));
    REQUIRE(source_location_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to source location"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&debug_info->instruction_locations, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *source_location_ptr = (const struct kefir_source_location *) node->value;
    } else {
        *source_location_ptr = NULL;
    }
    return KEFIR_OK;
}
