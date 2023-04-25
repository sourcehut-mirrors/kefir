/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
    struct kefir_ast_flow_control_point *top_target_block = target_position;
    for (const struct kefir_list_entry *iter = kefir_list_head(target_parents); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_flow_control_structure *, control_struct, iter->value);
        if (control_struct->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK) {
            REQUIRE(!control_struct->value.block.contains_vla,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                           "Cannot jump into scope with local VLA variables"));
        }
        top_target_block = control_struct->parent_point;
    }

    struct kefir_ast_flow_control_point *top_origin_block = original_position;
    struct kefir_ast_flow_control_structure *current_origin_parent = original_position->parent;
    while (current_origin_parent != NULL && current_origin_parent != common_parent) {
        if (current_origin_parent->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK) {
            if (current_origin_parent->value.block.contains_vla) {
                const struct kefir_ast_flow_control_data_element *vla_element = NULL;
                REQUIRE_OK(kefir_ast_flow_control_structure_data_element_head(
                    &current_origin_parent->value.block.data_elements, &vla_element));

                REQUIRE_OK(kefir_ast_translator_resolve_vla_element(mem, context, builder, vla_element->identifier));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LOAD64, KEFIR_IR_MEMORY_FLAG_NONE));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_POPSCOPE, 0));
            }
        }
        top_origin_block = current_origin_parent->parent_point;
        current_origin_parent =
            current_origin_parent->parent_point != NULL ? current_origin_parent->parent_point->parent : NULL;
    }

    const struct kefir_list_entry *top_target_block_iter = top_target_block->parent_data_elts.head;
    const struct kefir_list_entry *top_origin_block_iter = top_origin_block->parent_data_elts.head;
    for (; top_target_block_iter != NULL;
         kefir_list_next(&top_target_block_iter), kefir_list_next(&top_origin_block_iter)) {
        REQUIRE(top_origin_block_iter == top_target_block_iter,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                       "Cannot jump in the scope with uninitialized VLA variables"));

        if (top_target_block_iter == top_target_block->parent_data_elts.tail) {
            kefir_list_next(&top_origin_block_iter);
            break;
        }
        REQUIRE(top_origin_block_iter != top_origin_block->parent_data_elts.tail,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location,
                                       "Cannot jump in the scope with uninitialized VLA variables"));
    }

    if (top_origin_block_iter != NULL) {
        const struct kefir_ast_flow_control_data_element *vla_element = top_origin_block_iter->value;
        REQUIRE_OK(kefir_ast_flow_control_structure_data_element_head(&current_origin_parent->value.block.data_elements,
                                                                      &vla_element));

        REQUIRE_OK(kefir_ast_translator_resolve_vla_element(mem, context, builder, vla_element->identifier));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LOAD64, KEFIR_IR_MEMORY_FLAG_NONE));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_POPSCOPE, 0));
    }

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_JMP, 0));
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
    REQUIRE(target_position->parent != NULL,
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
