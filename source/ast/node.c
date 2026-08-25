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

#include "kefir/ast/node.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

struct kefir_ast_node_base *kefir_ast_node_ref(struct kefir_ast_node_base *node) {
    REQUIRE(node != NULL, NULL);
    node->refcount++;
    return node;
}

kefir_result_t kefir_ast_node_free(struct kefir_mem *mem, struct kefir_ast_node_base *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));
    REQUIRE(node->refcount > 0, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected reference count of AST node"));

    if (--node->refcount == 0) {
        REQUIRE_OK(node->klass->free(mem, node));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_visitor_init(struct kefir_ast_visitor *visitor,
                                      kefir_result_t (*generic)(const struct kefir_ast_visitor *,
                                                                const struct kefir_ast_node_base *, void *)) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    *visitor = (const struct kefir_ast_visitor) {0};
    visitor->generic_handler = generic;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_properties_init(struct kefir_ast_node_properties *props) {
    REQUIRE(props != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node properties pointer"));
    *props = (struct kefir_ast_node_properties) {0};
    props->category = KEFIR_AST_NODE_CATEGORY_UNKNOWN;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_properties_reset(struct kefir_ast_node_properties *props, kefir_ast_node_category_t category) {
    REQUIRE(props != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node properties pointer"));
    if (props->category != category) {
        REQUIRE_OK(kefir_ast_node_properties_init(props));
        return KEFIR_OK;
    }

    switch (props->category) {
        case KEFIR_AST_NODE_CATEGORY_EXPRESSION: {
            struct kefir_ast_node_expression_properties *expr_props = props->expression_props;
            if (expr_props != NULL) {
                struct kefir_ast_constant_expression_value *const_expr_value = expr_props->constant_expression_value;
                if (const_expr_value != NULL) {
                    memset(const_expr_value, 0, sizeof(struct kefir_ast_constant_expression_value));
                }
                *expr_props = (struct kefir_ast_node_expression_properties) {
                    .constant_expression_value = const_expr_value
                };
            }
            *props = (struct kefir_ast_node_properties) {
                .expression_props = expr_props
            };
        } break;

        case KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR:
        case KEFIR_AST_NODE_CATEGORY_DECLARATION: {
            struct kefir_ast_node_declaration_properties *decl_props = props->declaration_props;
            if (decl_props != NULL) {
                *decl_props = (struct kefir_ast_node_declaration_properties) {0};
            }
            *props = (struct kefir_ast_node_properties) {
                .declaration_props = decl_props
            };
        } break;

        case KEFIR_AST_NODE_CATEGORY_STATEMENT: {
            struct kefir_ast_node_statement_properties *stmt_props = props->statement_props;
            if (stmt_props != NULL) {
                *stmt_props = (struct kefir_ast_node_statement_properties) {0};
            }
            *props = (struct kefir_ast_node_properties) {
                .statement_props = stmt_props
            };
        } break;

        case KEFIR_AST_NODE_CATEGORY_FUNCTION_DEFINITION: {
            struct kefir_ast_node_function_definition_properties *fndef_props = props->function_definition;
            if (fndef_props != NULL) {
                *fndef_props = (struct kefir_ast_node_function_definition_properties) {0};
            }
            *props = (struct kefir_ast_node_properties) {
                .function_definition = fndef_props
            };
        } break;

        case KEFIR_AST_NODE_CATEGORY_TYPE: {
            struct kefir_ast_node_type_properties *type_props = props->type_props;
            if (type_props != NULL) {
                *type_props = (struct kefir_ast_node_type_properties) {0};
            }
            *props = (struct kefir_ast_node_properties) {
                .type_props = type_props
            };
        } break;

        case KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY: {
            struct kefir_ast_node_inline_assembly_properties *inline_asm_props = props->inline_assembly;
            if (inline_asm_props != NULL) {
                *inline_asm_props = (struct kefir_ast_node_inline_assembly_properties) {0};
            }
            *props = (struct kefir_ast_node_properties) {
                .inline_assembly = inline_asm_props
            };
        } break;

        case KEFIR_AST_NODE_CATEGORY_UNKNOWN:
        case KEFIR_AST_NODE_CATEGORY_MEMBER_DESIGNATOR:
        case KEFIR_AST_NODE_CATEGORY_TRANSLATION_UNIT:
            *props = (struct kefir_ast_node_properties) {0};
            break;
    }
    props->category = KEFIR_AST_NODE_CATEGORY_UNKNOWN;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_properties_clone(struct kefir_ast_node_properties *dst_props,
                                               const struct kefir_ast_node_properties *src_props) {
    REQUIRE(dst_props != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node properties destination pointer"));
    REQUIRE(src_props != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node properties source pointer"));
    memcpy(dst_props, src_props, sizeof(struct kefir_ast_node_properties));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_allocate_expression_props(struct kefir_memory_arena *arena, struct kefir_ast_node_base *node) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->properties.expression_props == NULL) {
        node->properties.expression_props = kefir_memory_arena_alloc(arena, sizeof(struct kefir_ast_node_expression_properties), _Alignof(struct kefir_ast_node_expression_properties));
        REQUIRE(node->properties.expression_props != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node expression properties"));
        memset(node->properties.expression_props, 0, sizeof(struct kefir_ast_node_expression_properties));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_allocate_declaration_props(struct kefir_memory_arena *arena, struct kefir_ast_node_base *node) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->properties.declaration_props == NULL) {
        node->properties.declaration_props = kefir_memory_arena_alloc(arena, sizeof(struct kefir_ast_node_declaration_properties), _Alignof(struct kefir_ast_node_declaration_properties));
        REQUIRE(node->properties.declaration_props != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node declaration properties"));
        memset(node->properties.declaration_props, 0, sizeof(struct kefir_ast_node_declaration_properties));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_allocate_statement_props(struct kefir_memory_arena *arena, struct kefir_ast_node_base *node) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->properties.statement_props == NULL) {
        node->properties.statement_props = kefir_memory_arena_alloc(arena, sizeof(struct kefir_ast_node_statement_properties), _Alignof(struct kefir_ast_node_statement_properties));
        REQUIRE(node->properties.statement_props != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node statement properties"));
        memset(node->properties.statement_props, 0, sizeof(struct kefir_ast_node_statement_properties));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_allocate_function_definition_props(struct kefir_memory_arena *arena, struct kefir_ast_node_base *node) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->properties.function_definition == NULL) {
        node->properties.function_definition = kefir_memory_arena_alloc(arena, sizeof(struct kefir_ast_node_function_definition_properties), _Alignof(struct kefir_ast_node_function_definition_properties));
        REQUIRE(node->properties.function_definition != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node function definition properties"));
        memset(node->properties.function_definition, 0, sizeof(struct kefir_ast_node_function_definition_properties));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_allocate_type_props(struct kefir_memory_arena *arena, struct kefir_ast_node_base *node) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->properties.type_props == NULL) {
        node->properties.type_props = kefir_memory_arena_alloc(arena, sizeof(struct kefir_ast_node_type_properties), _Alignof(struct kefir_ast_node_type_properties));
        REQUIRE(node->properties.type_props != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node type properties"));
        memset(node->properties.type_props, 0, sizeof(struct kefir_ast_node_type_properties));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_allocate_inline_asm_props(struct kefir_memory_arena *arena, struct kefir_ast_node_base *node) {
    REQUIRE(arena != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory arena"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->properties.inline_assembly == NULL) {
        node->properties.inline_assembly = kefir_memory_arena_alloc(arena, sizeof(struct kefir_ast_node_inline_assembly_properties), _Alignof(struct kefir_ast_node_inline_assembly_properties));
        REQUIRE(node->properties.inline_assembly != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node inline assembly properties"));
        memset(node->properties.inline_assembly, 0, sizeof(struct kefir_ast_node_inline_assembly_properties));
    }
    return KEFIR_OK;
}
