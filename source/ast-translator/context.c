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

#include "kefir/ast-translator/context.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/extensions.h"

kefir_result_t kefir_ast_translator_context_init(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                 const struct kefir_ast_context *ast_context,
                                                 const struct kefir_ast_translator_environment *environment,
                                                 struct kefir_ir_module *module,
                                                 const struct kefir_ast_translator_context_extensions *extensions) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected a pointer to valid AST translator context"));
    REQUIRE(ast_context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(environment != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator environment"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));

    context->base_context = NULL;
    context->ast_context = ast_context;
    context->environment = environment;
    context->module = module;
    context->global_scope_layout = NULL;
    context->local_scope_layout = NULL;
    context->function_debug_info = NULL;
    context->debug_entry_hierarchy = KEFIR_IR_DEBUG_ENTRY_ID_NONE;

    context->debug_entries = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_translator_debug_entries));
    REQUIRE(context->debug_entries != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST translator debug type bundle"));
    kefir_result_t res = kefir_ast_translator_debug_entries_init(context->debug_entries);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, context->debug_entries);
        return res;
    });

    context->extensions = extensions;
    context->extensions_payload = NULL;
    KEFIR_RUN_EXTENSION0(&res, mem, context, on_init);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_translator_debug_entries_free(mem, context->debug_entries);
        KEFIR_FREE(mem, context->debug_entries);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_context_init_local(struct kefir_mem *mem,
                                                       struct kefir_ast_translator_context *context,
                                                       const struct kefir_ast_context *ast_context,
                                                       struct kefir_ir_function_debug_info *function_debug_info,
                                                       struct kefir_ast_translator_context *base_context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected a pointer to valid AST translator context"));
    REQUIRE(ast_context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(base_context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid base AST translator context"));

    context->base_context = base_context;
    context->ast_context = ast_context;
    context->environment = base_context->environment;
    context->module = base_context->module;
    context->global_scope_layout = NULL;
    context->local_scope_layout = NULL;
    context->function_debug_info = function_debug_info;
    context->debug_entries = base_context->debug_entries;
    context->debug_entry_hierarchy = KEFIR_IR_DEBUG_ENTRY_ID_NONE;

    context->extensions = base_context->extensions;
    context->extensions_payload = NULL;
    kefir_result_t res;
    KEFIR_RUN_EXTENSION0(&res, mem, context, on_init);
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_context_free(struct kefir_mem *mem, struct kefir_ast_translator_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected a pointer to valid AST translator context"));

    kefir_result_t res;
    KEFIR_RUN_EXTENSION0(&res, mem, context, on_free);
    REQUIRE_OK(res);

    if (context->base_context == NULL) {
        REQUIRE_OK(kefir_ast_translator_debug_entries_free(mem, context->debug_entries));
        KEFIR_FREE(mem, context->debug_entries);
    }

    context->base_context = NULL;
    context->ast_context = NULL;
    context->environment = NULL;
    context->module = NULL;
    context->global_scope_layout = NULL;
    context->local_scope_layout = NULL;
    context->debug_entries = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_context_push_debug_hierarchy_entry(struct kefir_mem *mem,
                                                                       struct kefir_ast_translator_context *context,
                                                                       kefir_ir_debug_entry_tag_t tag,
                                                                       kefir_ir_debug_entry_id_t *entry_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected a valid AST translator context"));

    kefir_ir_debug_entry_id_t entry_id;
    if (context->debug_entry_hierarchy != KEFIR_IR_DEBUG_ENTRY_ID_NONE) {
        REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, &context->module->debug_info.entries,
                                                  context->debug_entry_hierarchy, tag, &entry_id));
    } else {
        REQUIRE_OK(kefir_ir_debug_entry_new(mem, &context->module->debug_info.entries, tag, &entry_id));
    }
    context->debug_entry_hierarchy = entry_id;
    ASSIGN_PTR(entry_id_ptr, entry_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_context_pop_debug_hierarchy_entry(struct kefir_mem *mem,
                                                                      struct kefir_ast_translator_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected a valid AST translator context"));
    REQUIRE(context->debug_entry_hierarchy != KEFIR_IR_DEBUG_ENTRY_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "AST translation context debug entry hierarchy is empty"));

    const struct kefir_ir_debug_entry *entry;
    REQUIRE_OK(kefir_ir_debug_entry_get(&context->module->debug_info.entries, context->debug_entry_hierarchy, &entry));
    context->debug_entry_hierarchy = entry->parent_identifier;

    return KEFIR_OK;
}
