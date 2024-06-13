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

#ifndef KEFIR_AST_CONTEXT_IMPL_H_
#define KEFIR_AST_CONTEXT_IMPL_H_

#include "kefir/core/mem.h"
#include "kefir/ast/scope.h"

kefir_result_t kefir_ast_context_free_scoped_identifier(struct kefir_mem *, struct kefir_ast_scoped_identifier *,
                                                        void *);

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_object_identifier(
    struct kefir_mem *, const struct kefir_ast_type *, struct kefir_ast_identifier_flat_scope *,
    kefir_ast_scoped_identifier_storage_t, struct kefir_ast_alignment *, kefir_ast_scoped_identifier_linkage_t,
    kefir_bool_t, struct kefir_ast_initializer *, const char *, const struct kefir_source_location *);

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_constant(struct kefir_mem *,
                                                                               struct kefir_ast_constant_expression *,
                                                                               const struct kefir_ast_type *,
                                                                               const struct kefir_source_location *);

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_type_tag(struct kefir_mem *,
                                                                               const struct kefir_ast_type *,
                                                                               const struct kefir_source_location *);

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_type_definition(
    struct kefir_mem *, const struct kefir_ast_type *, struct kefir_ast_alignment *,
    const struct kefir_source_location *);

kefir_result_t kefir_ast_context_allocate_scoped_type_definition_update_alignment(struct kefir_mem *,
                                                                                  struct kefir_ast_scoped_identifier *,
                                                                                  struct kefir_ast_alignment *);

kefir_result_t kefir_ast_context_type_retrieve_tag(const struct kefir_ast_type *, const char **);

kefir_result_t kefir_ast_context_update_existing_scoped_type_tag(struct kefir_ast_scoped_identifier *,
                                                                 const struct kefir_ast_type *);

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_function_identifier(
    struct kefir_mem *, const struct kefir_ast_type *, kefir_ast_function_specifier_t,
    kefir_ast_scoped_identifier_storage_t, kefir_bool_t, kefir_bool_t, kefir_bool_t, const char *, const char *,
    const struct kefir_source_location *);

kefir_ast_function_specifier_t kefir_ast_context_merge_function_specifiers(kefir_ast_function_specifier_t,
                                                                           kefir_ast_function_specifier_t);

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_label(struct kefir_mem *,
                                                                            struct kefir_ast_flow_control_structure *,
                                                                            const struct kefir_source_location *);

kefir_result_t kefir_ast_context_merge_alignment(struct kefir_mem *, struct kefir_ast_alignment **,
                                                 struct kefir_ast_alignment *);

#define KEFIR_AST_CONTEXT_MERGE_FUNCTION_ASM_LABEL(_ordinary_id, _attributes)                           \
    do {                                                                                                \
        if ((_ordinary_id)->function.asm_label == NULL) {                                               \
            (_ordinary_id)->function.asm_label = (_attributes)->asm_label;                              \
        } else {                                                                                        \
            REQUIRE((_attributes)->asm_label == NULL ||                                                 \
                        strcmp((_attributes)->asm_label, (_ordinary_id)->function.asm_label) == 0,      \
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,                              \
                                           "Assembly label does not match with previous declaration")); \
        }                                                                                               \
    } while (0)

#define KEFIR_AST_CONTEXT_MERGE_FUNCTION_ALIAS_ATTR(_ordinary_id, _attributes)                           \
    do {                                                                                                 \
        if ((_attributes)->alias != NULL) {                                                              \
            REQUIRE((_attributes)->asm_label == NULL && (_ordinary_id)->function.asm_label == NULL,      \
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,                               \
                                           "Assembly label cannot be attached to an aliased function")); \
            if ((_ordinary_id)->function.alias != NULL) {                                                \
                REQUIRE(strcmp((_attributes)->alias, (_ordinary_id)->function.alias) == 0,               \
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,                           \
                                               "Alias mismatch in function redeclaration"));             \
            } else {                                                                                     \
                (_ordinary_id)->function.alias = (_attributes)->alias;                                   \
            }                                                                                            \
        }                                                                                                \
    } while (0)

#define KEFIR_AST_CONTEXT_MERGE_BOOL(_left, _right) \
    do {                                            \
        *(_left) = *(_left) || (_right);            \
    } while (0)

#define KEFIR_AST_CONTEXT_FUNCTION_GET_ATTR(_attributes, _name, _default) \
    ((_attributes) != NULL ? (_attributes)->_name : (_default))

#define KEFIR_AST_CONTEXT_FUNCTION_IDENTIFIER_INSERT(_mem, _context, _identifier, _ordinary_id)                        \
    do {                                                                                                               \
        const char *id = kefir_string_pool_insert((_mem), &(_context)->symbols, (_identifier), NULL);                  \
        REQUIRE(id != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table")); \
        res = kefir_ast_identifier_flat_scope_insert((_mem), &(_context)->function_identifiers, id, (_ordinary_id));   \
        REQUIRE_ELSE(res == KEFIR_OK, {                                                                                \
            kefir_ast_context_free_scoped_identifier((_mem), ordinary_id, NULL);                                       \
            return res;                                                                                                \
        });                                                                                                            \
    } while (0)

#endif
