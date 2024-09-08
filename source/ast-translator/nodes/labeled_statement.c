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

#include "kefir/ast-translator/translator_impl.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/flow_control.h"
#include "kefir/ast-translator/util.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_translate_labeled_statement_node(struct kefir_mem *mem,
                                                          struct kefir_ast_translator_context *context,
                                                          struct kefir_irbuilder_block *builder,
                                                          const struct kefir_ast_labeled_statement *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST labeled statement node"));

    if (node->base.properties.statement_props.scoped_id->label.public_label != NULL) {
        REQUIRE_OK(kefir_irblock_add_public_label(mem, builder->block, context->ast_context->symbols,
                                                  node->base.properties.statement_props.scoped_id->label.public_label,
                                                  KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder)));
    }
    REQUIRE_OK(kefir_ast_translator_flow_control_point_resolve(
        mem, node->base.properties.statement_props.target_flow_control_point,
        KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder)));
    if (context->debug_entries != NULL) {
        kefir_ir_debug_entry_id_t label_entry_id;
        REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, &context->module->debug_info.entries,
                                                  context->debug_entry_hierarchy, KEFIR_IR_DEBUG_ENTRY_LABEL,
                                                  &label_entry_id));
        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &context->module->debug_info.entries,
                                                      &context->module->symbols, label_entry_id,
                                                      &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME(node->label)));
        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
            mem, &context->module->debug_info.entries, &context->module->symbols, label_entry_id,
            &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_BEGIN(KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder))));
    }
    REQUIRE_OK(kefir_ast_translate_statement(mem, node->statement, builder, context));
    return KEFIR_OK;
}
