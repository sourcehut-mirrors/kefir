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

#include "kefir/ast/context_impl.h"
#include "kefir/ast/local_context.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_context_free_scoped_identifier(struct kefir_mem *mem,
                                                        struct kefir_ast_scoped_identifier *scoped_id, void *payload) {
    UNUSED(payload);
    REQUIRE_OK(kefir_ast_scoped_identifier_run_cleanup(mem, scoped_id));
    switch (scoped_id->klass) {
        case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
            REQUIRE_OK(kefir_ast_alignment_free(mem, scoped_id->object.alignment));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
            if (scoped_id->function.local_context != NULL) {
                REQUIRE_OK(kefir_ast_local_context_free(mem, scoped_id->function.local_context));
                KEFIR_FREE(mem, scoped_id->function.local_context);
                scoped_id->function.local_context = NULL;
            }
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT:
            REQUIRE_OK(kefir_ast_constant_expression_free(mem, scoped_id->enum_constant.value));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_LABEL:
            if (scoped_id->label.point != NULL) {
                REQUIRE_OK(kefir_ast_flow_control_point_free(mem, scoped_id->label.point));
                scoped_id->label.point = NULL;
            }
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION:
            if (scoped_id->type_definition.alignment != NULL) {
                REQUIRE_OK(kefir_ast_alignment_free(mem, scoped_id->type_definition.alignment));
            }
            scoped_id->type_definition.type = NULL;
            break;

        default:
            break;
    }
    KEFIR_FREE(mem, scoped_id);
    return KEFIR_OK;
}

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_object_identifier(
    struct kefir_mem *mem, const struct kefir_ast_type *type, struct kefir_ast_identifier_flat_scope *definition_scope,
    kefir_ast_scoped_identifier_storage_t storage, struct kefir_ast_alignment *alignment,
    kefir_ast_scoped_identifier_linkage_t linkage, kefir_bool_t external, struct kefir_ast_initializer *initializer,
    const char *asm_label, const struct kefir_source_location *source_location) {
    struct kefir_ast_scoped_identifier *scoped_id = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_scoped_identifier));
    scoped_id->klass = KEFIR_AST_SCOPE_IDENTIFIER_OBJECT;
    scoped_id->cleanup.callback = NULL;
    scoped_id->cleanup.payload = NULL;
    scoped_id->definition_scope = definition_scope;
    scoped_id->object.type = type;
    scoped_id->object.storage = storage;
    scoped_id->object.external = external;
    scoped_id->object.linkage = linkage;
    scoped_id->object.initializer = initializer;
    scoped_id->object.visibility = KEFIR_AST_DECLARATOR_VISIBILITY_DEFAULT;
    scoped_id->object.asm_label = asm_label;
    scoped_id->object.vl_array = KEFIR_ID_NONE;
    scoped_id->object.alias = NULL;
    scoped_id->object.flags.weak = false;
    scoped_id->object.defining_function = NULL;
    if (source_location != NULL) {
        scoped_id->source_location = *source_location;
    } else {
        kefir_result_t res = kefir_source_location_empty(&scoped_id->source_location);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, scoped_id);
            return NULL;
        });
    }
    memset(scoped_id->payload.content, 0, KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE);
    scoped_id->payload.ptr = scoped_id->payload.content;
    scoped_id->payload.cleanup = &scoped_id->cleanup;
    if (alignment != NULL) {
        scoped_id->object.alignment = alignment;
    } else {
        scoped_id->object.alignment = kefir_ast_alignment_default(mem);
        REQUIRE_ELSE(scoped_id->object.alignment != NULL, {
            KEFIR_FREE(mem, scoped_id);
            return NULL;
        });
    }
    return scoped_id;
}

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_constant(
    struct kefir_mem *mem, struct kefir_ast_constant_expression *value, const struct kefir_ast_type *type,
    const struct kefir_source_location *source_location) {
    struct kefir_ast_scoped_identifier *scoped_id = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_scoped_identifier));
    scoped_id->klass = KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT;
    scoped_id->cleanup.callback = NULL;
    scoped_id->cleanup.payload = NULL;
    scoped_id->enum_constant.type = type;
    scoped_id->enum_constant.value = value;
    if (source_location != NULL) {
        scoped_id->source_location = *source_location;
    } else {
        kefir_result_t res = kefir_source_location_empty(&scoped_id->source_location);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, scoped_id);
            return NULL;
        });
    }
    memset(scoped_id->payload.content, 0, KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE);
    scoped_id->payload.ptr = scoped_id->payload.content;
    scoped_id->payload.cleanup = &scoped_id->cleanup;
    return scoped_id;
}

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_type_tag(
    struct kefir_mem *mem, const struct kefir_ast_type *type, const struct kefir_source_location *source_location) {
    struct kefir_ast_scoped_identifier *scoped_id = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_scoped_identifier));
    scoped_id->klass = KEFIR_AST_SCOPE_IDENTIFIER_TYPE_TAG;
    scoped_id->cleanup.callback = NULL;
    scoped_id->cleanup.payload = NULL;
    scoped_id->type = type;
    if (source_location != NULL) {
        scoped_id->source_location = *source_location;
    } else {
        kefir_result_t res = kefir_source_location_empty(&scoped_id->source_location);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, scoped_id);
            return NULL;
        });
    }
    memset(scoped_id->payload.content, 0, KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE);
    scoped_id->payload.ptr = scoped_id->payload.content;
    scoped_id->payload.cleanup = &scoped_id->cleanup;
    return scoped_id;
}

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_type_definition(
    struct kefir_mem *mem, const struct kefir_ast_type *type, struct kefir_ast_alignment *alignment,
    const struct kefir_source_location *source_location) {
    struct kefir_ast_scoped_identifier *scoped_id = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_scoped_identifier));
    scoped_id->klass = KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION;
    scoped_id->cleanup.callback = NULL;
    scoped_id->cleanup.payload = NULL;
    scoped_id->type_definition.type = type;
    scoped_id->type_definition.alignment = alignment;
    if (source_location != NULL) {
        scoped_id->source_location = *source_location;
    } else {
        kefir_result_t res = kefir_source_location_empty(&scoped_id->source_location);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, scoped_id);
            return NULL;
        });
    }
    memset(scoped_id->payload.content, 0, KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE);
    scoped_id->payload.ptr = scoped_id->payload.content;
    scoped_id->payload.cleanup = &scoped_id->cleanup;
    return scoped_id;
}

