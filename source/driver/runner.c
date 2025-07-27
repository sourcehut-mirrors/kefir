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

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <dlfcn.h>
#include "kefir/platform/input.h"
#include "kefir/platform/filesystem_source.h"
#include "kefir/core/util.h"
#include "kefir/compiler/compiler.h"
#include "kefir/core/os_error.h"
#include "kefir/core/error_format.h"
#include "kefir/compiler/configuration.h"
#include "kefir/lexer/format.h"
#include "kefir/ast/format.h"
#include "kefir/ir/format.h"
#include "kefir/preprocessor/format.h"
#include "kefir/preprocessor/source_dependency_locator.h"
#include "kefir/core/version.h"
#include "kefir/driver/runner.h"
#include "kefir/optimizer/module.h"
#include "kefir/optimizer/format.h"
#include "kefir/optimizer/analysis.h"
#include "kefir/parser/builtins.h"
#include "kefir/ast/analyzer/declarator.h"

// ATTENTION: This is module is not a part of the core library, thus memory management
//            is different here. While all the modules from core library shall correctly
//            handle memory deallocations in all cases (leaks, use-after-free, double free,
//            etc. are considered bugs), in main application part this rule is relaxed.
//            Specifically, correct memory deallocation is not deemed necessary, as it is
//            known that all the memory will be eventually deallocated by the OS. At the same time,
//            it is beneficially to correctly deallocate memory when there are no runtime errors:
//            it enables Valgrind use in end2end tests, thus increasing dynamic analysis coverage.
//            Based on this idea, code below is written with following assumptions:
//                - In case of happy-path, memory deallocations should happen correctly with no
//                  Valgrind warnings.
//                - In case of runtime errors, memory deallocations might be omitted. Valgrind
//                  warnings are considered normal.
//                - Other memory management issues (use-after-frees, double frees, etc.) are
//                  considered unacceptable and should be fixed.

static kefir_result_t open_output(const char *filepath, FILE **output) {
    if (filepath != NULL) {
        *output = fopen(filepath, "w");
        REQUIRE(*output != NULL, KEFIR_SET_OS_ERROR("Unable to open output file"));
    } else {
        *output = stdout;
    }
    return KEFIR_OK;
}

