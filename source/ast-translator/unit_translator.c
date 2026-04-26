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

#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/function_definition.h"
#include "kefir/ast/downcast.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t allocate_function_context(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                struct kefir_ast_node_base *external_definition,
                                                struct kefir_ast_translator_function_context *function_context) {
    REQUIRE(external_definition->properties.category == KEFIR_AST_NODE_CATEGORY_FUNCTION_DEFINITION,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected function definition AST node"));

    kefir_result_t res;
    struct kefir_ast_function_definition *function_definition = NULL;
    REQUIRE_MATCH_OK(&res, kefir_ast_downcast_function_definition(external_definition, &function_definition, false),
                     KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected function definition AST node"));

    REQUIRE_OK(kefir_ast_translator_function_context_init(mem, context, function_definition, function_context));
    return KEFIR_OK;
}

static kefir_result_t translate_unit_impl(struct kefir_mem *mem, struct kefir_ast_translation_unit *unit,
                                          struct kefir_ast_translator_context *context, kefir_bool_t consume) {
    for (kefir_size_t i = 0; i < unit->external_definitions_length; i++) {
        struct kefir_ast_node_base *external_definition = unit->external_definitions[i];

        switch (external_definition->properties.category) {
            case KEFIR_AST_NODE_CATEGORY_DECLARATION:
            case KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR:
                // Intentionally left blank
                break;

            case KEFIR_AST_NODE_CATEGORY_FUNCTION_DEFINITION: {
                struct kefir_ast_translator_function_context func_ctx;
                REQUIRE_OK(allocate_function_context(mem, context, external_definition, &func_ctx));

                kefir_result_t res = kefir_ast_translator_function_context_translate(mem, &func_ctx);
                REQUIRE_CHAIN(&res, kefir_ast_translator_function_context_finalize(mem, &func_ctx));
                REQUIRE_ELSE(res == KEFIR_OK, {
                    kefir_ast_translator_function_context_free(mem, &func_ctx);
                    return res;
                });
                REQUIRE_OK(kefir_ast_translator_function_context_free(mem, &func_ctx));

                if (consume) {
                    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, external_definition));
                    unit->external_definitions[i] = NULL;
                }
            } break;

            case KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY:
                REQUIRE_OK(kefir_ast_translate_inline_assembly(mem, external_definition, NULL, context));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected external definition node category");
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_unit(struct kefir_mem *mem, const struct kefir_ast_node_base *node,
                                        struct kefir_ast_translator_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL && node->properties.category == KEFIR_AST_NODE_CATEGORY_TRANSLATION_UNIT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation unit"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));

    kefir_result_t res;
    struct kefir_ast_translation_unit *unit = NULL;
    REQUIRE_MATCH_OK(&res, kefir_ast_downcast_translation_unit(node, &unit, false),
                     KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));

    REQUIRE_OK(translate_unit_impl(mem, unit, context, false));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_unit_consume(struct kefir_mem *mem, struct kefir_ast_node_base *node,
                                                struct kefir_ast_translator_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL && node->properties.category == KEFIR_AST_NODE_CATEGORY_TRANSLATION_UNIT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation unit"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));

    kefir_result_t res;
    struct kefir_ast_translation_unit *unit = NULL;
    REQUIRE_MATCH_OK(&res, kefir_ast_downcast_translation_unit(node, &unit, false),
                     KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));

    REQUIRE_OK(translate_unit_impl(mem, unit, context, true));
    return KEFIR_OK;
}
