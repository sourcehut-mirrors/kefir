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

#include "kefir/compiler/compiler.h"
#include "kefir/codegen/codegen.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static const char KefirCodegenBigintRuntime[] = {
#include STRINGIFY(KEFIR_COMPILER_RUNTIME_KEFIR_BIGINT_INCLUDE)
};
static kefir_uint64_t KefirCodegenBigintRuntimeLength = sizeof(KefirCodegenBigintRuntime);

static kefir_result_t source_locator_open(struct kefir_mem *mem,
                                          const struct kefir_preprocessor_source_locator *locator,
                                          const char *include_file, kefir_bool_t system,
                                          const struct kefir_preprocessor_source_file_info *source_file_info,
                                          kefir_preprocessor_source_locator_mode_t mode,
                                          struct kefir_preprocessor_source_file *source_file) {
    UNUSED(mem);
    UNUSED(locator);
    UNUSED(include_file);
    UNUSED(system);
    UNUSED(source_file_info);
    UNUSED(mode);
    UNUSED(source_file);

    return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Include files are not permitted during runtime processing");
}

static kefir_result_t mark_used_functions(struct kefir_ir_module *module, const struct kefir_hashtreeset *functions) {
    UNUSED(functions);

    kefir_result_t res;
    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(functions, &iter); res == KEFIR_OK; res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, function_name, iter.entry);
        REQUIRE_OK(kefir_ir_module_mark_function_used(module, function_name));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t generate_runtime_functions_impl(struct kefir_mem *mem, FILE *output,
                                                      const struct kefir_hashtreeset *functions,
                                                      struct kefir_compiler_context *context) {
    const char *filename = "<bigint-runtime>";

    struct kefir_token_buffer buffer;
    struct kefir_ast_translation_unit *defs_unit;
    struct kefir_ir_module ir_module;
    struct kefir_opt_module opt_module;
    REQUIRE_OK(kefir_token_buffer_init(&buffer));

    kefir_result_t res =
        kefir_compiler_preprocess_lex(mem, context, &context->builtin_token_allocator, &buffer,
                                      KefirCodegenBigintRuntime, KefirCodegenBigintRuntimeLength, filename, filename);
    REQUIRE_CHAIN(&res, kefir_compiler_parse(mem, context, &buffer, &defs_unit));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_buffer_free(mem, &buffer);
        return res;
    });

    res = kefir_token_buffer_free(mem, &buffer);
    REQUIRE_CHAIN(&res, kefir_compiler_analyze(mem, context, KEFIR_AST_NODE_BASE(defs_unit)));
    REQUIRE_CHAIN(&res, kefir_ir_module_alloc(mem, &ir_module));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(defs_unit));
        return res;
    });

    res = kefir_compiler_translate(mem, context, defs_unit, &ir_module, false);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_module_free(mem, &ir_module);
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(defs_unit));
        return res;
    });
    res = KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(defs_unit));
    REQUIRE_CHAIN(&res, mark_used_functions(&ir_module, functions));
    REQUIRE_CHAIN(&res, kefir_opt_module_init(mem, &ir_module, &opt_module));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_module_free(mem, &ir_module);
        return res;
    });

    res = kefir_compiler_optimize(mem, context, &ir_module, &opt_module);
    REQUIRE_CHAIN(&res, kefir_compiler_codegen_optimized(mem, context, &opt_module, output));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_module_free(mem, &opt_module);
        kefir_ir_module_free(mem, &ir_module);
        return res;
    });

    res = kefir_opt_module_free(mem, &opt_module);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_module_free(mem, &ir_module);
        return res;
    });
    REQUIRE_OK(kefir_ir_module_free(mem, &ir_module));

    return KEFIR_OK;
}

static kefir_result_t generate_runtime_functions(struct kefir_mem *mem, FILE *output,
                                                 const struct kefir_hashtreeset *functions, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output file"));
    REQUIRE(functions != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid set of runtime functions"));
    ASSIGN_DECL_CAST(const struct kefir_compiler_codegen_runtime_hooks *, hooks, payload);
    REQUIRE(hooks != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler runtime hooks"));

    struct kefir_preprocessor_source_locator source_locator;
    source_locator.open = source_locator_open;
    source_locator.payload = NULL;

    struct kefir_compiler_profile profile = *hooks->compiler_context->profile;
    profile.runtime_hooks_enabled = false;
    struct kefir_compiler_context context;
    REQUIRE_OK(kefir_compiler_context_init(mem, &context, &profile, &source_locator, NULL));

    context.codegen_configuration = hooks->compiler_context->codegen_configuration;
    context.codegen_configuration.debug_info = false;
    context.codegen_configuration.runtime_function_generator_mode = true;

    kefir_result_t res = kefir_optimizer_configuration_copy_from(mem, &context.optimizer_configuration,
                                                                 &hooks->compiler_context->optimizer_configuration);
    REQUIRE_CHAIN(&res, generate_runtime_functions_impl(mem, output, functions, &context));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_compiler_context_free(mem, &context);
        return res;
    });
    REQUIRE_OK(kefir_compiler_context_free(mem, &context));
    return KEFIR_OK;
}

kefir_result_t kefir_compiler_init_runtime_hooks(const struct kefir_compiler_context *context,
                                                 struct kefir_compiler_codegen_runtime_hooks *hooks) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler context"));
    REQUIRE(hooks != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to code generator runtime hooks"));

    hooks->compiler_context = context;
    hooks->hooks.generate_runtime_functions = generate_runtime_functions;
    hooks->hooks.payload = hooks;
    return KEFIR_OK;
}
