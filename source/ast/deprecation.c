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

#include "kefir/ast/deprecation.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_ast_check_type_deprecation(const struct kefir_ast_context *context,
                                                const struct kefir_ast_type *type,
                                                const struct kefir_source_location *source_location) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));

    if (context->configuration->warning_output != NULL) {
        kefir_bool_t deprecated = false;
        const char *deprecated_message = NULL;

        const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(type);
        if ((unqualified_type->tag == KEFIR_AST_TYPE_STRUCTURE || unqualified_type->tag == KEFIR_AST_TYPE_UNION) &&
            unqualified_type->structure_type.flags.deprecated) {
            deprecated = true;
            deprecated_message = unqualified_type->structure_type.flags.deprecated_message;
        } else if (unqualified_type->tag == KEFIR_AST_TYPE_ENUMERATION &&
                   unqualified_type->enumeration_type.flags.deprecated) {
            deprecated = true;
            deprecated_message = unqualified_type->enumeration_type.flags.deprecated_message;
        }

        if (deprecated) {
            if (deprecated_message == NULL) {
                deprecated_message = "the type has been deprecated";
            }
            if (source_location != NULL && source_location->source != NULL) {
                fprintf(context->configuration->warning_output,
                        "%s@%" KEFIR_UINT_FMT ":%" KEFIR_UINT_FMT " warning: %s\n", source_location->source,
                        source_location->line, source_location->column, deprecated_message);
            } else {
                fprintf(context->configuration->warning_output, "warning: %s\n", deprecated_message);
            }
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_ast_check_field_deprecation(const struct kefir_ast_context *context,
                                                 const struct kefir_ast_struct_field *field,
                                                 const struct kefir_source_location *source_location) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(field != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST structure field"));

    if (context->configuration->warning_output != NULL) {
        kefir_bool_t deprecated = field->flags.deprecated;
        const char *deprecated_message = field->flags.deprecated_message;

        if (deprecated) {
            if (deprecated_message == NULL) {
                deprecated_message = "the field has been deprecated";
            }
            if (source_location != NULL && source_location->source != NULL) {
                fprintf(context->configuration->warning_output,
                        "%s@%" KEFIR_UINT_FMT ":%" KEFIR_UINT_FMT " warning: %s\n", source_location->source,
                        source_location->line, source_location->column, deprecated_message);
            } else {
                fprintf(context->configuration->warning_output, "warning: %s\n", deprecated_message);
            }
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_ast_check_scoped_identifier_deprecation(const struct kefir_ast_context *context,
                                                             const struct kefir_ast_scoped_identifier *scoped_id,
                                                             const struct kefir_source_location *source_location) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(scoped_id != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST scoped identifier"));

    if (context->configuration->warning_output != NULL) {
        kefir_bool_t deprecated = false;
        const char *deprecated_message = NULL;

        switch (scoped_id->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
                deprecated = scoped_id->object.flags.deprecated;
                deprecated_message = scoped_id->object.flags.deprecated_message;
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
                deprecated = scoped_id->function.flags.deprecated;
                deprecated_message = scoped_id->function.flags.deprecated_message;
                break;

            default:
                // Intentionally left blank
                break;
        }

        if (deprecated) {
            if (deprecated_message == NULL) {
                deprecated_message = "the identifier has been deprecated";
            }
            if (source_location != NULL && source_location->source != NULL) {
                fprintf(context->configuration->warning_output,
                        "%s@%" KEFIR_UINT_FMT ":%" KEFIR_UINT_FMT " warning: %s\n", source_location->source,
                        source_location->line, source_location->column, deprecated_message);
            } else {
                fprintf(context->configuration->warning_output, "warning: %s\n", deprecated_message);
            }
        }
    }

    return KEFIR_OK;
}