static kefir_result_t is_system_include_path(const char *path, kefir_bool_t *res, void *payload) {
    REQUIRE(res != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to flag"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(const struct kefir_compiler_runner_configuration *, options, payload);

    if (path == NULL) {
        *res = false;
        return KEFIR_OK;
    }

    *res = kefir_hashtreeset_has(&options->system_include_directories, (kefir_hashtreeset_entry_t) path);
    return KEFIR_OK;
}

static kefir_result_t output_dependencies(
    const struct kefir_compiler_runner_configuration *options,
    struct kefir_preprocessor_dependencies_source_locator *dependencies_source_locator, FILE *dependency_output) {
    const char *first_dep = options->input_filepath != NULL ? options->input_filepath : "";
    if (options->dependency_output.target_name != NULL) {
        fprintf(dependency_output, "%s: %s", options->dependency_output.target_name, first_dep);
    } else if (first_dep != NULL) {
        fprintf(dependency_output, "%s.o: %s", first_dep, first_dep);
    } else {
        fprintf(dependency_output, "a.out: %s", first_dep);
    }
    REQUIRE_OK(kefir_preprocessor_dependencies_source_locator_format_make_rule_prerequisites(
        dependencies_source_locator, options->dependency_output.output_system_deps, dependency_output));
    fprintf(dependency_output, "\n");
    if (options->dependency_output.add_phony_targets) {
        REQUIRE_OK(kefir_preprocessor_dependencies_source_locator_format_make_rule_phony_targets(
            dependencies_source_locator, options->dependency_output.output_system_deps, dependency_output));
    }
    fprintf(dependency_output, "\n");
    return KEFIR_OK;
}

static kefir_result_t load_extension_lib(struct kefir_mem *mem,
                                         const struct kefir_compiler_runner_configuration *options,
                                         const struct kefir_compiler_extensions **extensions, void **extension_lib) {
    if (options->extension_lib == NULL) {
        *extensions = NULL;
        *extension_lib = NULL;
        return KEFIR_OK;
    }

#ifndef KEFIR_EXTENSION_SUPPORT
    UNUSED(mem);
    UNUSED(extensions);
    UNUSED(extension_lib);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Extension library loading support has been disabled at compile time");
#else
    void *extlib = dlopen(options->extension_lib, RTLD_LAZY | RTLD_LOCAL);
    REQUIRE(extlib != NULL,
            KEFIR_SET_OS_ERRORF("Failed to load extension library %s: %s", options->extension_lib, dlerror()));

    *extension_lib = extlib;

    void *entry_sym = dlsym(extlib, KEFIR_COMPILER_EXTENSION_ENTRY);
    REQUIRE(entry_sym != NULL, KEFIR_SET_OS_ERRORF("Failed to locate entry point of extension library: %s", dlerror()));

    typedef kefir_result_t (*entry_fn_t)(struct kefir_mem *, const struct kefir_compiler_runner_configuration *,
                                         const struct kefir_compiler_extensions **);
    entry_fn_t entry_fn = *(entry_fn_t *) &entry_sym;
    REQUIRE_OK(entry_fn(mem, options, extensions));

    return KEFIR_OK;
#endif
}

static kefir_result_t unload_extension_lib(void *extlib) {
    if (extlib != NULL) {
        int res = dlclose(extlib);
        REQUIRE(res == 0, KEFIR_SET_OS_ERRORF("Failed to unload extension library: %s", dlerror()));
    }
    return KEFIR_OK;
}

static kefir_result_t dump_action_impl(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options,
                                       kefir_result_t (*action)(struct kefir_mem *,
                                                                const struct kefir_compiler_runner_configuration *,
                                                                struct kefir_compiler_context *, const char *,
                                                                const char *, kefir_size_t, FILE *)) {
    FILE *output;
    struct kefir_cli_input input;
    struct kefir_string_pool symbols;
    struct kefir_compiler_profile profile;
    struct kefir_compiler_context compiler;
    struct kefir_preprocessor_filesystem_source_locator filesystem_source_locator;
    struct kefir_preprocessor_dependencies_source_locator dependencies_source_locator;

    const char *source_id = NULL;
    if (options->source_id != NULL) {
        source_id = options->source_id;
    } else if (options->input_filepath != NULL) {
        source_id = options->input_filepath;
    } else {
        source_id = "<stdin>";
    }

    REQUIRE_OK(open_output(options->output_filepath, &output));
    REQUIRE_OK(kefir_cli_input_open(mem, &input, options->input_filepath, stdin));
    REQUIRE_OK(kefir_string_pool_init(&symbols));
    REQUIRE_OK(kefir_preprocessor_filesystem_source_locator_init(&filesystem_source_locator, &symbols));
    for (const struct kefir_list_entry *iter = kefir_list_head(&options->include_path); iter != NULL;
         kefir_list_next(&iter)) {
        REQUIRE_OK(kefir_preprocessor_filesystem_source_locator_append(
            mem, &filesystem_source_locator, (const char *) iter->value,
            kefir_hashtreeset_has(&options->quote_include_directories, (kefir_hashtreeset_entry_t) iter->value)));
    }
    for (const struct kefir_list_entry *iter = kefir_list_head(&options->embed_path); iter != NULL;
         kefir_list_next(&iter)) {
        REQUIRE_OK(kefir_preprocessor_filesystem_source_locator_append_embed(mem, &filesystem_source_locator,
                                                                             (const char *) iter->value));
    }
    struct kefir_preprocessor_source_locator *source_locator = &filesystem_source_locator.locator;

    if (options->dependency_output.output_dependencies) {
        REQUIRE_OK(kefir_preprocessor_dependencies_source_locator_init(source_locator, is_system_include_path,
                                                                       (void *) options, &dependencies_source_locator));
        source_locator = &dependencies_source_locator.locator;
    }

    void *extension_lib = NULL;
    const struct kefir_compiler_extensions *extensions = NULL;
    REQUIRE_OK(load_extension_lib(mem, options, &extensions, &extension_lib));

    REQUIRE_OK(kefir_compiler_profile(mem, &profile, options->target_profile, &options->target_profile_config));
    REQUIRE_OK(
        kefir_compiler_context_init(mem, &compiler, options->standard_version, &profile, source_locator, extensions));

    compiler.preprocessor_configuration.assembly_mode = options->preprocessor_assembly_mode;
    compiler.preprocessor_configuration.named_macro_vararg = options->features.named_macro_vararg;
    compiler.preprocessor_configuration.include_next = options->features.include_next;
    compiler.preprocessor_configuration.va_args_concat = options->features.va_args_concat;
    compiler.preprocessor_configuration.standard_version = options->standard_version;
    compiler.preprocessor_context.environment.stdc_no_atomics = !options->features.declare_atomic_support;
    for (const char **attribute = KEFIR_DECLARATOR_ANALYZER_SUPPORTED_GNU_ATTRIBUTES; *attribute != NULL; ++attribute) {
        REQUIRE_OK(kefir_hashtreeset_add(mem, &compiler.preprocessor_context.environment.supported_gnu_attributes,
                                         (kefir_hashtreeset_entry_t) *attribute));
    }
    for (const struct kefir_declarator_analyzer_std_attribute_descriptor *attribute =
             &KEFIR_DECLARATOR_ANALYZER_SUPPORTED_STD_ATTRIBUTES[0];
         attribute->attribute != NULL; ++attribute) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &compiler.preprocessor_context.environment.supported_std_attributes,
                                         (kefir_hashtree_key_t) attribute->attribute,
                                         (kefir_hashtree_value_t) attribute->version));
    }
    for (const char **builtin = KEFIR_PARSER_SUPPORTED_BUILTINS; *builtin != NULL; ++builtin) {
        REQUIRE_OK(kefir_hashtreeset_add(mem, &compiler.preprocessor_context.environment.supported_builtins,
                                         (kefir_hashtreeset_entry_t) *builtin));
    }

    compiler.parser_configuration.fail_on_attributes = options->features.fail_on_attributes;
    compiler.parser_configuration.implicit_function_definition_int = options->features.missing_function_return_type;
    compiler.parser_configuration.designated_initializer_colons = options->features.designated_initializer_colons;
    compiler.parser_configuration.label_addressing = options->features.labels_as_values;
    compiler.parser_configuration.statement_expressions = options->features.statement_expressions;
    compiler.parser_configuration.omitted_conditional_operand = options->features.omitted_conditional_operand;
    compiler.parser_configuration.fail_on_assembly = options->features.fail_on_assembly;
    compiler.parser_configuration.switch_case_ranges = options->features.switch_case_ranges;
    compiler.parser_configuration.designator_subscript_ranges = options->features.designator_subscript_ranges;

    compiler.ast_global_context.configuration.analysis.non_strict_qualifiers = options->features.non_strict_qualifiers;
    compiler.ast_global_context.configuration.analysis.fixed_enum_type = options->features.signed_enum_type;
    compiler.ast_global_context.configuration.analysis.implicit_function_declaration =
        options->features.implicit_function_declaration;
    compiler.ast_global_context.configuration.analysis.ext_pointer_arithmetics =
        options->features.ext_pointer_arithmetics;
    compiler.ast_global_context.configuration.analysis.missing_braces_subobj =
        options->features.missing_braces_subobject;
    compiler.ast_global_context.configuration.analysis.int_to_pointer = options->features.int_to_pointer;
    compiler.ast_global_context.configuration.analysis.permissive_pointer_conv =
        options->features.permissive_pointer_conv;
    compiler.ast_global_context.configuration.standard_version = options->standard_version;

    compiler.translator_configuration.empty_structs = options->features.empty_structs;
    compiler.translator_configuration.precise_bitfield_load_store = options->features.precise_bitfield_load_store;

    compiler.codegen_configuration.emulated_tls = options->codegen.emulated_tls;
    compiler.codegen_configuration.position_independent_code = options->codegen.position_independent_code;
    compiler.codegen_configuration.debug_info = options->debug_info;
    compiler.codegen_configuration.omit_frame_pointer = options->codegen.omit_frame_pointer;
    compiler.codegen_configuration.valgrind_compatible_x87 = options->codegen.valgrind_compatible_x87;
    compiler.codegen_configuration.syntax = options->codegen.syntax;
    compiler.codegen_configuration.print_details = options->codegen.print_details;
    compiler.codegen_configuration.pipeline_spec = options->codegen.pipeline_spec;

    compiler.optimizer_configuration.max_inline_depth = options->optimizer.max_inline_depth;
    compiler.optimizer_configuration.max_inlines_per_function = options->optimizer.max_inlines_per_function;
    if (options->optimizer_pipeline_spec != NULL) {
        char buf[256];
        const char *spec = options->optimizer_pipeline_spec;

        while (spec != NULL && *spec != '\0') {
            const char *next_spec = strchr(spec, ',');
            if (next_spec == NULL) {
                REQUIRE_OK(
                    kefir_optimizer_configuration_add_pipeline_pass(mem, &compiler.optimizer_configuration, spec));
                spec = NULL;
            } else if (next_spec - spec > 0) {
                size_t length = next_spec - spec;
                REQUIRE(length < sizeof(buf) - 1,
                        KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Optimizer pass specification element exceeds maximum length"));
                snprintf(buf, sizeof(buf), "%.*s", (int) length, spec);
                REQUIRE_OK(
                    kefir_optimizer_configuration_add_pipeline_pass(mem, &compiler.optimizer_configuration, buf));
                spec = next_spec + 1;
            } else {
                spec++;
            }
        }
    }

    FILE *stage_output = output;
    FILE *dependency_output = NULL;
    if (options->dependency_output.output_dependencies) {
        if (options->dependency_output.output_filename == NULL) {
            stage_output = NULL;
            dependency_output = output;
        } else {
            REQUIRE_OK(open_output(options->dependency_output.output_filename, &dependency_output));
        }
    }

    REQUIRE_OK(action(mem, options, &compiler, source_id, input.content, input.length, stage_output));
    REQUIRE_OK(kefir_compiler_context_free(mem, &compiler));
    REQUIRE_OK(unload_extension_lib(extension_lib));
    if (options->dependency_output.output_dependencies) {
        REQUIRE_OK(output_dependencies(options, &dependencies_source_locator, dependency_output));
        REQUIRE_OK(kefir_preprocessor_dependencies_source_locator_free(mem, &dependencies_source_locator));
    }
    if (stage_output != NULL) {
        fclose(stage_output);
    }
    if (dependency_output != NULL) {
        fclose(dependency_output);
    }
    REQUIRE_OK(kefir_preprocessor_filesystem_source_locator_free(mem, &filesystem_source_locator));
    REQUIRE_OK(kefir_string_pool_free(mem, &symbols));
    REQUIRE_OK(kefir_cli_input_close(mem, &input));
    return KEFIR_OK;
}

