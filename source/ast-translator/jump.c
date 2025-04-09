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

#include "kefir/ast-translator/jump.h"
#include "kefir/ast-translator/flow_control.h"
#include "kefir/ast-translator/misc.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t perform_jump(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                   struct kefir_irbuilder_block *builder,
                                   struct kefir_ast_flow_control_point *original_position,
                                   struct kefir_ast_flow_control_point *target_position,
                                   struct kefir_list *target_parents,
                                   struct kefir_ast_flow_control_structure *common_parent,
                                   const struct kefir_source_location *source_location) {
    struct kefir_ast_flow_control_structure *top_target_block = target_position->self;
    for (const struct kefir_list_entry *iter = kefir_list_head(target_parents); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_flow_control_structure *, control_struct, iter->value);
        if (control_struct->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK) {
            REQUIRE(!kefir_ast_flow_control_block_contains_vl_arrays(control_struct),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                           "Cannot jump into scope with local VLA variables"));
        }
        top_target_block = control_struct;
    }

    struct kefir_ast_flow_control_structure *top_origin_block = original_position->self;
    struct kefir_ast_flow_control_structure *current_origin_parent =
        kefir_ast_flow_control_structure_parent(original_position->self);
    while (current_origin_parent != NULL && current_origin_parent != common_parent) {
        if (current_origin_parent->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK) {
            if (kefir_ast_flow_control_block_contains_vl_arrays(current_origin_parent)) {
                kefir_id_t vla_element;
                REQUIRE_OK(kefir_ast_flow_control_block_vl_array_head(current_origin_parent, &vla_element));

                REQUIRE_OK(kefir_ast_translator_resolve_vla_element(mem, context, builder, vla_element));
                REQUIRE_OK(
                    KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_LOAD, KEFIR_IR_MEMORY_FLAG_NONE));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCOPE_POP, 0));
            }
        }
        top_origin_block = current_origin_parent;
        current_origin_parent = kefir_ast_flow_control_structure_parent(current_origin_parent);
    }

    REQUIRE(top_target_block != top_origin_block && kefir_ast_flow_control_structure_parent(top_target_block) != NULL &&
                kefir_ast_flow_control_structure_parent(top_origin_block) != NULL &&
                kefir_ast_flow_control_structure_parent(top_target_block) ==
                    kefir_ast_flow_control_structure_parent(top_origin_block) &&
                kefir_ast_flow_control_structure_parent(top_target_block) == common_parent,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST flow control structure"));

    kefir_bool_t found_origin_first = false, found_target_first = false;
    for (const struct kefir_ast_flow_control_structure *iter =
             kefir_ast_flow_control_structure_first_child(common_parent);
         iter != NULL; iter = kefir_ast_flow_control_structure_next_sibling(iter)) {
        if (found_origin_first) {
            if (iter == top_target_block) {
                break;
            } else {
                REQUIRE(iter->type != KEFIR_AST_FLOW_CONTROL_VL_ARRAY,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                               "Cannot jump in the scope with uninitialized VLA variables"));
            }
        } else if (found_target_first) {
            if (iter == top_origin_block) {
                break;
            } else if (iter->type == KEFIR_AST_FLOW_CONTROL_VL_ARRAY) {
                REQUIRE_OK(kefir_ast_translator_resolve_vla_element(mem, context, builder, iter->value.vl_array_id));
                REQUIRE_OK(
                    KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_LOAD, KEFIR_IR_MEMORY_FLAG_NONE));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCOPE_POP, 0));
                break;
            }
        } else if (iter == top_origin_block) {
            found_origin_first = true;
        } else if (iter == top_target_block) {
            found_target_first = true;
        }
    }
    REQUIRE(found_origin_first || found_target_first,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST flow control structure"));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_JUMP, 0));
    REQUIRE_OK(kefir_ast_translator_flow_control_point_reference(mem, target_position, builder->block,
                                                                 KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) - 1));

    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_jump(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                        struct kefir_irbuilder_block *builder,
                                        struct kefir_ast_flow_control_point *original_position,
                                        struct kefir_ast_flow_control_point *target_position,
                                        const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(original_position != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid origin AST flow control point"));
    REQUIRE(target_position != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target AST flow control point"));
    REQUIRE(target_position->self != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected target AST flow control point bound to flow control structure"));

    struct kefir_list target_parents;
    REQUIRE_OK(kefir_list_init(&target_parents));
    struct kefir_ast_flow_control_structure *common_parent = NULL;
    kefir_result_t res = kefir_ast_flow_control_point_common_parent(original_position, target_position, &common_parent);
    REQUIRE_CHAIN(&res, kefir_ast_flow_control_point_parents(mem, target_position, &target_parents, common_parent));
    REQUIRE_CHAIN(&res, perform_jump(mem, context, builder, original_position, target_position, &target_parents,
                                     common_parent, source_location));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &target_parents);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &target_parents));
    return KEFIR_OK;
}
