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

#include "kefir/ast/node.h"
#include "kefir/ast/node_internal.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

NODE_VISIT_IMPL(ast_translation_unit_visit, kefir_ast_translation_unit, translation_unit)

kefir_result_t ast_translation_unit_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_translation_unit *, node, base->self);

    REQUIRE_OK(kefir_list_free(mem, &node->external_definitions));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_TRANSLATION_UNIT_CLASS = {
    .type = KEFIR_AST_TRANSLATION_UNIT, .visit = ast_translation_unit_visit, .free = ast_translation_unit_free};

static kefir_result_t external_definition_free(struct kefir_mem *mem, struct kefir_list *list,
                                               struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, ext_def, entry->value);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, ext_def));
    return KEFIR_OK;
}

struct kefir_ast_translation_unit *kefir_ast_new_translation_unit(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_translation_unit *unit = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_translation_unit));
    REQUIRE(unit != NULL, NULL);
    unit->base.refcount = 1;
    unit->base.klass = &AST_TRANSLATION_UNIT_CLASS;
    unit->base.self = unit;
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

    res = kefir_list_init(&unit->external_definitions);
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&unit->external_definitions, external_definition_free, NULL));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, unit);
        return NULL;
    });

    return unit;
}
