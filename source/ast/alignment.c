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

#include <string.h>
#include "kefir/ast/alignment.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

struct kefir_ast_alignment *kefir_ast_alignment_default(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_alignment *alignment = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_alignment));
    REQUIRE(alignment != NULL, NULL);
    *alignment = (const struct kefir_ast_alignment) {0};
    alignment->klass = KEFIR_AST_ALIGNMENT_DEFAULT;
    alignment->value = KEFIR_AST_DEFAULT_ALIGNMENT;
    return alignment;
}

struct kefir_ast_alignment *kefir_ast_alignment_as_type(struct kefir_mem *mem, const struct kefir_ast_type *type) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(type != NULL, NULL);
    struct kefir_ast_alignment *alignment = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_alignment));
    REQUIRE(alignment != NULL, NULL);
    *alignment = (const struct kefir_ast_alignment) {0};
    alignment->klass = KEFIR_AST_ALIGNMENT_AS_TYPE;
    alignment->value = KEFIR_AST_DEFAULT_ALIGNMENT;
    alignment->type = type;
    return alignment;
}

struct kefir_ast_alignment *kefir_ast_alignment_const_expression(struct kefir_mem *mem, kefir_size_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_alignment *alignment = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_alignment));
    REQUIRE(alignment != NULL, NULL);
    *alignment = (const struct kefir_ast_alignment) {0};
    alignment->klass = KEFIR_AST_ALIGNMENT_AS_CONST_EXPR;
    alignment->value = value;
    return alignment;
}

struct kefir_ast_alignment *kefir_ast_alignment_clone(struct kefir_mem *mem,
                                                      const struct kefir_ast_alignment *alignment) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(alignment != NULL, NULL);

    struct kefir_ast_alignment *new_alignment = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_alignment));
    REQUIRE(new_alignment != NULL, NULL);
    new_alignment->klass = alignment->klass;
    new_alignment->value = alignment->value;
    switch (alignment->klass) {
        case KEFIR_AST_ALIGNMENT_DEFAULT:
        case KEFIR_AST_ALIGNMENT_AS_CONST_EXPR:
            // Intentionally left blank
            break;

        case KEFIR_AST_ALIGNMENT_AS_TYPE:
            new_alignment->type = alignment->type;
            break;
    }
    return new_alignment;
}

kefir_result_t kefir_ast_alignment_free(struct kefir_mem *mem, struct kefir_ast_alignment *alignment) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(alignment != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST alignment"));
    switch (alignment->klass) {
        case KEFIR_AST_ALIGNMENT_DEFAULT:
            // Intentionally left blank
            break;

        case KEFIR_AST_ALIGNMENT_AS_TYPE:
            alignment->klass = KEFIR_AST_ALIGNMENT_DEFAULT;
            alignment->type = NULL;
            break;

        case KEFIR_AST_ALIGNMENT_AS_CONST_EXPR:
            alignment->klass = KEFIR_AST_ALIGNMENT_DEFAULT;
            break;
    }
    KEFIR_FREE(mem, alignment);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_alignment_evaluate(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                            struct kefir_ast_alignment *alignment) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(alignment != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST alignment"));

    switch (alignment->klass) {
        case KEFIR_AST_ALIGNMENT_DEFAULT:
            alignment->value = KEFIR_AST_DEFAULT_ALIGNMENT;
            break;

        case KEFIR_AST_ALIGNMENT_AS_TYPE: {
            kefir_ast_target_environment_opaque_type_t target_type;
            REQUIRE_OK(kefir_ast_context_type_cache_get_type(mem, context->cache, alignment->type, &target_type, NULL));
            struct kefir_ast_target_environment_object_info type_info;
            REQUIRE_OK(
                KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, target_type, NULL, &type_info));
            alignment->value = type_info.alignment;
        } break;

        case KEFIR_AST_ALIGNMENT_AS_CONST_EXPR:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}
