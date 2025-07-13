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

#include "kefir/driver/tools.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/string_array.h"
#include "kefir/platform/process.h"
#include "kefir/driver/runner.h"
#include "kefir/compiler/compiler.h"
#include <stdio.h>
#include <signal.h>

static kefir_result_t output_compiler_config(FILE *output,
                                             const struct kefir_compiler_runner_configuration *configuration) {
    fprintf(output, "+ kefir -cc1");
    switch (configuration->standard_version) {
        case KEFIR_C17_STANDARD_VERSION:
            fprintf(output, "--c17-standard");
            break;

        case KEFIR_C23_STANDARD_VERSION:
            fprintf(output, "--c23-standard");
            break;
    }
    if (configuration->dependency_output.output_dependencies) {
        fprintf(output, " --dump-dependencies");
        if (configuration->dependency_output.output_system_deps) {
            fprintf(output, " --system-dependencies");
        } else {
            fprintf(output, " --no-system-dependencies");
        }
        if (configuration->dependency_output.add_phony_targets) {
            fprintf(output, " --add-phony-targets");
        } else {
            fprintf(output, " --no-add-phony-targets");
        }
        if (configuration->dependency_output.target_name != NULL) {
            fprintf(output, " --dependency-target %s", configuration->dependency_output.target_name);
        }
        if (configuration->dependency_output.output_filename != NULL) {
            fprintf(output, " --dependency-output %s", configuration->dependency_output.output_filename);
        }
    }
    switch (configuration->action) {
        case KEFIR_COMPILER_RUNNER_ACTION_PREPROCESS:
            fprintf(output, " --preprocess");
            break;

        case KEFIR_COMPILER_RUNNER_ACTION_DUMP_TOKENS:
            fprintf(output, " --dump-tokens");
            break;

        case KEFIR_COMPILER_RUNNER_ACTION_DUMP_AST:
            fprintf(output, " --dump-ast");
            break;

        case KEFIR_COMPILER_RUNNER_ACTION_DUMP_IR:
            fprintf(output, " --dump-ir");
            break;

        case KEFIR_COMPILER_RUNNER_ACTION_DUMP_OPT:
            fprintf(output, " --dump-opt");
            break;

        case KEFIR_COMPILER_RUNNER_ACTION_DUMP_ASSEMBLY:
            // Intentionally left blank
            break;
    }

    if (configuration->input_filepath != NULL) {
        fprintf(output, " %s", configuration->input_filepath);
    }
    if (configuration->output_filepath != NULL) {
        fprintf(output, " -o %s", configuration->output_filepath);
    }
    if (configuration->target_profile != NULL) {
        fprintf(output, " --target-profile %s", configuration->target_profile);
    }
    if (configuration->source_id != NULL) {
        fprintf(output, " --source-id %s", configuration->source_id);
    }

    switch (configuration->error_report_type) {
        case KEFIR_COMPILER_RUNNER_ERROR_REPORT_TABULAR:
            fprintf(output, " --tabular-errors");
            break;

        case KEFIR_COMPILER_RUNNER_ERROR_REPORT_JSON:
            fprintf(output, " --json-errors");
            break;
    }

    if (configuration->debug_info) {
        fprintf(output, " --debug-info");
    }
    if (configuration->skip_preprocessor) {
        fprintf(output, " --skip-preprocessor");
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&configuration->include_path); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, str, iter->value);
        if (kefir_hashtreeset_has(&configuration->system_include_directories, (kefir_hashtreeset_entry_t) str)) {
            fprintf(output, " --system-include-dir %s", str);
        } else if (kefir_hashtreeset_has(&configuration->quote_include_directories, (kefir_hashtreeset_entry_t) str)) {
            fprintf(output, " --quote-include-dir %s", str);
        } else {
            fprintf(output, " --include-dir %s", str);
        }
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&configuration->include_files); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, str, iter->value);
        fprintf(output, " --include %s", str);
    }

    struct kefir_hashtree_node_iterator iter2;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&configuration->defines, &iter2); node != NULL;
         node = kefir_hashtree_next(&iter2)) {
        ASSIGN_DECL_CAST(const char *, str, node->key);
        ASSIGN_DECL_CAST(const char *, str2, node->value);
        if (str2 != NULL) {
            fprintf(output, " --define %s=%s", str, str2);
        } else {
            fprintf(output, " --define %s", str);
        }
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&configuration->undefines); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, str, iter->value);
        fprintf(output, " --undefine %s", str);
    }

    if (!configuration->default_pp_timestamp) {
        fprintf(output, " --pp-timestamp %ld", (long) configuration->pp_timestamp);
    }

    if (configuration->optimizer_pipeline_spec != NULL) {
        fprintf(output, " --optimizer-pipeline %s", configuration->optimizer_pipeline_spec);
    }

    if (configuration->features.declare_atomic_support) {
        fprintf(output, " --declare-atomic-support");
    } else {
        fprintf(output, " --no-declare-atomic-support");
    }