static kefir_result_t build_predefined_macros(struct kefir_mem *mem,
                                              const struct kefir_compiler_runner_configuration *options,
                                              struct kefir_compiler_context *compiler) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&options->undefines); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, identifier, iter->value);
        identifier = kefir_string_pool_insert(mem, &compiler->ast_global_context.symbols, identifier, NULL);
        REQUIRE(identifier != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert undefined macro into symbol table"));
        if (!kefir_hashtree_has(&compiler->preprocessor_context.undefined_macros, (kefir_hashtree_key_t) identifier)) {
            REQUIRE_OK(kefir_hashtree_insert(mem, &compiler->preprocessor_context.undefined_macros,
                                             (kefir_hashtree_key_t) identifier, (kefir_hashtree_value_t) 0));

            kefir_result_t res = kefir_preprocessor_user_macro_scope_remove(
                mem, &compiler->preprocessor_context.user_macros, identifier);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
            }
        }
    }

    struct kefir_hashtree_node_iterator macro_iter;
    const struct kefir_hashtree_node *macro_node = kefir_hashtree_iter(&options->defines, &macro_iter);
    for (; macro_node != NULL; macro_node = kefir_hashtree_next(&macro_iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, macro_node->key);
        ASSIGN_DECL_CAST(const char *, raw_value, macro_node->value);

        struct kefir_preprocessor_user_macro *macro =
            kefir_preprocessor_user_macro_new_object(mem, &compiler->ast_global_context.symbols, identifier);
        REQUIRE(macro != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate preprocessor macro"));
        kefir_result_t res = KEFIR_OK;

        if (raw_value != NULL) {
            res = kefir_compiler_preprocessor_tokenize(mem, compiler, &compiler->builtin_token_allocator,
                                                       &macro->replacement, raw_value, strlen(raw_value), identifier);
        } else {
            struct kefir_token *allocated_token;
            res = kefir_token_allocator_allocate_empty(mem, &compiler->builtin_token_allocator, &allocated_token);
            REQUIRE_CHAIN(&res, kefir_token_new_constant_int(1, allocated_token));
            REQUIRE_CHAIN(&res, kefir_token_buffer_emplace(mem, &macro->replacement, allocated_token));
        }
        REQUIRE_CHAIN(
            &res, kefir_preprocessor_user_macro_scope_insert(mem, &compiler->preprocessor_context.user_macros, macro));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_preprocessor_user_macro_free(mem, macro);
            return res;
        });
    }

    return KEFIR_OK;
}

