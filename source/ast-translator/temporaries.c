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

#include <stdio.h>
#include "kefir/ast-translator/temporaries.h"
#include "kefir/ast-translator/scope/scoped_identifier.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/ast/runtime.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_translator_fetch_temporary(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                    struct kefir_irbuilder_block *builder,
                                                    const struct kefir_ast_temporary_identifier *temporary) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(temporary != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST temporary identifier"));
    REQUIRE(temporary->valid, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unable to fetch invalid temporary"));

#define BUFFER_LEN 128
    if (temporary->mode == KEFIR_AST_TEMPORARY_MODE_LOCAL_NESTED) {
        const struct kefir_ast_scoped_identifier *scoped_id = NULL;
        REQUIRE_OK(context->ast_context->resolve_ordinary_identifier(
            context->ast_context, KEFIR_AST_TRANSLATOR_TEMPORARIES_IDENTIFIER, &scoped_id));
        ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_id_layout,
                         scoped_id->payload.ptr);

        char TEMP_VALUE[BUFFER_LEN] = {0}, TEMP_MEMBER[BUFFER_LEN] = {0};

        snprintf(TEMP_VALUE, BUFFER_LEN - 1, KEFIR_AST_TRANSLATOR_TEMPORARY_VALUE_IDENTIFIER, temporary->identifier);
        snprintf(TEMP_MEMBER, BUFFER_LEN - 1, KEFIR_AST_TRANSLATOR_TEMPORARY_MEMBER_IDENTIFIER, temporary->field);

        struct kefir_ast_designator temp_value_designator = {
            .type = KEFIR_AST_DESIGNATOR_MEMBER, .member = TEMP_VALUE, .next = NULL};
        struct kefir_ast_designator temp_member_designator = {
            .type = KEFIR_AST_DESIGNATOR_MEMBER, .member = TEMP_MEMBER, .next = NULL};

        struct kefir_ast_type_layout *temp_value_layout = NULL, *temp_member_layout = NULL;
        REQUIRE_OK(kefir_ast_type_layout_resolve(scoped_id_layout->layout, &temp_value_designator, &temp_value_layout,
                                                 NULL, NULL));
        REQUIRE_OK(
            kefir_ast_type_layout_resolve(temp_value_layout, &temp_member_designator, &temp_member_layout, NULL, NULL));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IROPCODE_GETLOCAL, scoped_id_layout->type_id,
                                                   scoped_id_layout->layout->value));
        kefir_size_t total_offset =
            temp_value_layout->properties.relative_offset + temp_member_layout->properties.relative_offset;
        if (total_offset > 0) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IADD1, total_offset));
        }
    } else if (temporary->mode == KEFIR_AST_TEMPORARY_MODE_GLOBAL) {
        char buf[BUFFER_LEN] = {0};
        snprintf(buf, sizeof(buf) - 1, KEFIR_AST_TRANSLATOR_TEMPORARY_GLOBAL_IDENTIFIER, temporary->identifier,
                 temporary->field);

        kefir_id_t id;
        REQUIRE(kefir_ir_module_symbol(mem, context->module, buf, &id) != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR module symbol"));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_GETGLOBAL, id));
    } else {
        char buf[BUFFER_LEN] = {0};
        snprintf(buf, sizeof(buf) - 1, KEFIR_AST_TRANSLATOR_TEMPORARY_LOCAL_IDENTIFIER, temporary->identifier,
                 temporary->field);

        const struct kefir_ast_scoped_identifier *scoped_id = NULL;
        REQUIRE_OK(context->ast_context->resolve_ordinary_identifier(context->ast_context, buf, &scoped_id));
        REQUIRE_OK(kefir_ast_translator_object_lvalue(mem, context, builder, buf, scoped_id));
    }
#undef BUFFER_LEN
    return KEFIR_OK;
}
