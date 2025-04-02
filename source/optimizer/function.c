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

#include "kefir/optimizer/function.h"
#include "kefir/optimizer/module.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct block_inline_entry {
    kefir_size_t num_of_source_functions;
    struct kefir_hashtreeset source_functions;
};

static kefir_result_t free_block_inline_entry(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                              kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);

    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct block_inline_entry *, entry, value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer block inline entry"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &entry->source_functions));
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_function_init(const struct kefir_opt_module *module, const struct kefir_ir_function *ir_func,
                                       struct kefir_opt_function *func) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(ir_func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer function"));

    func->ir_func = ir_func;
    func->num_of_inlines = 0;
    func->debug_info_mapping.ir_code_length = kefir_irblock_length(&ir_func->body);
    REQUIRE_OK(kefir_opt_code_container_init(&func->code));
    REQUIRE_OK(kefir_opt_code_debug_info_init(&func->debug_info));
    REQUIRE_OK(kefir_hashtree_init(&func->inlines, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&func->inlines, free_block_inline_entry, NULL));
    func->code.event_listener = &func->debug_info.listener;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_function_free(struct kefir_mem *mem, struct kefir_opt_function *func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer function"));

    REQUIRE_OK(kefir_hashtree_free(mem, &func->inlines));
    REQUIRE_OK(kefir_opt_code_debug_info_free(mem, &func->debug_info));
    REQUIRE_OK(kefir_opt_code_container_free(mem, &func->code));
    func->ir_func = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_function_block_inlined_from(struct kefir_mem *mem, struct kefir_opt_function *function,
                                                     kefir_opt_block_id_t block_id,
                                                     const struct kefir_opt_function *source_function,
                                                     kefir_opt_block_id_t source_block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(source_function != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source optimizer function"));

    struct block_inline_entry *entry = NULL;

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&function->inlines, (kefir_hashtree_key_t) block_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        entry = KEFIR_MALLOC(mem, sizeof(struct block_inline_entry));
        REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate block inline entry"));
        entry->num_of_source_functions = 0;
        res = kefir_hashtreeset_init(&entry->source_functions, &kefir_hashtree_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &function->inlines, (kefir_hashtree_key_t) block_id,
                                                  (kefir_hashtree_value_t) entry));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, entry);
            return res;
        });
    } else {
        REQUIRE_OK(res);
        entry = (struct block_inline_entry *) node->value;
    }

    if (function != source_function) {
        REQUIRE_OK(kefir_hashtreeset_add(mem, &entry->source_functions,
                                         (kefir_hashtreeset_entry_t) source_function->ir_func->declaration->id));
        entry->num_of_source_functions++;
    }

    res = kefir_hashtree_at(&source_function->inlines, (kefir_hashtree_key_t) source_block_id, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(const struct block_inline_entry *, source_entry, node->value);
        REQUIRE_OK(kefir_hashtreeset_merge(mem, &entry->source_functions, &source_entry->source_functions, NULL, NULL));
        entry->num_of_source_functions += source_entry->num_of_source_functions;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_function_block_can_inline(const struct kefir_opt_function *function,
                                                   kefir_opt_block_id_t block_id,
                                                   const struct kefir_opt_function *inlined_function,
                                                   kefir_size_t max_inline_depth, kefir_bool_t *can_inline_ptr) {
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(inlined_function != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid inlined optimizer function"));
    REQUIRE(can_inline_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&function->inlines, (kefir_hashtree_key_t) block_id, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(const struct block_inline_entry *, entry, node->value);
        *can_inline_ptr =
            !kefir_hashtreeset_has(&entry->source_functions,
                                   (kefir_hashtreeset_entry_t) inlined_function->ir_func->declaration->id) &&
            (entry->num_of_source_functions < max_inline_depth);
    } else {
        *can_inline_ptr = true;
    }

    return KEFIR_OK;
}