static kefir_result_t include_predefined(struct kefir_mem *mem,
                                         const struct kefir_compiler_runner_configuration *options,
                                         struct kefir_compiler_context *compiler, const char *source_id,
                                         struct kefir_token_buffer *tokens) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&options->include_files); iter != NULL;
         kefir_list_next(&iter)) {
        REQUIRE_OK(kefir_compiler_preprocess_include(mem, compiler, &compiler->builtin_token_allocator, tokens,
                                                     source_id, (const char *) iter->value));
    }
    return KEFIR_OK;
}

static kefir_result_t lex_file(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options,
                               struct kefir_compiler_context *compiler, struct kefir_token_allocator *token_allocator,
                               const char *source_id, const char *source, kefir_size_t length,
                               struct kefir_token_buffer *tokens) {
    if (options->skip_preprocessor) {
        REQUIRE_OK(kefir_compiler_lex(mem, compiler, token_allocator, tokens, source, length, source_id));
    } else {
        REQUIRE_OK(build_predefined_macros(mem, options, compiler));
        if (!options->default_pp_timestamp) {
            compiler->preprocessor_context.environment.timestamp = options->pp_timestamp;
        }
        REQUIRE_OK(include_predefined(mem, options, compiler, source_id, tokens));
        REQUIRE_OK(kefir_compiler_preprocess_lex(mem, compiler, token_allocator, tokens, source, length, source_id,
                                                 options->input_filepath));
    }
    return KEFIR_OK;
}

