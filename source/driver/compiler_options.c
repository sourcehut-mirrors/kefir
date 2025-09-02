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

#include "kefir/driver/compiler_options.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

#define MEMBERSZ(structure, field) (sizeof(((structure *) NULL)->field))

static kefir_result_t preprocess_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                      const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    UNUSED(arg);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    REQUIRE(!options->skip_preprocessor,
            KEFIR_SET_ERRORF(KEFIR_UI_ERROR, "Cannot combine %s with skipping preprocessor", option->long_option));
    return KEFIR_OK;
}

static kefir_result_t pp_timestamp_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                        const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    UNUSED(arg);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    options->default_pp_timestamp = false;
    return KEFIR_OK;
}

static kefir_result_t define_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                  const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    const char *iter = arg;
    while (*iter != '\0' && *iter != '=') {
        ++iter;
    }

    char macro_identifier[4097];
    kefir_size_t identifier_length = MIN((kefir_size_t) (iter - arg), sizeof(macro_identifier) - 1);
    strncpy(macro_identifier, arg, identifier_length);
    macro_identifier[identifier_length] = '\0';
    const char *value = *iter != '\0' ? iter + 1 : NULL;
    REQUIRE_OK(kefir_compiler_runner_configuration_define(mem, options, macro_identifier, value));
    return KEFIR_OK;
}

static kefir_result_t undefine_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                    const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    const char *symbol = kefir_string_pool_insert(mem, symbols, arg, NULL);
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into string pool"));

    REQUIRE_OK(
        kefir_list_insert_after(mem, &options->undefines, kefir_list_tail(&options->undefines), (void *) symbol));
    if (kefir_hashtree_has(&options->defines, (kefir_hashtree_key_t) symbol)) {
        REQUIRE_OK(kefir_hashtree_delete(mem, &options->defines, (kefir_hashtree_key_t) symbol));
    }
    return KEFIR_OK;
}

static kefir_result_t include_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                   const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    const char *symbol = kefir_string_pool_insert(mem, symbols, arg, NULL);
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into string pool"));

    REQUIRE_OK(
        kefir_list_insert_after(mem, &options->include_path, kefir_list_tail(&options->include_path), (void *) symbol));
    return KEFIR_OK;
}

static kefir_result_t sys_include_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                       const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    const char *symbol = kefir_string_pool_insert(mem, symbols, arg, NULL);
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into string pool"));

    REQUIRE_OK(
        kefir_list_insert_after(mem, &options->include_path, kefir_list_tail(&options->include_path), (void *) symbol));
    REQUIRE_OK(kefir_hashtreeset_add(mem, &options->system_include_directories, (kefir_hashtreeset_entry_t) symbol));
    return KEFIR_OK;
}

static kefir_result_t quote_include_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                         const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    const char *symbol = kefir_string_pool_insert(mem, symbols, arg, NULL);
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into string pool"));

    REQUIRE_OK(
        kefir_list_insert_after(mem, &options->include_path, kefir_list_tail(&options->include_path), (void *) symbol));
    REQUIRE_OK(kefir_hashtreeset_add(mem, &options->quote_include_directories, (kefir_hashtreeset_entry_t) symbol));
    return KEFIR_OK;
}

static kefir_result_t embed_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                 const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    const char *symbol = kefir_string_pool_insert(mem, symbols, arg, NULL);
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into string pool"));

    REQUIRE_OK(
        kefir_list_insert_after(mem, &options->embed_path, kefir_list_tail(&options->embed_path), (void *) symbol));
    return KEFIR_OK;
}

static kefir_result_t include_file_hook(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                        const struct kefir_cli_option *option, void *raw_options, const char *arg) {
    UNUSED(mem);
    UNUSED(symbols);
    UNUSED(option);
    ASSIGN_DECL_CAST(struct kefir_compiler_runner_configuration *, options, raw_options);

    const char *symbol = kefir_string_pool_insert(mem, symbols, arg, NULL);
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into string pool"));

    REQUIRE_OK(kefir_list_insert_after(mem, &options->include_files, kefir_list_tail(&options->include_files),
                                       (void *) symbol));
    return KEFIR_OK;
}

