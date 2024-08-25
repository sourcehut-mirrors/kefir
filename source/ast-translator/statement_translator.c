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

#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/translator_impl.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/extensions.h"

static kefir_result_t translate_not_impl(const struct kefir_ast_visitor *visitor,
                                         const struct kefir_ast_node_base *base, void *payload) {
    UNUSED(visitor);
    UNUSED(base);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Cannot translate non-statement AST node");
}

#define TRANSLATE_NODE(_id, _type)                                                                                     \
    static kefir_result_t translate_##_id(const struct kefir_ast_visitor *visitor, const _type *node, void *payload) { \
        REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));                 \
        REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));                       \
        REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));                  \
        ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);                                    \
        REQUIRE_OK(kefir_ast_translate_##_id##_node(param->mem, param->context, param->builder, node));                \
        return KEFIR_OK;                                                                                               \
    }

TRANSLATE_NODE(expression_statement, struct kefir_ast_expression_statement)
TRANSLATE_NODE(compound_statement, struct kefir_ast_compound_statement)
TRANSLATE_NODE(conditional_statement, struct kefir_ast_conditional_statement)
TRANSLATE_NODE(switch_statement, struct kefir_ast_switch_statement)
TRANSLATE_NODE(while_statement, struct kefir_ast_while_statement)
TRANSLATE_NODE(do_while_statement, struct kefir_ast_do_while_statement)
TRANSLATE_NODE(for_statement, struct kefir_ast_for_statement)
TRANSLATE_NODE(labeled_statement, struct kefir_ast_labeled_statement)
TRANSLATE_NODE(case_statement, struct kefir_ast_case_statement)
TRANSLATE_NODE(goto_statement, struct kefir_ast_goto_statement)
TRANSLATE_NODE(continue_statement, struct kefir_ast_continue_statement)
TRANSLATE_NODE(break_statement, struct kefir_ast_break_statement)
TRANSLATE_NODE(return_statement, struct kefir_ast_return_statement)
#undef TRANSLATE_NODE

static kefir_result_t translate_extension_node(const struct kefir_ast_visitor *visitor,
                                               const struct kefir_ast_extension_node *node, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);

    REQUIRE(param->context->extensions != NULL && param->context->extensions->translate_extension_node != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Extension node translation procedure is not defined"));
    REQUIRE_OK(param->context->extensions->translate_extension_node(
        param->mem, param->context, node, param->builder, KEFIR_AST_TRANSLATOR_CONTEXT_EXTENSION_TAG_STATEMENT));
    return KEFIR_OK;
}

static kefir_result_t translate_inline_asm(const struct kefir_ast_visitor *visitor,
                                           const struct kefir_ast_inline_assembly *node, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);
    REQUIRE_OK(
        kefir_ast_translate_inline_assembly(param->mem, KEFIR_AST_NODE_BASE(node), param->builder, param->context));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_statement(struct kefir_mem *mem, const struct kefir_ast_node_base *base,
                                             struct kefir_irbuilder_block *builder,
                                             struct kefir_ast_translator_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));

    struct kefir_ast_visitor visitor;
    REQUIRE_OK(kefir_ast_visitor_init(&visitor, translate_not_impl));
    visitor.expression_statement = translate_expression_statement;
    visitor.compound_statement = translate_compound_statement;
    visitor.conditional_statement = translate_conditional_statement;
    visitor.switch_statement = translate_switch_statement;
    visitor.while_statement = translate_while_statement;
    visitor.do_while_statement = translate_do_while_statement;
    visitor.for_statement = translate_for_statement;
    visitor.labeled_statement = translate_labeled_statement;
    visitor.case_statement = translate_case_statement;
    visitor.goto_statement = translate_goto_statement;
    visitor.continue_statement = translate_continue_statement;
    visitor.break_statement = translate_break_statement;
    visitor.return_statement = translate_return_statement;
    visitor.extension_node = translate_extension_node;
    visitor.goto_address_statement = translate_goto_statement;
    visitor.inline_assembly = translate_inline_asm;

    kefir_result_t res;
    KEFIR_RUN_EXTENSION(&res, mem, context, before_translate, base, builder,
                        KEFIR_AST_TRANSLATOR_CONTEXT_EXTENSION_TAG_STATEMENT, &visitor);
    REQUIRE_OK(res);
    struct kefir_ast_translator_parameters param = {.mem = mem, .builder = builder, .context = context};
    const kefir_size_t begin_ir_index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    REQUIRE_OK(KEFIR_AST_NODE_VISIT(&visitor, base, &param));
    KEFIR_RUN_EXTENSION(&res, mem, context, after_translate, base, builder,
                        KEFIR_AST_TRANSLATOR_CONTEXT_EXTENSION_TAG_STATEMENT);
    REQUIRE_OK(res);

    if (context->function_debug_info != NULL && KEFIR_SOURCE_LOCATION_IS_VALID(&base->source_location)) {
        const kefir_size_t end_ir_index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(kefir_ir_source_map_insert(mem, &context->function_debug_info->source_map,
                                              context->ast_context->symbols, &base->source_location, begin_ir_index,
                                              end_ir_index));
    }
    return KEFIR_OK;
}
