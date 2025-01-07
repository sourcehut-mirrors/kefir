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

#ifndef KEFIR_AST_SCOPE_H_
#define KEFIR_AST_SCOPE_H_

#include "kefir/core/mem.h"
#include "kefir/ast/type.h"
#include "kefir/ast/constants.h"
#include "kefir/ast/alignment.h"
#include "kefir/ast/constant_expression.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/tree.h"
#include "kefir/ast/initializer.h"
#include "kefir/core/util.h"
#include "kefir/ast/type_layout.h"
#include "kefir/ast/flow_control.h"

#define KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE (sizeof(kefir_uptr_t) * 6)

typedef struct kefir_ast_scoped_identifier kefir_ast_scoped_identifier_t;
typedef struct kefir_ast_identifier_block_scope kefir_ast_identifier_block_scope_t;

typedef struct kefir_ast_scoped_identifier_cleanup {
    kefir_result_t (*callback)(struct kefir_mem *, struct kefir_ast_scoped_identifier *, void *);
    void *payload;
} kefir_ast_scoped_identifier_cleanup_t;

typedef struct kefir_ast_scoped_identifier {
    kefir_ast_scoped_identifier_class_t klass;
    struct kefir_ast_scoped_identifier_cleanup cleanup;
    struct kefir_ast_identifier_flat_scope *definition_scope;
    union {
        struct {
            const struct kefir_ast_type *type;
            struct kefir_ast_alignment *alignment;
            kefir_ast_scoped_identifier_storage_t storage;
            kefir_ast_scoped_identifier_linkage_t linkage;
            kefir_bool_t external;
            struct kefir_ast_initializer *UNOWNED(initializer);
            kefir_id_t vl_array;
            kefir_ast_declarator_visibility_attr_t visibility;
            const char *asm_label;
            const char *alias;
            const char *defining_function;
            struct {
                kefir_bool_t weak;
            } flags;
        } object;

        struct {
            const struct kefir_ast_type *type;
            kefir_ast_function_specifier_t specifier;
            kefir_ast_scoped_identifier_storage_t storage;
            kefir_bool_t external;
            kefir_bool_t defined;
            kefir_bool_t inline_definition;
            kefir_ast_declarator_visibility_attr_t visibility;
            const char *alias;
            struct {
                kefir_bool_t weak;
                kefir_bool_t gnu_inline;
                kefir_bool_t constructor;
                kefir_bool_t destructor;
            } flags;
            struct kefir_ast_local_context **local_context_ptr;
            struct kefir_ast_local_context *local_context;
            const char *asm_label;
        } function;

        struct {
            const struct kefir_ast_type *type;
            struct kefir_ast_constant_expression *value;
        } enum_constant;

        struct {
            const struct kefir_ast_type *type;
            struct kefir_ast_alignment *alignment;
        } type_definition;

        const struct kefir_ast_type *type;

        struct {
            struct kefir_ast_flow_control_point *point;
            const char *public_label;
        } label;
    };
    struct kefir_source_location source_location;
    struct {
        unsigned char content[KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE];
        void *ptr;
        struct kefir_ast_scoped_identifier_cleanup *cleanup;
    } payload;
} kefir_ast_scoped_identifier_t;

typedef struct kefir_ast_identifier_flat_scope_iterator {
    struct kefir_hashtree_node_iterator iter;

    const char *identifier;
    struct kefir_ast_scoped_identifier *value;
} kefir_ast_identifier_flat_scope_iterator_t;

typedef struct kefir_ast_identifier_flat_scope {
    struct kefir_hashtree content;

    kefir_result_t (*remove_callback)(struct kefir_mem *, struct kefir_ast_scoped_identifier *, void *);
    void *remove_payload;
} kefir_ast_identifier_flat_scope_t;

#define KEFIR_AST_SCOPE_SET_CLEANUP(_scope, _callback, _payload) \
    do {                                                         \
        (_scope)->payload.cleanup->callback = (_callback);       \
        (_scope)->payload.cleanup->payload = (_payload);         \
    } while (0)

kefir_result_t kefir_ast_scoped_identifier_run_cleanup(struct kefir_mem *, struct kefir_ast_scoped_identifier *);

