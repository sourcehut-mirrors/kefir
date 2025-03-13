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

#ifndef CONSTANT_EXPRESSION_H_
#define CONSTANT_EXPRESSION_H_

#include <string.h>

#define ASSERT_INTEGER_CONST_EXPR(_mem, _context, _node, _value)                                             \
    do {                                                                                                     \
        struct kefir_ast_node_base *base = KEFIR_AST_NODE_BASE((_node));                                     \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), base));                                         \
        ASSERT(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(base, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER)); \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->integer == (_value));                         \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), base));                                                        \
    } while (0)

#define ASSERT_UINTEGER_CONST_EXPR(_mem, _context, _node, _value)                                            \
    do {                                                                                                     \
        struct kefir_ast_node_base *base = KEFIR_AST_NODE_BASE((_node));                                     \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), base));                                         \
        ASSERT(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(base, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER)); \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->uinteger == (_value));                        \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), base));                                                        \
    } while (0)

#define ASSERT_FLOAT_CONST_EXPR(_mem, _context, _node, _value)                                                  \
    do {                                                                                                        \
        struct kefir_ast_node_base *base = KEFIR_AST_NODE_BASE((_node));                                        \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), base));                                            \
        ASSERT(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(base, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT));      \
        ASSERT(DOUBLE_EQUALS((double) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->floating_point, (_value), \
                             DOUBLE_EPSILON));                                                                  \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), base));                                                           \
    } while (0)

#define ASSERT_IDENTIFIER_CONST_EXPR(_mem, _context, _node, _value, _offset)                                 \
    do {                                                                                                     \
        struct kefir_ast_node_base *base = KEFIR_AST_NODE_BASE((_node));                                     \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), base));                                         \
        ASSERT(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(base, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)); \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.type ==                               \
               KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER);                                             \
        ASSERT(strcmp(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.base.literal, (_value)) == 0); \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.offset == (_offset));                 \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), base));                                                        \
    } while (0)

#define ASSERT_LITERAL_CONST_EXPR(_mem, _context, _node, _value)                                                      \
    do {                                                                                                              \
        struct kefir_ast_node_base *base = KEFIR_AST_NODE_BASE((_node));                                              \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), base));                                                  \
        ASSERT(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(base, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS));          \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS); \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.type ==                                        \
               KEFIR_AST_CONSTANT_EXPRESSION_POINTER_LITERAL);                                                        \
        ASSERT(strcmp(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.base.string.content, (_value)) == 0);   \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.offset == 0);                                  \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), base));                                                                 \
    } while (0)

#define ASSERT_INTPTR_CONST_EXPR(_mem, _context, _node, _value, _offset)                                              \
    do {                                                                                                              \
        struct kefir_ast_node_base *base = KEFIR_AST_NODE_BASE((_node));                                              \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), base));                                                  \
        ASSERT(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(base, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS));          \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS); \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.type ==                                        \
               KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER);                                                        \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.base.integral == (_value));                    \
        ASSERT(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(base)->pointer.offset == (_offset));                          \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), base));                                                                 \
    } while (0)

#define ASSERT_CONST_EXPR_NOK(_mem, _context, _node)                     \
    do {                                                                 \
        struct kefir_ast_node_base *base = KEFIR_AST_NODE_BASE((_node)); \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), base));     \
        ASSERT(!KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(base));            \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), base));                    \
    } while (0)

#endif