#define FEATURE(_id, _name)                         \
    if (configuration->features._id) {              \
        fprintf(output, " --feature-%s", _name);    \
    } else {                                        \
        fprintf(output, " --no-feature-%s", _name); \
    }

    FEATURE(fail_on_attributes, "fail-on-attributes")
    FEATURE(missing_function_return_type, "missing-function-return-type")
    FEATURE(designated_initializer_colons, "designated-init-colons")
    FEATURE(labels_as_values, "labels-as-values")
    FEATURE(non_strict_qualifiers, "non-strict-qualifiers")
    FEATURE(signed_enum_type, "signed-enums")
    FEATURE(implicit_function_declaration, "implicit-function-decl")
    FEATURE(empty_structs, "empty-structs")
    FEATURE(ext_pointer_arithmetics, "ext-pointer-arithmetics")
    FEATURE(missing_braces_subobject, "missing-braces-subobj")
    FEATURE(statement_expressions, "statement-expressions")
    FEATURE(omitted_conditional_operand, "omitted-conditional-operand")
    FEATURE(int_to_pointer, "int-to-pointer")
    FEATURE(permissive_pointer_conv, "permissive-pointer-conv")
    FEATURE(named_macro_vararg, "named-macro-vararg")
    FEATURE(include_next, "include-next")
    FEATURE(fail_on_assembly, "fail-on-assembly")
    FEATURE(va_args_concat, "va-args-comma-concat")
    FEATURE(switch_case_ranges, "switch-case-ranges")
    FEATURE(designator_subscript_ranges, "designator-subscript-ranges")
#undef FEATURE

#define CODEGEN(_id, _name)                         \
    if (configuration->codegen._id) {               \
        fprintf(output, " --codegen-%s", _name);    \
    } else {                                        \
        fprintf(output, " --no-codegen-%s", _name); \
    }

    CODEGEN(emulated_tls, "emulated-tls")
    CODEGEN(position_independent_code, "pic")
    CODEGEN(omit_frame_pointer, "omit-frame-pointer")

#undef CODEGEN

    if (configuration->codegen.syntax != NULL) {
        fprintf(output, " --codegen-syntax %s", configuration->codegen.syntax);
    }
    if (configuration->codegen.print_details != NULL) {
        fprintf(output, " --codegen-details %s", configuration->codegen.print_details);
    }
    if (configuration->codegen.pipeline_spec != NULL) {
        fprintf(output, " --codegen-pipeline %s", configuration->codegen.pipeline_spec);
    }

    fprintf(output, "\n");
    return KEFIR_OK;
}

static void sighandler(int signum) {
    fprintf(stderr, "Kefir compiler caught signal: %d, terminating\n", signum);
    exit(EXIT_FAILURE);
}

static void configure_compiler_signals(void) {
    signal(SIGABRT, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGFPE, sighandler);
}

