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

#ifndef KEFIR_IR_MODULE_H_
#define KEFIR_IR_MODULE_H_

#include "kefir/core/mem.h"
#include "kefir/core/list.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/string_pool.h"
#include "kefir/ir/function.h"
#include "kefir/ir/data.h"
#include "kefir/ir/assembly.h"
#include "kefir/ir/debug.h"

typedef enum kefir_ir_identifier_type {
    KEFIR_IR_IDENTIFIER_FUNCTION,
    KEFIR_IR_IDENTIFIER_GLOBAL_DATA,
    KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA
} kefir_ir_identifier_type_t;

typedef enum kefir_ir_identifier_scope {
    KEFIR_IR_IDENTIFIER_SCOPE_EXPORT,
    KEFIR_IR_IDENTIFIER_SCOPE_EXPORT_WEAK,
    KEFIR_IR_IDENTIFIER_SCOPE_IMPORT,
    KEFIR_IR_IDENTIFIER_SCOPE_LOCAL
} kefir_ir_identifier_scope_t;

typedef enum kefir_ir_identifier_visibility {
    KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
    KEFIR_IR_IDENTIFIER_VISIBILITY_HIDDEN,
    KEFIR_IR_IDENTIFIER_VISIBILITY_INTERNAL,
    KEFIR_IR_IDENTIFIER_VISIBILITY_PROTECTED
} kefir_ir_identifier_visibility_t;

typedef struct kefir_ir_identifier {
    const char *symbol;
    kefir_ir_identifier_type_t type;
    kefir_ir_identifier_scope_t scope;
    kefir_ir_identifier_visibility_t visibility;
    const char *alias;
    struct {
        kefir_ir_debug_entry_id_t entry;
    } debug_info;
} kefir_ir_identifier_t;

typedef struct kefir_ir_module_string_literal {
    kefir_ir_string_literal_type_t type;
    kefir_bool_t public;
    void *content;
    kefir_size_t length;
} kefir_ir_module_string_literal_;

typedef struct kefir_ir_module {
    struct kefir_string_pool symbols;
    struct kefir_list types;
    struct kefir_hashtree function_declarations;
    struct kefir_hashtree identifiers;
    struct kefir_hashtree functions;
    struct kefir_hashtree named_types;
    struct kefir_hashtree named_data;
    struct kefir_hashtree string_literals;
    struct kefir_hashtree inline_assembly;
    struct kefir_hashtree global_inline_asm;

    kefir_id_t next_type_id;
    kefir_id_t next_string_literal_id;
    kefir_id_t next_function_decl_id;
    kefir_id_t next_inline_assembly_id;

    struct kefir_ir_module_debug_info debug_info;
} kefir_ir_module_t;

kefir_result_t kefir_ir_module_alloc(struct kefir_mem *, struct kefir_ir_module *);

kefir_result_t kefir_ir_module_free(struct kefir_mem *, struct kefir_ir_module *);

const char *kefir_ir_module_symbol(struct kefir_mem *, struct kefir_ir_module *, const char *, kefir_id_t *);

kefir_result_t kefir_ir_module_string_literal(struct kefir_mem *, struct kefir_ir_module *,
                                              kefir_ir_string_literal_type_t, kefir_bool_t, const void *, kefir_size_t,
                                              kefir_id_t *);

struct kefir_ir_type *kefir_ir_module_new_type(struct kefir_mem *, struct kefir_ir_module *, kefir_size_t,
                                               kefir_id_t *);

struct kefir_ir_function_decl *kefir_ir_module_new_function_declaration(struct kefir_mem *, struct kefir_ir_module *,
                                                                        const char *, kefir_id_t, bool, kefir_id_t);

kefir_result_t kefir_ir_module_declare_identifier(struct kefir_mem *, struct kefir_ir_module *, const char *,
                                                  const struct kefir_ir_identifier *);

struct kefir_ir_function *kefir_ir_module_new_function(struct kefir_mem *, struct kefir_ir_module *,
                                                       struct kefir_ir_function_decl *, kefir_id_t, kefir_size_t);