static kefir_result_t dump_preprocessed_impl(struct kefir_mem *mem,
                                             const struct kefir_compiler_runner_configuration *options,
                                             struct kefir_compiler_context *compiler, const char *source_id,
                                             const char *source, kefir_size_t length, FILE *output) {
    UNUSED(options);
    struct kefir_token_buffer tokens;
    struct kefir_token_allocator token_allocator;
    REQUIRE_OK(kefir_token_buffer_init(&tokens));
    REQUIRE_OK(kefir_token_allocator_init(&token_allocator));
    REQUIRE_OK(build_predefined_macros(mem, options, compiler));
    REQUIRE_OK(include_predefined(mem, options, compiler, source_id, &tokens));
    REQUIRE_OK(kefir_compiler_preprocess(mem, compiler, &token_allocator, &tokens, source, length, source_id,
                                         options->input_filepath));
    if (output != NULL) {
        REQUIRE_OK(kefir_preprocessor_format(output, &tokens, KEFIR_PREPROCESSOR_WHITESPACE_FORMAT_ORIGINAL));
    }
    REQUIRE_OK(kefir_token_buffer_free(mem, &tokens));
    REQUIRE_OK(kefir_token_allocator_free(mem, &token_allocator));
    return KEFIR_OK;
}

static kefir_result_t action_dump_preprocessed(struct kefir_mem *mem,
                                               const struct kefir_compiler_runner_configuration *options) {
    REQUIRE_OK(dump_action_impl(mem, options, dump_preprocessed_impl));
    return KEFIR_OK;
}

