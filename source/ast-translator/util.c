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

#include "kefir/ast-translator/util.h"
#include "kefir/ast-translator/scope/scoped_identifier.h"
#include "kefir/ast-translator/scope/scope_layout_impl.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

const struct kefir_ast_type *kefir_ast_translator_normalize_type(const struct kefir_ast_type *original) {
    REQUIRE(original != NULL, NULL);

    return kefir_ast_type_conv_unwrap_enumeration(kefir_ast_unqualified_type(original));
}

kefir_result_t kefir_ast_translator_scoped_identifier_allocate_object(struct kefir_memory_arena *arena, const struct kefir_ast_scoped_identifier *scoped_identifier, struct kefir_ast_translator_scoped_identifier_object **object_ptr) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(scoped_identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST scoped identifier"));

    if (scoped_identifier->payload == NULL) {
        struct kefir_ast_translator_scoped_identifier_object *object = kefir_memory_arena_alloc(arena, sizeof(struct kefir_ast_translator_scoped_identifier_object), _Alignof(struct kefir_ast_translator_scoped_identifier_object));
        REQUIRE(object != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST translator scoped identifier object"));
        memset(object, 0, sizeof(struct kefir_ast_translator_scoped_identifier_object));
        ((struct kefir_ast_scoped_identifier *) scoped_identifier)->payload = object;
        KEFIR_AST_SCOPE_SET_CLEANUP((struct kefir_ast_scoped_identifier *) scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);
    }

    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload);
    ASSIGN_PTR(object_ptr, scoped_identifier_layout);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_scoped_identifier_allocate_function(struct kefir_memory_arena *arena, const struct kefir_ast_scoped_identifier *scoped_identifier, struct kefir_ast_translator_scoped_identifier_function **function_ptr) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(scoped_identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST scoped identifier"));

    if (scoped_identifier->payload == NULL) {
        struct kefir_ast_translator_scoped_identifier_function *function = kefir_memory_arena_alloc(arena, sizeof(struct kefir_ast_translator_scoped_identifier_function), _Alignof(struct kefir_ast_translator_scoped_identifier_function));
        REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST translator scoped identifier function"));
        memset(function, 0, sizeof(struct kefir_ast_translator_scoped_identifier_function));
        ((struct kefir_ast_scoped_identifier *) scoped_identifier)->payload = function;
        KEFIR_AST_SCOPE_SET_CLEANUP((struct kefir_ast_scoped_identifier *) scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);
    }

    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_function *, scoped_identifier_layout,
                     scoped_identifier->payload);
    ASSIGN_PTR(function_ptr, scoped_identifier_layout);
    return KEFIR_OK;
}
