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

#ifndef KEFIR_AST_TRANSLATOR_SCOPE_SCOPED_IDENTIFIER_H_
#define KEFIR_AST_TRANSLATOR_SCOPE_SCOPED_IDENTIFIER_H_

#include "kefir/ast/local_context.h"
#include "kefir/ast-translator/environment.h"
#include "kefir/ast-translator/debug/translator.h"
#include "kefir/ir/builder.h"
#include "kefir/ir/module.h"
#include "kefir/ir/module.h"
#include "kefir/core/hashtree.h"
#include "kefir/ast/type_layout.h"
#include "kefir/ast-translator/function_declaration.h"

typedef struct kefir_ast_translator_scoped_identifier_entry {
    const char *identifier;
    const struct kefir_ast_scoped_identifier *value;
} kefir_ast_translator_scoped_identifier_t;

typedef struct kefir_ast_translator_scoped_identifier_object {
    kefir_id_t identifier;
    kefir_id_t type_id;
    struct kefir_ir_type *type;
    struct kefir_ast_type_layout *layout;
    struct {
        kefir_bool_t present;
        kefir_ir_debug_entry_id_t variable;
    } debug_info;
} kefir_ast_translator_scoped_identifier_object_t;

typedef struct kefir_ast_translator_scoped_identifier_function {
    struct kefir_ast_translator_function_declaration *declaration;
    struct {
        kefir_bool_t present;
        kefir_ir_debug_entry_id_t subprogram;
    } debug_info;
} kefir_ast_translator_scoped_identifier_function_t;

_Static_assert(sizeof(struct kefir_ast_translator_scoped_identifier_object) <= KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE,
               "Unable to fit scoped identifier object into payload field");

_Static_assert(sizeof(struct kefir_ast_translator_scoped_identifier_function) <=
                   KEFIR_AST_SCOPED_IDENTIFIER_PAYLOAD_SIZE,
               "Unable to fit scoped identifier function into payload field");

kefir_result_t kefir_ast_translator_scope_layout_complete_object_type(struct kefir_mem *,
                                                                      const struct kefir_ast_context *, const char *,
                                                                      const struct kefir_ast_scoped_identifier *,
                                                                      const struct kefir_ast_type **);

#endif
