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

#ifndef KEFIR_AST_NODE_BASE_H_
#define KEFIR_AST_NODE_BASE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/core/list.h"
#include "kefir/ast/base.h"
#include "kefir/ast/constants.h"
#include "kefir/ast/type.h"
#include "kefir/ast/temporaries.h"
#include "kefir/ast/constant_expression.h"
#include "kefir/core/source_location.h"

typedef struct kefir_ast_visitor kefir_ast_visitor_t;
typedef struct kefir_ast_visitor kefir_ast_visitor_t;

typedef struct kefir_ast_node_class {
    kefir_ast_node_type_t type;
    kefir_result_t (*visit)(const struct kefir_ast_node_base *, const struct kefir_ast_visitor *, void *);
    kefir_result_t (*free)(struct kefir_mem *, struct kefir_ast_node_base *);
} kefir_ast_node_class_t;

typedef struct kefir_ast_node_expression_properties {
    kefir_uint32_t lvalue : 1,
                   addressable : 1,
                   atomic : 1,
                   preserve_after_eval : 1,
                   constant_expression_evaluated : 1,
                   constant_expression : 1;
    kefir_uint32_t alignment;
    struct kefir_ast_constant_expression_value *constant_expression_value;
    struct kefir_ast_bitfield_properties bitfield_props;
    const char *identifier;
    const struct kefir_ast_string_literal_data *string_literal;
    const struct kefir_ast_scoped_identifier *scoped_id;
    struct kefir_ast_temporary_identifier *temporary_identifier;
    union {
        struct kefir_ast_flow_control_structure *flow_control_statement;
        struct kefir_ast_flow_control_point *flow_control_point;
    };
    struct kefir_ast_temporary_identifier *preserve_after_eval_temporary_identifier;
} kefir_ast_node_expression_properties_t;

typedef struct kefir_ast_node_declaration_properties {
    kefir_ast_scoped_identifier_storage_t storage;
    kefir_ast_function_specifier_t function;
    kefir_bool_t static_assertion;
    kefir_uint32_t alignment;
    const char *identifier;
    const struct kefir_ast_type *original_type;
    const struct kefir_ast_scoped_identifier *scoped_id;
    struct kefir_ast_temporary_identifier temporary_identifier;
} kefir_ast_node_declaration_properties_t;

typedef struct kefir_ast_node_statement_properties {
    struct kefir_ast_flow_control_point *origin_flow_control_point;
    struct kefir_ast_flow_control_point *target_flow_control_point;
    struct kefir_ast_flow_control_structure *flow_control_statement;
    const struct kefir_ast_scoped_identifier *scoped_id;
    const struct kefir_ast_type *return_type;
} kefir_ast_node_statement_properties_t;

typedef struct kefir_ast_node_function_definition_properties {
    kefir_ast_function_specifier_t function;
    const char *identifier;
    const struct kefir_ast_scoped_identifier *scoped_id;

    struct {
        kefir_bool_t enable_fenv_access;
        kefir_bool_t disallow_fp_contract;
        kefir_ast_pragma_on_off_value_t cx_limited_range;
    } pragma_stats;
} kefir_ast_node_function_definition_properties_t;

typedef struct kefir_ast_node_type_properties {
    kefir_uint32_t alignment;
    kefir_ast_scoped_identifier_storage_t storage;
} kefir_ast_node_type_properties_t;

typedef struct kefir_ast_node_inline_assembly_properties {
    struct kefir_ast_flow_control_point *origin_flow_control_point;
    struct kefir_ast_flow_control_branching_point *branching_point;
} kefir_ast_node_inline_assembly_properties_t;

typedef struct kefir_ast_node_properties {
    kefir_ast_node_category_t category;
    const struct kefir_ast_type *type;
    union {
        struct kefir_ast_node_expression_properties *expression_props;
        struct kefir_ast_node_declaration_properties *declaration_props;
        struct kefir_ast_node_statement_properties *statement_props;
        struct kefir_ast_node_function_definition_properties *function_definition;
        struct kefir_ast_node_type_properties *type_props;
        struct kefir_ast_node_inline_assembly_properties *inline_assembly;
    };
} kefir_ast_node_properties_t;

typedef struct kefir_ast_node_base {
    kefir_uint64_t refcount;
    const struct kefir_ast_node_class *klass;
    struct kefir_ast_node_properties properties;
    struct kefir_source_location source_location;
} kefir_ast_node_base_t;

#define KEFIR_AST_NODE_STRUCT(id, content) \
    typedef struct id {                    \
        struct kefir_ast_node_base base;   \
        struct content;                    \
    } id##_t; \
    _Static_assert(offsetof(struct id, base) == 0, "Unexpected offset of AST node base")

#define KEFIR_AST_NODE_BASE(node) (&(node)->base)
#define KEFIR_AST_NODE_SELF(node) ((void *) (node))
#define KEFIR_AST_NODE_VISIT(visitor, base, payload) ((base)->klass->visit((base), (visitor), (payload)))
#define KEFIR_AST_NODE_REF(base) (kefir_ast_node_ref((base)))
#define KEFIR_AST_NODE_FREE(mem, base) (kefir_ast_node_free((mem), (base)))

#define KEFIR_AST_VISITOR_METHOD(id, type) \
    kefir_result_t (*id)(const struct kefir_ast_visitor *, const struct type *, void *)

struct kefir_ast_node_base *kefir_ast_node_ref(struct kefir_ast_node_base *);
kefir_result_t kefir_ast_node_free(struct kefir_mem *, struct kefir_ast_node_base *);

kefir_result_t kefir_ast_visitor_init(struct kefir_ast_visitor *,
                                      KEFIR_AST_VISITOR_METHOD(method, kefir_ast_node_base));

kefir_result_t kefir_ast_node_properties_init(struct kefir_ast_node_properties *);
kefir_result_t kefir_ast_node_properties_reset(struct kefir_ast_node_properties *, kefir_ast_node_category_t);

kefir_result_t kefir_ast_node_properties_clone(struct kefir_ast_node_properties *,
                                               const struct kefir_ast_node_properties *);

#define KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(_node)                       \
    ((_node)->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION && \
     (_node)->properties.expression_props->constant_expression)
#define KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(_node, _klass) \
    (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION((_node)) &&          \
     (_node)->properties.expression_props->constant_expression_value->klass == (_klass))
#define KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(_node)                                                               \
    (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION((_node)) ? (_node)->properties.expression_props->constant_expression_value \
                                                    : NULL)

kefir_result_t kefir_ast_node_allocate_expression_props(struct kefir_memory_arena *, struct kefir_ast_node_base *);
kefir_result_t kefir_ast_node_allocate_declaration_props(struct kefir_memory_arena *, struct kefir_ast_node_base *);
kefir_result_t kefir_ast_node_allocate_statement_props(struct kefir_memory_arena *, struct kefir_ast_node_base *);
kefir_result_t kefir_ast_node_allocate_function_definition_props(struct kefir_memory_arena *, struct kefir_ast_node_base *);
kefir_result_t kefir_ast_node_allocate_type_props(struct kefir_memory_arena *, struct kefir_ast_node_base *);
kefir_result_t kefir_ast_node_allocate_inline_asm_props(struct kefir_memory_arena *, struct kefir_ast_node_base *);
kefir_result_t kefir_ast_node_allocate_temporary_identifier(struct kefir_memory_arena *, struct kefir_ast_temporary_identifier **);

#endif
