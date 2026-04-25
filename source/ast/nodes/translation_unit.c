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
#include "kefir/ast/node_internal.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

NODE_VISIT_IMPL(ast_translation_unit_visit, kefir_ast_translation_unit, translation_unit)

kefir_result_t ast_translation_unit_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_translation_unit *, node, base->self);

    for (kefir_size_t i = 0; i < node->external_definitions_length; i++) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->external_definitions[i]));
    }
    KEFIR_FREE(mem, node->external_definitions);
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_TRANSLATION_UNIT_CLASS = {
    .type = KEFIR_AST_TRANSLATION_UNIT, .visit = ast_translation_unit_visit, .free = ast_translation_unit_free};

struct kefir_ast_translation_unit *kefir_ast_new_translation_unit(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_translation_unit *unit = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_translation_unit));
    REQUIRE(unit != NULL, NULL);
    unit->base.refcount = 1;
    unit->base.klass = &AST_TRANSLATION_UNIT_CLASS;
    unit->base.self = unit;
    unit->external_definitions = NULL;
    unit->external_definitions_capacity = 0;
    unit->external_definitions_length = 0;
    kefir_result_t res = kefir_ast_node_properties_init(&unit->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, unit);
        return NULL;
    });
    res = kefir_source_location_empty(&unit->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, unit);
        return NULL;
    });

    return unit;
}

kefir_result_t kefir_ast_translation_unit_append(struct kefir_mem *mem, struct kefir_ast_translation_unit *node,
                                                 struct kefir_ast_node_base *item) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST compound statement"));
    REQUIRE(item != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->external_definitions_length + 1 > node->external_definitions_capacity) {
        kefir_size_t new_capacity = MAX(1, 2 * node->external_definitions_capacity);
        struct kefir_ast_node_base **new_ext_defs =
            KEFIR_REALLOC(mem, node->external_definitions, sizeof(struct kefir_ast_node_base *) * new_capacity);
        REQUIRE(new_ext_defs != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE,
                                                      "Failed to allocate AST translation unit external definitions"));

        node->external_definitions_capacity = new_capacity;
        node->external_definitions = new_ext_defs;
    }
    node->external_definitions[node->external_definitions_length++] = item;
    return KEFIR_OK;
}