struct kefir_ir_data *kefir_ir_module_new_named_data(struct kefir_mem *, struct kefir_ir_module *, const char *,
                                                     kefir_ir_data_storage_t, kefir_id_t);

struct kefir_ir_data *kefir_ir_module_get_named_data(struct kefir_ir_module *, const char *);

const char *kefir_ir_module_get_named_symbol(const struct kefir_ir_module *, kefir_id_t);

const struct kefir_ir_function_decl *kefir_ir_module_get_declaration(const struct kefir_ir_module *, kefir_id_t);

struct kefir_ir_type *kefir_ir_module_get_named_type(const struct kefir_ir_module *, kefir_id_t);

const struct kefir_ir_function_decl *kefir_ir_module_function_declaration_iter(const struct kefir_ir_module *,
                                                                               struct kefir_hashtree_node_iterator *);
const struct kefir_ir_function_decl *kefir_ir_module_function_declaration_next(struct kefir_hashtree_node_iterator *);

const struct kefir_ir_function *kefir_ir_module_function_iter(const struct kefir_ir_module *,
                                                              struct kefir_hashtree_node_iterator *);
const struct kefir_ir_function *kefir_ir_module_function_next(struct kefir_hashtree_node_iterator *);

const char *kefir_ir_module_identifiers_iter(const struct kefir_ir_module *, struct kefir_hashtree_node_iterator *,
                                             const struct kefir_ir_identifier **);
const char *kefir_ir_module_identifiers_next(struct kefir_hashtree_node_iterator *,
                                             const struct kefir_ir_identifier **);
kefir_result_t kefir_ir_module_get_identifier(const struct kefir_ir_module *, const char *,
                                              const struct kefir_ir_identifier **);

kefir_result_t kefir_ir_module_get_string_literal(const struct kefir_ir_module *, kefir_id_t,
                                                  kefir_ir_string_literal_type_t *, kefir_bool_t *, const void **,
                                                  kefir_size_t *);

kefir_result_t kefir_ir_module_string_literal_iter(const struct kefir_ir_module *,
                                                   struct kefir_hashtree_node_iterator *, kefir_id_t *,
                                                   kefir_ir_string_literal_type_t *, kefir_bool_t *, const void **,
                                                   kefir_size_t *);

kefir_result_t kefir_ir_module_string_literal_next(struct kefir_hashtree_node_iterator *, kefir_id_t *,
                                                   kefir_ir_string_literal_type_t *, kefir_bool_t *, const void **,
                                                   kefir_size_t *);

const struct kefir_ir_data *kefir_ir_module_named_data_iter(const struct kefir_ir_module *,
                                                            struct kefir_hashtree_node_iterator *, const char **);
const struct kefir_ir_data *kefir_ir_module_named_data_next(struct kefir_hashtree_node_iterator *, const char **);

const struct kefir_ir_type *kefir_ir_module_named_type_iter(const struct kefir_ir_module *,
                                                            struct kefir_hashtree_node_iterator *, kefir_id_t *);
const struct kefir_ir_type *kefir_ir_module_named_type_next(struct kefir_hashtree_node_iterator *, kefir_id_t *);

struct kefir_ir_inline_assembly *kefir_ir_module_new_inline_assembly(struct kefir_mem *, struct kefir_ir_module *,
                                                                     const char *, kefir_id_t *);
const struct kefir_ir_inline_assembly *kefir_ir_module_get_inline_assembly(const struct kefir_ir_module *, kefir_id_t);
const struct kefir_ir_inline_assembly *kefir_ir_module_inline_assembly_iter(const struct kefir_ir_module *,
                                                                            struct kefir_hashtree_node_iterator *,
                                                                            kefir_id_t *);
const struct kefir_ir_inline_assembly *kefir_ir_module_inline_assembly_next(struct kefir_hashtree_node_iterator *,
                                                                            kefir_id_t *);

kefir_result_t kefir_ir_module_inline_assembly_global(struct kefir_mem *, struct kefir_ir_module *, kefir_id_t);

#endif