static kefir_result_t dump_tokens_impl(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options,
                                       struct kefir_compiler_context *compiler, const char *source_id,
                                       const char *source, kefir_size_t length, FILE *output) {
    UNUSED(options);
    struct kefir_token_buffer tokens;
    struct kefir_token_allocator token_allocator;
    REQUIRE_OK(kefir_token_buffer_init(&tokens));
    REQUIRE_OK(kefir_token_allocator_init(&token_allocator));
    REQUIRE_OK(lex_file(mem, options, compiler, &token_allocator, source_id, source, length, &tokens));

    if (output != NULL) {
        struct kefir_json_output json;
        REQUIRE_OK(kefir_json_output_init(&json, output, 4));
        REQUIRE_OK(kefir_token_buffer_format(&json, &tokens, options->debug_info));
        REQUIRE_OK(kefir_json_output_finalize(&json));
    }
    REQUIRE_OK(kefir_token_buffer_free(mem, &tokens));
    REQUIRE_OK(kefir_token_allocator_free(mem, &token_allocator));
    return KEFIR_OK;
}

static kefir_result_t action_dump_tokens(struct kefir_mem *mem,
                                         const struct kefir_compiler_runner_configuration *options) {
    REQUIRE_OK(dump_action_impl(mem, options, dump_tokens_impl));
    return KEFIR_OK;
}

static kefir_result_t dump_ast_impl(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options,
                                    struct kefir_compiler_context *compiler, const char *source_id, const char *source,
                                    kefir_size_t length, FILE *output) {
    UNUSED(options);
    struct kefir_token_buffer tokens;
    struct kefir_token_allocator token_allocator;
    struct kefir_ast_translation_unit *unit = NULL;

    REQUIRE_OK(kefir_token_buffer_init(&tokens));
    REQUIRE_OK(kefir_token_allocator_init(&token_allocator));
    REQUIRE_OK(lex_file(mem, options, compiler, &token_allocator, source_id, source, length, &tokens));
    REQUIRE_OK(kefir_compiler_parse(mem, compiler, &tokens, &unit));
    REQUIRE_OK(kefir_compiler_analyze(mem, compiler, KEFIR_AST_NODE_BASE(unit)));

    REQUIRE_OK(kefir_token_buffer_free(mem, &tokens));
    REQUIRE_OK(kefir_token_allocator_free(mem, &token_allocator));

    if (output != NULL) {
        struct kefir_json_output json;
        REQUIRE_OK(kefir_json_output_init(&json, output, 4));
        REQUIRE_OK(kefir_ast_format(&json, KEFIR_AST_NODE_BASE(unit), options->debug_info));
        REQUIRE_OK(kefir_json_output_finalize(&json));
    }

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(unit)));
    return KEFIR_OK;
}

