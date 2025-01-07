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

#include <stdio.h>
#include "kefir/core/mem.h"
#include "kefir/core/util.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/layout.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/test/util.h"
#include "kefir/util/json.h"
#include "./translate_type_common.c"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    struct kefir_ast_translator_environment env;
    struct kefir_ast_global_context global_context;

    REQUIRE_OK(kefir_string_pool_init(&symbols));
    REQUIRE_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    REQUIRE_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));
    REQUIRE_OK(
        kefir_ast_global_context_init(mem, kefir_util_default_type_traits(), &env.target_env, &global_context, NULL));

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, stdout, 4));
    REQUIRE_OK(kefir_json_output_array_begin(&json));

    struct kefir_ast_struct_type *struct_type1 = NULL;
    const struct kefir_ast_type *type1 = kefir_ast_type_structure(mem, &type_bundle, "struct1", &struct_type1);
    REQUIRE_OK(kefir_ast_struct_type_field(mem, &symbols, struct_type1, "x", kefir_ast_type_char(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(mem, &symbols, struct_type1, "y",
                                           kefir_ast_type_array(mem, &type_bundle, kefir_ast_type_char(),
                                                                kefir_ast_constant_expression_integer(mem, 1), NULL),
                                           NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(
        mem, &symbols, struct_type1, "z",
        kefir_ast_type_unbounded_array(mem, &type_bundle, kefir_ast_type_signed_int(), NULL), NULL));
    REQUIRE_OK(dump_type(mem, &global_context.context, &json, type1));

    struct kefir_ast_struct_type *struct_type2 = NULL;
    const struct kefir_ast_type *type2 = kefir_ast_type_structure(mem, &type_bundle, "struct2", &struct_type2);
    REQUIRE_OK(kefir_ast_struct_type_field(mem, &symbols, struct_type2, "a", kefir_ast_type_char(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(
        mem, &symbols, struct_type2, "z",
        kefir_ast_type_unbounded_array(mem, &type_bundle, kefir_ast_type_signed_short(), NULL),
        kefir_ast_alignment_const_expression(mem, kefir_ast_constant_expression_integer(mem, 8))));
    REQUIRE_OK(dump_type(mem, &global_context.context, &json, type2));

    struct kefir_ast_struct_type *struct_type3 = NULL;
    const struct kefir_ast_type *type3 = kefir_ast_type_structure(mem, &type_bundle, "struct3", &struct_type3);
    REQUIRE_OK(kefir_ast_struct_type_field(mem, &symbols, struct_type3, "internal1", kefir_ast_type_char(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(
        mem, &symbols, struct_type3, "internal2",
        kefir_ast_type_unbounded_array(mem, &type_bundle, kefir_ast_type_float(), NULL), NULL));

    struct kefir_ast_struct_type *struct_type4 = NULL;
    const struct kefir_ast_type *type4 = kefir_ast_type_structure(mem, &type_bundle, "struct4", &struct_type4);
    REQUIRE_OK(kefir_ast_struct_type_field(mem, &symbols, struct_type4, "outer1", kefir_ast_type_unsigned_int(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(mem, &symbols, struct_type4, NULL, type3, NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(
        mem, &symbols, struct_type4, "outer2",
        kefir_ast_type_unbounded_array(mem, &type_bundle, kefir_ast_type_signed_char(), NULL), NULL));
    REQUIRE_OK(dump_type(mem, &global_context.context, &json, type4));

    REQUIRE_OK(kefir_json_output_array_end(&json));
    REQUIRE_OK(kefir_json_output_finalize(&json));

    REQUIRE_OK(kefir_ast_global_context_free(mem, &global_context));
    REQUIRE_OK(kefir_ast_type_bundle_free(mem, &type_bundle));
    REQUIRE_OK(kefir_string_pool_free(mem, &symbols));
    return KEFIR_OK;
}