kefir_result_t kefir_ast_identifier_flat_scope_init(struct kefir_ast_identifier_flat_scope *);
kefir_result_t kefir_ast_identifier_flat_scope_free(struct kefir_mem *, struct kefir_ast_identifier_flat_scope *);
kefir_result_t kefir_ast_identifier_flat_scope_cleanup_payload(struct kefir_mem *,
                                                               const struct kefir_ast_identifier_flat_scope *);
kefir_result_t kefir_ast_identifier_flat_scope_on_removal(
    struct kefir_ast_identifier_flat_scope *,
    kefir_result_t (*)(struct kefir_mem *, struct kefir_ast_scoped_identifier *, void *), void *);
kefir_result_t kefir_ast_identifier_flat_scope_insert(struct kefir_mem *, struct kefir_ast_identifier_flat_scope *,
                                                      const char *, struct kefir_ast_scoped_identifier *);
kefir_result_t kefir_ast_identifier_flat_scope_at(const struct kefir_ast_identifier_flat_scope *, const char *,
                                                  struct kefir_ast_scoped_identifier **);
kefir_bool_t kefir_ast_identifier_flat_scope_has(const struct kefir_ast_identifier_flat_scope *, const char *);
kefir_bool_t kefir_ast_identifier_flat_scope_empty(const struct kefir_ast_identifier_flat_scope *);
kefir_result_t kefir_ast_identifier_flat_scope_iter(const struct kefir_ast_identifier_flat_scope *,
                                                    struct kefir_ast_identifier_flat_scope_iterator *);
kefir_result_t kefir_ast_identifier_flat_scope_next(const struct kefir_ast_identifier_flat_scope *,
                                                    struct kefir_ast_identifier_flat_scope_iterator *);

typedef struct kefir_ast_identifier_block_scope {
    struct kefir_tree_node root;
    struct kefir_tree_node external_root;
    struct kefir_list scopes;

    kefir_result_t (*remove_callback)(struct kefir_mem *, struct kefir_ast_scoped_identifier *, void *);
    void *remove_payload;
} kefir_ast_identifier_block_scope_t;

kefir_result_t kefir_ast_identifier_block_scope_init(struct kefir_mem *, struct kefir_ast_identifier_block_scope *);
kefir_result_t kefir_ast_identifier_block_scope_free(struct kefir_mem *, struct kefir_ast_identifier_block_scope *);
kefir_result_t kefir_ast_identifier_block_scope_cleanup_payload(struct kefir_mem *,
                                                                const struct kefir_ast_identifier_block_scope *);
kefir_result_t kefir_ast_identifier_block_scope_on_removal(
    struct kefir_ast_identifier_block_scope *,
    kefir_result_t (*)(struct kefir_mem *, struct kefir_ast_scoped_identifier *, void *), void *);
struct kefir_ast_identifier_flat_scope *kefir_ast_identifier_block_scope_top(
    const struct kefir_ast_identifier_block_scope *);
kefir_result_t kefir_ast_identifier_block_scope_push(struct kefir_mem *, struct kefir_ast_identifier_block_scope *);
kefir_result_t kefir_ast_identifier_block_scope_pop(struct kefir_ast_identifier_block_scope *);

kefir_result_t kefir_ast_identifier_block_scope_push_external(struct kefir_mem *,
                                                              struct kefir_ast_identifier_block_scope *,
                                                              struct kefir_ast_identifier_flat_scope *);
kefir_result_t kefir_ast_identifier_block_scope_pop_external(struct kefir_mem *,
                                                             struct kefir_ast_identifier_block_scope *);

kefir_result_t kefir_ast_identifier_block_scope_insert(struct kefir_mem *,
                                                       const struct kefir_ast_identifier_block_scope *, const char *,
                                                       struct kefir_ast_scoped_identifier *);
kefir_result_t kefir_ast_identifier_block_scope_at(const struct kefir_ast_identifier_block_scope *, const char *,
                                                   struct kefir_ast_scoped_identifier **);
kefir_bool_t kefir_ast_identifier_block_scope_empty(const struct kefir_ast_identifier_block_scope *);

#endif
