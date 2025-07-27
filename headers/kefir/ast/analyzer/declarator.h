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

#ifndef KEFIR_AST_ANALYZER_DECLARATOR_H_
#define KEFIR_AST_ANALYZER_DECLARATOR_H_

#include "kefir/ast/declarator.h"
#include "kefir/ast/type.h"
#include "kefir/ast/context.h"
#include "kefir/ast/alignment.h"

typedef struct kefir_declarator_analyzer_std_attribute_descriptor {
    const char *attribute;
    kefir_c_language_standard_version_t version;
} kefir_declarator_analyzer_std_attribute_descriptor_t;

extern const struct kefir_declarator_analyzer_std_attribute_descriptor
    KEFIR_DECLARATOR_ANALYZER_SUPPORTED_STD_ATTRIBUTES[];
extern const char *KEFIR_DECLARATOR_ANALYZER_SUPPORTED_GNU_ATTRIBUTES[];

#define KEFIR_AST_DECLARATION_ANALYSIS_NORMAL (0ull)
#define KEFIR_AST_DECLARATION_ANALYSIS_FUNCTION_DEFINITION_CONTEXT (1ull)
#define KEFIR_AST_DECLARATION_ANALYSIS_IGNORE_ALIGNMENT_SPECIFIER (1ull << 1)
#define KEFIR_AST_DECLARATION_ANALYSIS_FORBID_ALIGNMENT_DECREASE (1ull << 2)

kefir_result_t kefir_ast_analyze_declaration_specifiers(struct kefir_mem *, const struct kefir_ast_context *,
                                                        const struct kefir_ast_declarator_specifier_list *,
                                                        const struct kefir_ast_type **,
                                                        kefir_ast_scoped_identifier_storage_t *,
                                                        kefir_ast_function_specifier_t *, kefir_size_t *,
                                                        kefir_uint64_t, const struct kefir_source_location *);

kefir_result_t kefir_ast_analyze_declaration_declarator(struct kefir_mem *, const struct kefir_ast_context *,
                                                        const struct kefir_ast_declarator_specifier_list *,
                                                        const struct kefir_ast_declarator *, const char **,
                                                        const struct kefir_ast_type **, kefir_size_t *, kefir_uint64_t,
                                                        struct kefir_ast_declarator_attributes *);

kefir_result_t kefir_ast_analyze_declaration(struct kefir_mem *, const struct kefir_ast_context *,
                                             const struct kefir_ast_declarator_specifier_list *,
                                             const struct kefir_ast_declarator *, const char **,
                                             const struct kefir_ast_type **, kefir_ast_scoped_identifier_storage_t *,
                                             kefir_ast_function_specifier_t *, kefir_size_t *, kefir_uint64_t,
                                             struct kefir_ast_declarator_attributes *,
                                             const struct kefir_source_location *);

#endif