struct kefir_cli_option KefirCompilerConfigurationOptions[] = {
#define SIMPLE(short, long, has_arg, action, action_param, field) \
    {short,                                                       \
     long,                                                        \
     has_arg,                                                     \
     action,                                                      \
     action_param,                                                \
     offsetof(struct kefir_compiler_runner_configuration, field), \
     MEMBERSZ(struct kefir_compiler_runner_configuration, field), \
     NULL,                                                        \
     NULL,                                                        \
     NULL}
#define PREHOOK(short, long, has_arg, action, action_param, field, hook) \
    {short,                                                              \
     long,                                                               \
     has_arg,                                                            \
     action,                                                             \
     action_param,                                                       \
     offsetof(struct kefir_compiler_runner_configuration, field),        \
     MEMBERSZ(struct kefir_compiler_runner_configuration, field),        \
     hook,                                                               \
     NULL,                                                               \
     NULL}
#define POSTHOOK(short, long, has_arg, action, action_param, field, hook) \
    {short,                                                               \
     long,                                                                \
     has_arg,                                                             \
     action,                                                              \
     action_param,                                                        \
     offsetof(struct kefir_compiler_runner_configuration, field),         \
     MEMBERSZ(struct kefir_compiler_runner_configuration, field),         \
     NULL,                                                                \
     hook,                                                                \
     NULL}
#define CUSTOM(short, long, has_arg, hook) \
    {short, long, has_arg, KEFIR_CLI_OPTION_ACTION_NONE, 0, 0, 0, hook, NULL, NULL}
#define FEATURE(name, field)                                                                 \
    SIMPLE(0, "feature-" name, false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true, field), \
        SIMPLE(0, "no-feature-" name, false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false, field)
#define INTERNAL(name, field)                                                                 \
    SIMPLE(0, "internal-" name, false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true, field), \
        SIMPLE(0, "no-internal-" name, false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false, field)
#define CODEGEN(name, field)                                                                 \
    SIMPLE(0, "codegen-" name, false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true, field), \
        SIMPLE(0, "no-codegen-" name, false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false, field)

    SIMPLE('o', "output", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, output_filepath),
    SIMPLE(0, "c17-standard", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_C17_STANDARD_VERSION,
           standard_version),
    SIMPLE(0, "c23-standard", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_C23_STANDARD_VERSION,
           standard_version),
    PREHOOK('p', "preprocess", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_COMPILER_RUNNER_ACTION_PREPROCESS,
            action, preprocess_hook),
    SIMPLE('P', "skip-preprocessor", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true, skip_preprocessor),
    SIMPLE(0, "dump-tokens", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_COMPILER_RUNNER_ACTION_DUMP_TOKENS,
           action),
    SIMPLE(0, "dump-ast", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_COMPILER_RUNNER_ACTION_DUMP_AST,
           action),
    SIMPLE(0, "dump-ir", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_COMPILER_RUNNER_ACTION_DUMP_IR, action),
    SIMPLE(0, "dump-opt", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_COMPILER_RUNNER_ACTION_DUMP_OPT,
           action),
    SIMPLE(0, "json-errors", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_COMPILER_RUNNER_ERROR_REPORT_JSON,
           error_report_type),
    SIMPLE(0, "tabular-errors", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT,
           KEFIR_COMPILER_RUNNER_ERROR_REPORT_TABULAR, error_report_type),
    SIMPLE(0, "target-profile", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, target_profile),
    SIMPLE(0, "source-id", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, source_id),
    SIMPLE(0, "debug-info", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true, debug_info),
    SIMPLE(0, "no-debug-info", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false, debug_info),
    POSTHOOK(0, "pp-timestamp", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_UINTARG, 0, pp_timestamp, pp_timestamp_hook),
    SIMPLE(0, "optimizer-pipeline", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, optimizer_pipeline_spec),
    SIMPLE(0, "dump-dependencies", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true,
           dependency_output.output_dependencies),
    SIMPLE(0, "system-dependencies", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true,
           dependency_output.output_system_deps),
    SIMPLE(0, "no-system-dependencies", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false,
           dependency_output.output_system_deps),
    SIMPLE(0, "add-phony-targets", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true,
           dependency_output.add_phony_targets),
    SIMPLE(0, "no-add-phony-targets", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false,
           dependency_output.add_phony_targets),
    SIMPLE(0, "dependency-target", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, dependency_output.target_name),
    SIMPLE(0, "dependency-output", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, dependency_output.output_filename),
    SIMPLE(0, "extension-lib", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, extension_lib),

    SIMPLE(0, "unsigned-char", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_COMPILER_PROFILE_CHAR_UNSIGNED,
           target_profile_config.char_signedness),
    SIMPLE(0, "signed-char", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_COMPILER_PROFILE_CHAR_SIGNED,
           target_profile_config.char_signedness),

    SIMPLE(0, "enable-lowering", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false, optimizer.disable_lowering),
    SIMPLE(0, "disable-lowering", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true, optimizer.disable_lowering),

    CUSTOM('D', "define", true, define_hook),
    CUSTOM('U', "undefine", true, undefine_hook),
    CUSTOM('I', "include-dir", true, include_hook),
    CUSTOM(0, "system-include-dir", true, sys_include_hook),
    CUSTOM(0, "quote-include-dir", true, quote_include_hook),
    CUSTOM(0, "embed-dir", true, embed_hook),
    CUSTOM(0, "include", true, include_file_hook),