static kefir_result_t action_dump_ast(struct kefir_mem *mem,
                                      const struct kefir_compiler_runner_configuration *options) {
    REQUIRE_OK(dump_action_impl(mem, options, dump_ast_impl));
    return KEFIR_OK;
}

static kefir_result_t dump_ir_impl(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options,
                                   struct kefir_compiler_context *compiler, const char *source_id, const char *source,
                                   kefir_size_t length, FILE *output) {
    UNUSED(options);
    struct kefir_token_buffer tokens;
    struct kefir_token_allocator token_allocator;
    struct kefir_ast_translation_unit *unit = NULL;
    struct kefir_ir_module module;

    REQUIRE_OK(kefir_token_buffer_init(&tokens));
    REQUIRE_OK(kefir_token_allocator_init(&token_allocator));
    REQUIRE_OK(lex_file(mem, options, compiler, &token_allocator, source_id, source, length, &tokens));
    REQUIRE_OK(kefir_compiler_parse(mem, compiler, &tokens, &unit));
    REQUIRE_OK(kefir_compiler_analyze(mem, compiler, KEFIR_AST_NODE_BASE(unit)));

    REQUIRE_OK(kefir_token_buffer_free(mem, &tokens));
    REQUIRE_OK(kefir_token_allocator_free(mem, &token_allocator));

    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));
    REQUIRE_OK(kefir_compiler_translate(mem, compiler, unit, &module, true));

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(unit)));

    if (output != NULL) {
        REQUIRE_OK(kefir_ir_format_module(output, &module, options->debug_info));
    }

    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}

static kefir_result_t action_dump_ir(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options) {
    REQUIRE_OK(dump_action_impl(mem, options, dump_ir_impl));
    return KEFIR_OK;
}

static kefir_result_t dump_opt_impl(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options,
                                    struct kefir_compiler_context *compiler, const char *source_id, const char *source,
                                    kefir_size_t length, FILE *output) {
    UNUSED(options);
    struct kefir_token_buffer tokens;
    struct kefir_token_allocator token_allocator;
    struct kefir_ast_translation_unit *unit = NULL;
    struct kefir_ir_module module;
    struct kefir_opt_module opt_module;
    struct kefir_opt_module_analysis opt_analysis;

    REQUIRE_OK(kefir_token_buffer_init(&tokens));
    REQUIRE_OK(kefir_token_allocator_init(&token_allocator));
    REQUIRE_OK(lex_file(mem, options, compiler, &token_allocator, source_id, source, length, &tokens));
    REQUIRE_OK(kefir_compiler_parse(mem, compiler, &tokens, &unit));
    REQUIRE_OK(kefir_compiler_analyze(mem, compiler, KEFIR_AST_NODE_BASE(unit)));

    REQUIRE_OK(kefir_token_buffer_free(mem, &tokens));
    REQUIRE_OK(kefir_token_allocator_free(mem, &token_allocator));

    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));
    REQUIRE_OK(kefir_compiler_translate(mem, compiler, unit, &module, true));

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(unit)));

    REQUIRE_OK(kefir_opt_module_init(mem, &module, &opt_module));
    REQUIRE_OK(kefir_compiler_optimize(mem, compiler, &module, &opt_module));
    REQUIRE_OK(kefir_opt_module_analyze(mem, &opt_module, &opt_analysis));

    if (output != NULL) {
        struct kefir_json_output json;
        REQUIRE_OK(kefir_json_output_init(&json, output, 4));
        REQUIRE_OK(kefir_opt_module_format(mem, &json, &opt_module, &opt_analysis, options->debug_info));
        REQUIRE_OK(kefir_json_output_finalize(&json));
    }

    REQUIRE_OK(kefir_opt_module_analysis_free(mem, &opt_analysis));
    REQUIRE_OK(kefir_opt_module_free(mem, &opt_module));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}

