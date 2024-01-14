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

#include "kefir/ast/format.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_format_initializer_designation(struct kefir_json_output *json,
                                                        const struct kefir_ast_initializer_designation *designation,
                                                        kefir_bool_t display_source_location) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(designation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST initializer designation"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    if (designation->indexed) {
        REQUIRE_OK(kefir_json_output_string(json, "index"));
        REQUIRE_OK(kefir_json_output_object_key(json, "index"));
        REQUIRE_OK(kefir_ast_format(json, designation->index, display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_string(json, "member"));
        REQUIRE_OK(kefir_json_output_object_key(json, "member"));
        REQUIRE_OK(kefir_json_output_string(json, designation->identifier));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "next"));
    if (designation->next != NULL) {
        REQUIRE_OK(kefir_ast_format_initializer_designation(json, designation->next, display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }

    if (display_source_location) {
        REQUIRE_OK(kefir_json_output_object_key(json, "source_location"));
        if (kefir_source_location_get(&designation->source_location, NULL, NULL, NULL)) {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "source"));
            REQUIRE_OK(kefir_json_output_string(json, designation->source_location.source));
            REQUIRE_OK(kefir_json_output_object_key(json, "line"));
            REQUIRE_OK(kefir_json_output_uinteger(json, designation->source_location.line));
            REQUIRE_OK(kefir_json_output_object_key(json, "column"));
            REQUIRE_OK(kefir_json_output_uinteger(json, designation->source_location.column));
            REQUIRE_OK(kefir_json_output_object_end(json));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_format_initializer(struct kefir_json_output *json,
                                            const struct kefir_ast_initializer *initializer,
                                            kefir_bool_t display_source_location) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid json output"));
    REQUIRE(initializer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST initializer"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    switch (initializer->type) {
        case KEFIR_AST_INITIALIZER_EXPRESSION:
            REQUIRE_OK(kefir_json_output_string(json, "expression"));
            REQUIRE_OK(kefir_json_output_object_key(json, "expression"));
            REQUIRE_OK(kefir_ast_format(json, initializer->expression, display_source_location));
            break;

        case KEFIR_AST_INITIALIZER_LIST:
            REQUIRE_OK(kefir_json_output_string(json, "list"));
            REQUIRE_OK(kefir_json_output_object_key(json, "list"));
            REQUIRE_OK(kefir_json_output_array_begin(json));
            for (const struct kefir_list_entry *iter = kefir_list_head(&initializer->list.initializers); iter != NULL;
                 kefir_list_next(&iter)) {
                ASSIGN_DECL_CAST(struct kefir_ast_initializer_list_entry *, entry, iter->value);

                REQUIRE_OK(kefir_json_output_object_begin(json));
                REQUIRE_OK(kefir_json_output_object_key(json, "designation"));
                if (entry->designation != NULL) {
                    REQUIRE_OK(
                        kefir_ast_format_initializer_designation(json, entry->designation, display_source_location));
                } else {
                    REQUIRE_OK(kefir_json_output_null(json));
                }
                REQUIRE_OK(kefir_json_output_object_key(json, "initializer"));
                REQUIRE_OK(kefir_ast_format_initializer(json, entry->value, display_source_location));
                REQUIRE_OK(kefir_json_output_object_end(json));
            }
            REQUIRE_OK(kefir_json_output_array_end(json));
            break;
    }

    if (display_source_location) {
        REQUIRE_OK(kefir_json_output_object_key(json, "source_location"));
        if (kefir_source_location_get(&initializer->source_location, NULL, NULL, NULL)) {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "source"));
            REQUIRE_OK(kefir_json_output_string(json, initializer->source_location.source));
            REQUIRE_OK(kefir_json_output_object_key(json, "line"));
            REQUIRE_OK(kefir_json_output_uinteger(json, initializer->source_location.line));
            REQUIRE_OK(kefir_json_output_object_key(json, "column"));
            REQUIRE_OK(kefir_json_output_uinteger(json, initializer->source_location.column));
            REQUIRE_OK(kefir_json_output_object_end(json));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}
