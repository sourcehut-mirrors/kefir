/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#ifndef KEFIR_AST_NODE_BASE_H_
#define KEFIR_AST_NODE_BASE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/core/list.h"
#include "kefir/ast/base.h"
#include "kefir/ast/constants.h"
#include "kefir/ast/type.h"
#include "kefir/ast/temporaries.h"
#include "kefir/core/source_location.h"

typedef struct kefir_ast_visitor kefir_ast_visitor_t;
typedef struct kefir_ast_visitor kefir_ast_visitor_t;

typedef struct kefir_ast_node_class {
    kefir_ast_node_type_t type;
    kefir_result_t (*visit)(const struct kefir_ast_node_base *, const struct kefir_ast_visitor *, void *);
    struct kefir_ast_node_base *(*clone)(struct kefir_mem *, struct kefir_ast_node_base *);
    kefir_result_t (*free)(struct kefir_mem *, struct kefir_ast_node_base *);
} kefir_ast_node_class_t;

typedef struct kefir_ast_node_properties {
    kefir_ast_node_category_t category;
    const struct kefir_ast_type *type;
    struct {
        kefir_bool_t lvalue;
        kefir_bool_t constant_expression;
        kefir_bool_t addressable;
        struct kefir_ast_bitfield_properties bitfield_props;
        const char *identifier;
        struct {
            kefir_ast_string_literal_type_t type;
            void *content;
            kefir_size_t length;
        } string_literal;
        struct kefir_ast_temporary_identifier temporary;
        const struct kefir_ast_scoped_identifier *scoped_id;
        struct kefir_ast_flow_control_structure *flow_control_statement;
    } expression_props;

    struct {
        kefir_ast_scoped_identifier_storage_t storage;
        kefir_ast_function_specifier_t function;
        const char *identifier;
        kefir_size_t alignment;
        kefir_bool_t static_assertion;
        const struct kefir_ast_type *original_type;
        const struct kefir_ast_scoped_identifier *scoped_id;
    } declaration_props;

    struct {
        kefir_bool_t constant;
    } member_designator;

    struct {
        struct kefir_ast_flow_control_point *origin_flow_control_point;
        struct kefir_ast_flow_control_point *target_flow_control_point;
        struct kefir_ast_flow_control_structure *flow_control_statement;
        const struct kefir_ast_type *return_type;
    } statement_props;

    struct {
        kefir_ast_scoped_identifier_storage_t storage;
        kefir_ast_function_specifier_t function;
        const char *identifier;
        const struct kefir_ast_scoped_identifier *scoped_id;
    } function_definition;
} kefir_ast_node_properties_t;

typedef struct kefir_ast_node_base {
    const struct kefir_ast_node_class *klass;
    void *self;
    struct kefir_ast_node_properties properties;
    struct kefir_source_location source_location;
} kefir_ast_node_base_t;

#define KEFIR_AST_NODE_STRUCT(id, content) \
    typedef struct id {                    \
        struct kefir_ast_node_base base;   \
        struct content;                    \
    } id##_t

#define KEFIR_AST_NODE_BASE(node) (&(node)->base)
#define KEFIR_AST_NODE_VISIT(visitor, base, payload) ((base)->klass->visit((base), (visitor), (payload)))
#define KEFIR_AST_NODE_CLONE(mem, base) ((base) != NULL ? (base)->klass->clone((mem), (base)) : NULL)
#define KEFIR_AST_NODE_FREE(mem, base) ((base)->klass->free((mem), (base)))

#define KEFIR_AST_VISITOR_METHOD(id, type) \
    kefir_result_t (*id)(const struct kefir_ast_visitor *, const struct type *, void *)

kefir_result_t kefir_ast_visitor_init(struct kefir_ast_visitor *,
                                      KEFIR_AST_VISITOR_METHOD(method, kefir_ast_node_base));

kefir_result_t kefir_ast_node_properties_init(struct kefir_ast_node_properties *);

kefir_result_t kefir_ast_node_properties_clone(struct kefir_ast_node_properties *,
                                               const struct kefir_ast_node_properties *);

#endif