static kefir_result_t action_dump_opt(struct kefir_mem *mem,
                                      const struct kefir_compiler_runner_configuration *options) {
    REQUIRE_OK(dump_action_impl(mem, options, dump_opt_impl));
    return KEFIR_OK;
}

static kefir_result_t dump_asm_impl(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options,
                                    struct kefir_compiler_context *compiler, const char *source_id, const char *source,
                                    kefir_size_t length, FILE *output) {
    UNUSED(options);
    struct kefir_token_buffer tokens;
    struct kefir_token_allocator token_allocator;
    struct kefir_ast_translation_unit *unit = NULL;
    struct kefir_ir_module module;
    struct kefir_opt_module opt_module;

    REQUIRE_OK(kefir_token_buffer_init(&tokens));
    REQUIRE_OK(kefir_token_allocator_init(&token_allocator));
    REQUIRE_OK(lex_file(mem, options, compiler, &token_allocator, source_id, source, length, &tokens));
    REQUIRE_OK(kefir_compiler_parse(mem, compiler, &tokens, &unit));
    REQUIRE_OK(kefir_compiler_analyze(mem, compiler, KEFIR_AST_NODE_BASE(unit)));

    REQUIRE_OK(kefir_token_buffer_free(mem, &tokens));
    REQUIRE_OK(kefir_token_allocator_free(mem, &token_allocator));

    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));
    REQUIRE_OK(kefir_compiler_translate(mem, compiler, unit, &module, false));

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(unit)));

    REQUIRE_OK(kefir_opt_module_init(mem, &module, &opt_module));
    if (compiler->profile->optimizer_enabled) {
        REQUIRE_OK(kefir_compiler_optimize(mem, compiler, &module, &opt_module));
        if (output != NULL) {
            REQUIRE_OK(kefir_compiler_codegen_optimized(mem, compiler, &opt_module, output));
        }
    } else if (output != NULL) {
        REQUIRE_OK(kefir_compiler_codegen(mem, compiler, &module, output));
    }

    REQUIRE_OK(kefir_opt_module_free(mem, &opt_module));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}

static kefir_result_t action_dump_asm(struct kefir_mem *mem,
                                      const struct kefir_compiler_runner_configuration *options) {
    REQUIRE_OK(dump_action_impl(mem, options, dump_asm_impl));
    return KEFIR_OK;
}

static kefir_result_t (*Actions[])(struct kefir_mem *, const struct kefir_compiler_runner_configuration *) = {
    [KEFIR_COMPILER_RUNNER_ACTION_PREPROCESS] = action_dump_preprocessed,
    [KEFIR_COMPILER_RUNNER_ACTION_DUMP_TOKENS] = action_dump_tokens,
    [KEFIR_COMPILER_RUNNER_ACTION_DUMP_AST] = action_dump_ast,
    [KEFIR_COMPILER_RUNNER_ACTION_DUMP_IR] = action_dump_ir,
    [KEFIR_COMPILER_RUNNER_ACTION_DUMP_OPT] = action_dump_opt,
    [KEFIR_COMPILER_RUNNER_ACTION_DUMP_ASSEMBLY] = action_dump_asm};

kefir_result_t kefir_run_compiler(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *options) {
    REQUIRE_OK(Actions[options->action](mem, options));
    return KEFIR_OK;
}

kefir_bool_t kefir_report_error(FILE *output, kefir_result_t res, kefir_bool_t print_json) {
    if (res == KEFIR_OK) {
        return true;
    } else {
        if (!print_json) {
            fprintf(output, "Failed to compile! Error stack:\n");
            kefir_format_error_tabular(output, kefir_current_error());
        } else {
            kefir_format_error_json(output, kefir_current_error());
        }
        return false;
    }
}