static int run_compiler(void *payload) {
    configure_compiler_signals();
    ASSIGN_DECL_CAST(const struct kefir_compiler_runner_configuration *, configuration, payload);
    kefir_result_t res = kefir_run_compiler(kefir_system_memalloc(), configuration);
    return kefir_report_error(stderr, res, configuration->error_report_type == KEFIR_COMPILER_RUNNER_ERROR_REPORT_JSON)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}

kefir_result_t kefir_driver_run_compiler(const struct kefir_compiler_runner_configuration *configuration,
                                         struct kefir_process *process) {
    REQUIRE(configuration != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler runner configuration"));
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    if (configuration->verbose) {
        REQUIRE_OK(output_compiler_config(stderr, configuration));
    }

#ifdef KEFIR_DRIVER_DEBUG_NOFORK
    run_compiler((void *) configuration);
    exit(0);
#else
    REQUIRE_OK(kefir_process_run(process, run_compiler, (void *) configuration));
#endif
    return KEFIR_OK;
}

static kefir_result_t copy_string_list_to_array(struct kefir_mem *mem, const struct kefir_list *list,
                                                struct kefir_string_array *array) {
    for (const struct kefir_list_entry *iter = kefir_list_head(list); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, str, iter->value);
        REQUIRE_OK(kefir_string_array_append(mem, array, str));
    }
    return KEFIR_OK;
}

static kefir_result_t output_argv(FILE *output, const struct kefir_string_array *argv) {
    fprintf(output, "+");
    for (kefir_size_t i = 0; i < argv->length; i++) {
        fprintf(output, " %s", argv->array[i]);
    }
    fprintf(output, "\n");
    return KEFIR_OK;
}

kefir_result_t kefir_driver_run_assembler(struct kefir_mem *mem, const char *output_file, const char *input_file,
                                          const struct kefir_driver_assembler_configuration *config,
                                          const struct kefir_driver_external_resources *resources,
                                          struct kefir_process *process) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(output_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output file"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));
    REQUIRE(resources != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver external resources"));
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    struct kefir_string_array argv;
    REQUIRE_OK(kefir_string_array_init(mem, &argv));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, resources->assembler_path));
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, "-o"));
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, output_file));
    if (input_file != NULL) {
        REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, input_file));
    }

    switch (config->target) {
        case KEFIR_DRIVER_ASSEMBLER_GAS_INTEL:
        case KEFIR_DRIVER_ASSEMBLER_GAS_INTEL_PREFIX:
        case KEFIR_DRIVER_ASSEMBLER_GAS_ATT:
            // Intentionally left blank
            break;

        case KEFIR_DRIVER_ASSEMBLER_YASM:
            REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, "-f"));
            REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, "elf64"));
            break;
    }

    REQUIRE_CHAIN(&res, copy_string_list_to_array(mem, &config->arguments, &argv));

    if (config->verbose) {
        REQUIRE_OK(output_argv(stderr, &argv));
    }

    REQUIRE_CHAIN(&res, kefir_process_execute(process, resources->assembler_path, argv.array));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_array_free(mem, &argv);
        return res;
    });

    REQUIRE_OK(kefir_string_array_free(mem, &argv));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_run_linker(struct kefir_mem *mem, const char *output,
                                       const struct kefir_driver_linker_configuration *config,
                                       const struct kefir_driver_external_resources *resources,
                                       struct kefir_process *process) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linker output file"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));
    REQUIRE(resources != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver external resources"));
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    struct kefir_string_array argv;
    REQUIRE_OK(kefir_string_array_init(mem, &argv));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, resources->linker_path));

    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, "-o"));
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, output));
    REQUIRE_CHAIN(&res, copy_string_list_to_array(mem, &config->arguments, &argv));

    if (config->flags.verbose) {
        REQUIRE_OK(output_argv(stderr, &argv));
    }

    REQUIRE_CHAIN(&res, kefir_process_execute(process, resources->linker_path, argv.array));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_array_free(mem, &argv);
        return res;
    });

    REQUIRE_OK(kefir_string_array_free(mem, &argv));
    return KEFIR_OK;
}