kefir_result_t kefir_ast_context_allocate_scoped_type_definition_update_alignment(
    struct kefir_mem *mem, struct kefir_ast_scoped_identifier *scoped_id, struct kefir_ast_alignment *alignment) {
    if (scoped_id->type_definition.alignment == NULL) {
        scoped_id->type_definition.alignment = alignment;
    } else if (alignment != NULL && scoped_id->type_definition.alignment->value < alignment->value) {
        REQUIRE_OK(kefir_ast_alignment_free(mem, scoped_id->type_definition.alignment));
        scoped_id->type_definition.alignment = alignment;
    } else {
        REQUIRE_OK(kefir_ast_alignment_free(mem, alignment));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_context_type_retrieve_tag(const struct kefir_ast_type *type, const char **identifier) {
    switch (type->tag) {
        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION:
            REQUIRE(type->structure_type.identifier != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected struct/union with a tag"));
            *identifier = type->structure_type.identifier;
            return KEFIR_OK;

        case KEFIR_AST_TYPE_ENUMERATION:
            REQUIRE(type->enumeration_type.identifier != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected enum with a tag"));
            *identifier = type->enumeration_type.identifier;
            return KEFIR_OK;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected AST structure, union or enum type");
    }
}

kefir_result_t kefir_ast_context_update_existing_scoped_type_tag(struct kefir_ast_scoped_identifier *scoped_id,
                                                                 const struct kefir_ast_type *type) {
    REQUIRE(scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_TYPE_TAG,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot redefine with different kind of symbol"));
    REQUIRE(scoped_id->type->tag == type->tag,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot redefine tag with different type"));
    switch (scoped_id->type->tag) {
        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION:
            if (type->structure_type.complete) {
                REQUIRE(!scoped_id->type->structure_type.complete,
                        KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot redefine complete struct/union"));
                scoped_id->type = type;
            }
            return KEFIR_OK;

        case KEFIR_AST_TYPE_ENUMERATION:
            if (type->enumeration_type.complete) {
                REQUIRE(!scoped_id->type->enumeration_type.complete,
                        KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot redefine complete enumeration"));
                scoped_id->type = type;
            }
            return KEFIR_OK;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected AST type");
    }
}

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_function_identifier(
    struct kefir_mem *mem, const struct kefir_ast_type *type, kefir_ast_function_specifier_t specifier,
    kefir_ast_scoped_identifier_storage_t storage, kefir_bool_t external, kefir_bool_t defined,
    kefir_bool_t inline_definition, const char *alias, const char *asm_label,
    const struct kefir_source_location *source_location) {
    struct kefir_ast_scoped_identifier *scoped_id = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_scoped_identifier));
    scoped_id->klass = KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION;
    scoped_id->cleanup.callback = NULL;
    scoped_id->cleanup.payload = NULL;
    scoped_id->function.type = type;
    scoped_id->function.specifier = specifier;
    scoped_id->function.storage = storage;
    scoped_id->function.external = external;
    scoped_id->function.defined = defined;
    scoped_id->function.inline_definition = inline_definition;
    scoped_id->object.visibility = KEFIR_AST_DECLARATOR_VISIBILITY_DEFAULT;
    scoped_id->function.alias = alias;
    scoped_id->function.flags.weak = false;
    scoped_id->function.flags.gnu_inline = false;
    scoped_id->function.flags.always_inline = false;
    scoped_id->function.flags.noinline = false;
    scoped_id->function.flags.constructor = false;
    scoped_id->function.flags.destructor = false;
    scoped_id->function.local_context = NULL;
    scoped_id->function.local_context_ptr = &scoped_id->function.local_context;
    scoped_id->function.asm_label = asm_label;
    if (source_location != NULL) {
        scoped_id->source_location = *source_location;
    } else {
        kefir_result_t res = kefir_source_location_empty(&scoped_id->source_location);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, scoped_id);
            return NULL;
        });
    }
    memset(scoped_id->payload.content, 0, KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE);
    scoped_id->payload.ptr = scoped_id->payload.content;
    scoped_id->payload.cleanup = &scoped_id->cleanup;
    return scoped_id;
}

struct kefir_ast_scoped_identifier *kefir_ast_context_allocate_scoped_label(
    struct kefir_mem *mem, struct kefir_ast_flow_control_structure *parent,
    const struct kefir_source_location *source_location) {
    struct kefir_ast_scoped_identifier *scoped_id = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_scoped_identifier));
    scoped_id->klass = KEFIR_AST_SCOPE_IDENTIFIER_LABEL;
    scoped_id->cleanup.callback = NULL;
    scoped_id->cleanup.payload = NULL;
    if (source_location != NULL) {
        scoped_id->source_location = *source_location;
    } else {
        kefir_result_t res = kefir_source_location_empty(&scoped_id->source_location);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, scoped_id);
            return NULL;
        });
    }
    memset(scoped_id->payload.content, 0, KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE);
    scoped_id->payload.ptr = scoped_id->payload.content;
    scoped_id->payload.cleanup = &scoped_id->cleanup;
    scoped_id->label.point = kefir_ast_flow_control_point_alloc(mem, parent);
    scoped_id->label.public_label = NULL;
    REQUIRE_ELSE(scoped_id->label.point != NULL, {
        KEFIR_FREE(mem, scoped_id);
        return NULL;
    });
    return scoped_id;
}

kefir_result_t kefir_ast_context_merge_alignment(struct kefir_mem *mem, struct kefir_ast_alignment **original,
                                                 struct kefir_ast_alignment *alignment) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(original != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid original alignment pointer"));
    REQUIRE(alignment != NULL, KEFIR_OK);
    if (*original == NULL) {
        *original = alignment;
    } else if (alignment->value > (*original)->value) {
        REQUIRE_OK(kefir_ast_alignment_free(mem, *original));
        *original = alignment;
    } else {
        REQUIRE_OK(kefir_ast_alignment_free(mem, alignment));
    }
    return KEFIR_OK;
}