    SIMPLE(0, "optimizer-max-inline-depth", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_UINTARG, 0,
           optimizer.max_inline_depth),
    SIMPLE(0, "optimizer-max-inlines-per-func", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_UINTARG, 0,
           optimizer.max_inlines_per_function),

    FEATURE("non-strict-qualifiers", features.non_strict_qualifiers),
    FEATURE("signed-enums", features.signed_enum_type),
    FEATURE("implicit-function-decl", features.implicit_function_declaration),
    FEATURE("fail-on-attributes", features.fail_on_attributes),
    FEATURE("missing-function-return-type", features.missing_function_return_type),
    FEATURE("designated-init-colons", features.designated_initializer_colons),
    FEATURE("labels-as-values", features.labels_as_values),
    FEATURE("empty-structs", features.empty_structs),
    FEATURE("ext-pointer-arithmetics", features.ext_pointer_arithmetics),
    FEATURE("missing-braces-subobj", features.missing_braces_subobject),
    FEATURE("statement-expressions", features.statement_expressions),
    FEATURE("omitted-conditional-operand", features.omitted_conditional_operand),
    FEATURE("int-to-pointer", features.int_to_pointer),
    FEATURE("permissive-pointer-conv", features.permissive_pointer_conv),
    FEATURE("named-macro-vararg", features.named_macro_vararg),
    FEATURE("include-next", features.include_next),
    FEATURE("fail-on-assembly", features.fail_on_assembly),
    FEATURE("va-args-comma-concat", features.va_args_concat),
    FEATURE("switch-case-ranges", features.switch_case_ranges),
    FEATURE("designator-subscript-ranges", features.designator_subscript_ranges),

    SIMPLE(0, "preprocessor-assembly-mode", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true,
           preprocessor_assembly_mode),
    SIMPLE(0, "preprocessor-normal-mode", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false,
           preprocessor_assembly_mode),

    SIMPLE(0, "precise-bitfield-load-store", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true,
           features.precise_bitfield_load_store),
    SIMPLE(0, "no-precise-bitfield-load-store", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false,
           features.precise_bitfield_load_store),
    SIMPLE(0, "declare-atomic-support", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true,
           features.declare_atomic_support),
    SIMPLE(0, "no-declare-atomic-support", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false,
           features.declare_atomic_support),
    SIMPLE(0, "optimize-stack-frame", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, true,
           features.optimize_stack_frame),
    SIMPLE(0, "no-optimize-stack-frame", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, false,
           features.optimize_stack_frame),

    CODEGEN("emulated-tls", codegen.emulated_tls),
    CODEGEN("tls-common", codegen.tls_common),
    CODEGEN("pic", codegen.position_independent_code),
    CODEGEN("omit-frame-pointer", codegen.omit_frame_pointer),
    CODEGEN("valgrind-compatible-x87", codegen.valgrind_compatible_x87),
    SIMPLE(0, "codegen-tentative-common", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_AST_CONTEXT_TENTATIVE_DEFINITION_PLACEMENT_COMMON,
           codegen.tentative_definition_placement),
    SIMPLE(0, "codegen-tentative-no-common", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_AST_CONTEXT_TENTATIVE_DEFINITION_PLACEMENT_NO_COMMON,
           codegen.tentative_definition_placement),
    SIMPLE(0, "codegen-visibility-default", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_AST_DECLARATOR_VISIBILITY_DEFAULT,
           codegen.symbol_visibility),
    SIMPLE(0, "codegen-visibility-protected", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_AST_DECLARATOR_VISIBILITY_PROTECTED,
           codegen.symbol_visibility),
    SIMPLE(0, "codegen-visibility-hidden", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_AST_DECLARATOR_VISIBILITY_HIDDEN,
           codegen.symbol_visibility),
    SIMPLE(0, "codegen-visibility-internal", false, KEFIR_CLI_OPTION_ACTION_ASSIGN_CONSTANT, KEFIR_AST_DECLARATOR_VISIBILITY_INTERNAL,
           codegen.symbol_visibility),
    SIMPLE(0, "codegen-syntax", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, codegen.syntax),
    SIMPLE(0, "codegen-details", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, codegen.print_details),
    SIMPLE(0, "codegen-pipeline", true, KEFIR_CLI_OPTION_ACTION_ASSIGN_STRARG, 0, codegen.pipeline_spec)

#undef SIMPLE
#undef PREHOOK
#undef POSTHOOK
#undef CUSTOM
#undef INTERNAL
#undef FEATURE
#undef CODEGEN
};

const kefir_size_t KefirCompilerConfigurationOptionCount =
    sizeof(KefirCompilerConfigurationOptions) / sizeof(KefirCompilerConfigurationOptions[0]);
