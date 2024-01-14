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

#include <string.h>
#include "kefir/ast-translator/translator_impl.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/value.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/ir/module.h"
#include "kefir/ast-translator/scope/scoped_identifier.h"
#include "kefir/ast-translator/flow_control.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_translate_label_address_node(struct kefir_mem *mem,
                                                      struct kefir_ast_translator_context *context,
                                                      struct kefir_irbuilder_block *builder,
                                                      const struct kefir_ast_label_address *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST identifier node"));

    struct kefir_ast_flow_control_structure *label_parent =
        node->base.properties.expression_props.scoped_id->label.point->parent;
    while (label_parent != NULL) {
        if (label_parent->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK) {
            REQUIRE(!label_parent->value.block.contains_vla,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                           "None of blocks enclosing the label can contain VLAs"));
        }
        label_parent = label_parent->parent_point != NULL ? label_parent->parent_point->parent : NULL;
    }

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PUSHLABEL, 0));
    REQUIRE_OK(kefir_ast_translator_flow_control_point_reference(
        mem, node->base.properties.expression_props.scoped_id->label.point, builder->block,
        KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) - 1));
    return KEFIR_OK;
}
