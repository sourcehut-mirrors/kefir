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

#include "kefir/util/json.h"
#include "kefir/ast-translator/layout.h"

static kefir_result_t dump_type_layout(struct kefir_json_output *json, const struct kefir_ast_type_layout *layout) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, layout->value));
    REQUIRE_OK(kefir_json_output_object_key(json, "size"));
    REQUIRE_OK(kefir_json_output_uinteger(json, layout->properties.size));
    REQUIRE_OK(kefir_json_output_object_key(json, "alignment"));
    REQUIRE_OK(kefir_json_output_uinteger(json, layout->properties.alignment));
    REQUIRE_OK(kefir_json_output_object_key(json, "relative_offset"));
    REQUIRE_OK(kefir_json_output_uinteger(json, layout->properties.relative_offset));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    switch (layout->type->tag) {
        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION: {
            REQUIRE_OK(
                kefir_json_output_string(json, layout->type->tag == KEFIR_AST_TYPE_STRUCTURE ? "struct" : "union"));
            REQUIRE_OK(kefir_json_output_object_key(json, "fields"));
            REQUIRE_OK(kefir_json_output_array_begin(json));

            for (const struct kefir_list_entry *iter = kefir_list_head(&layout->structure_layout.member_list);
                 iter != NULL; kefir_list_next(&iter)) {
                ASSIGN_DECL_CAST(const struct kefir_ast_type_layout_structure_member *, member, iter->value);
                REQUIRE_OK(kefir_json_output_object_begin(json));
                REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
                if (member->identifier != NULL) {
                    REQUIRE_OK(kefir_json_output_string(json, member->identifier));
                } else {
                    REQUIRE_OK(kefir_json_output_null(json));
                }
                REQUIRE_OK(kefir_json_output_object_key(json, "layout"));
                REQUIRE_OK(dump_type_layout(json, member->layout));
                REQUIRE_OK(kefir_json_output_object_end(json));
            }
            REQUIRE_OK(kefir_json_output_array_end(json));
        } break;

        case KEFIR_AST_TYPE_ARRAY:
            REQUIRE_OK(kefir_json_output_string(json, "array"));
            REQUIRE_OK(kefir_json_output_object_key(json, "element"));
            REQUIRE_OK(dump_type_layout(json, layout->array_layout.element_type));
            break;

        default:
            REQUIRE_OK(kefir_json_output_string(json, "scalar"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t dump_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                struct kefir_json_output *json, const struct kefir_ast_type *type) {
    struct kefir_ir_type ir_type;
    struct kefir_irbuilder_type builder;
    struct kefir_ast_translator_environment env;

    REQUIRE_OK(kefir_ir_type_alloc(mem, 0, &ir_type));
    REQUIRE_OK(kefir_irbuilder_type_init(mem, &builder, &ir_type));
    REQUIRE_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));

    struct kefir_ast_type_layout *layout1 = NULL;
    REQUIRE_OK(kefir_ast_translate_object_type(mem, context, type, 0, &env, &builder, &layout1, NULL));
    REQUIRE_OK(kefir_ast_translator_evaluate_type_layout(mem, &env, layout1, &ir_type));
    REQUIRE(layout1 != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(dump_type_layout(json, layout1));
    REQUIRE_OK(kefir_ast_type_layout_free(mem, layout1));

    REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&builder));
    REQUIRE_OK(kefir_ir_type_free(mem, &ir_type));
    return KEFIR_OK;
}
