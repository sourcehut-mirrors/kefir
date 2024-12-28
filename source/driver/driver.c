/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#define _POSIX_SOURCE
#include "kefir/core/platform.h"

#include "kefir/driver/driver.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/os_error.h"
#include "kefir/core/string_array.h"
#include "kefir/driver/compiler_options.h"
#include "kefir/driver/target_configuration.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <libgen.h>

#define KEFIR_OPTIMIZER_PIPELINE_FULL_SPEC "phi-pull,mem2reg,phi-pull,constant-fold,op-simplify,branch-removal"

#define KEFIR_CODEGEN_AMD64_PIPELINE_FULL_SPEC \
    "amd64-drop-virtual,amd64-propagate-jump,amd64-eliminate-label,amd64-peephole"

static kefir_result_t driver_generate_asm_config(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                 struct kefir_driver_configuration *config,
                                                 const struct kefir_driver_external_resources *externals,
                                                 struct kefir_driver_assembler_configuration *assembler_config) {
    REQUIRE_OK(
        kefir_driver_apply_target_assembler_configuration(mem, symbols, externals, assembler_config, &config->target));
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->assembler_arguments); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, argument, iter->value);
        REQUIRE_OK(kefir_driver_assembler_configuration_add_argument(mem, assembler_config, argument));
    }
    assembler_config->verbose = config->flags.verbose;
    assembler_config->target = config->assembler.target;
    return KEFIR_OK;
}

static kefir_result_t driver_generate_linker_config(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                    struct kefir_driver_configuration *config,
                                                    const struct kefir_driver_external_resources *externals,
                                                    struct kefir_driver_linker_configuration *linker_config) {
    linker_config->flags.static_linking = config->flags.static_linking;
    linker_config->flags.shared_linking = config->flags.shared_linking;
    linker_config->flags.export_dynamic = config->flags.export_dynamic;
    linker_config->flags.pie_linking = config->flags.position_independent_executable;
    linker_config->flags.link_start_files = config->flags.link_start_files;
    linker_config->flags.link_default_libs = config->flags.link_default_libs;
    linker_config->flags.link_libc = config->flags.link_libc;
    linker_config->flags.link_atomics = config->flags.soft_atomics;
    linker_config->flags.verbose = config->flags.verbose;
    REQUIRE_OK(kefir_driver_apply_target_linker_initial_configuration(mem, symbols, externals, linker_config,
                                                                      &config->target));
    return KEFIR_OK;
}

static kefir_result_t driver_handle_linker_argument(struct kefir_mem *mem, const struct kefir_driver_argument *argument,
                                                    struct kefir_driver_linker_configuration *linker_config) {
    switch (argument->type) {
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED_ASSEMBLY:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY:
            assert(false);

        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY:
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, "-l"));
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, argument->value));
            break;

        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH:
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, "-L"));
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, argument->value));
            break;

        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT:
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, "-e"));
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, argument->value));
            break;

        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP:
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, "-s"));
            break;

        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC:
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, "-r"));
            break;

        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL:
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, "-u"));
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, argument->value));
            break;

        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA:
            REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, argument->value));
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_driver_generate_compiler_config(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                     struct kefir_driver_configuration *config,
                                                     const struct kefir_driver_external_resources *externals,
                                                     struct kefir_compiler_runner_configuration *compiler_config) {
    REQUIRE_OK(kefir_compiler_runner_configuration_init(compiler_config));

    compiler_config->verbose = config->flags.verbose;
    REQUIRE_OK(kefir_driver_apply_target_compiler_configuration(mem, symbols, externals, compiler_config,
                                                                &config->target, config));

    switch (config->stage) {
        case KEFIR_DRIVER_STAGE_PREPROCESS:
        case KEFIR_DRIVER_STAGE_PREPROCESS_SAVE:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_PREPROCESS;
            break;

        case KEFIR_DRIVER_STAGE_DEPENDENCY_OUTPUT:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_DEPENDENCIES;
            compiler_config->dependency_output.output_system_deps = config->dependency_output.output_system_deps;
            compiler_config->dependency_output.target_name = config->dependency_output.target_name;
            break;

        case KEFIR_DRIVER_STAGE_PRINT_TOKENS:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_TOKENS;
            break;

        case KEFIR_DRIVER_STAGE_PRINT_AST:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_AST;
            break;

        case KEFIR_DRIVER_STAGE_PRINT_IR:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_IR;
            break;

        case KEFIR_DRIVER_STAGE_PRINT_OPT:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_OPT;
            break;

        case KEFIR_DRIVER_STAGE_COMPILE:
        case KEFIR_DRIVER_STAGE_ASSEMBLE:
        case KEFIR_DRIVER_STAGE_LINK:
        case KEFIR_DRIVER_STAGE_RUN:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_ASSEMBLY;
            break;
    }

    compiler_config->skip_preprocessor = config->flags.skip_preprocessor;
    if (!config->flags.restrictive_mode) {
        compiler_config->features.missing_function_return_type = true;
        compiler_config->features.designated_initializer_colons = true;
        compiler_config->features.labels_as_values = true;
        compiler_config->features.non_strict_qualifiers = true;
        compiler_config->features.implicit_function_declaration = true;
        compiler_config->features.empty_structs = true;
        compiler_config->features.ext_pointer_arithmetics = true;
        compiler_config->features.missing_braces_subobject = true;
        compiler_config->features.statement_expressions = true;
        compiler_config->features.omitted_conditional_operand = true;
        compiler_config->features.int_to_pointer = true;
        compiler_config->features.permissive_pointer_conv = true;
        compiler_config->features.named_macro_vararg = true;
        compiler_config->features.include_next = true;
        compiler_config->features.va_args_concat = true;
        compiler_config->features.switch_case_ranges = true;
        compiler_config->features.designator_subscript_ranges = true;
    }

    compiler_config->debug_info = config->flags.debug_info;
    compiler_config->codegen.position_independent_code = config->flags.position_independent_code;
    if (compiler_config->codegen.position_independent_code) {
        REQUIRE_OK(kefir_compiler_runner_configuration_define(mem, compiler_config, "__pic__", "2"));
        REQUIRE_OK(kefir_compiler_runner_configuration_define(mem, compiler_config, "__PIC__", "2"));
    }
    switch (config->flags.omit_frame_pointer) {
        case KEFIR_DRIVER_FRAME_POINTER_OMISSION_ENABLE:
            compiler_config->codegen.omit_frame_pointer = true;
            break;

        case KEFIR_DRIVER_FRAME_POINTER_OMISSION_DISABLE:
            compiler_config->codegen.omit_frame_pointer = false;
            break;

        case KEFIR_DRIVER_FRAME_POINTER_OMISSION_UNSPECIFIED:
            // Intentionally left blank
            break;
    }

    struct kefir_list_entry *include_insert_iter = NULL;
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->include_directories); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, include_dir, iter->value);
        include_dir = kefir_string_pool_insert(mem, symbols, include_dir, NULL);
        REQUIRE(include_dir != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include directory into string pool"));
        REQUIRE_OK(
            kefir_list_insert_after(mem, &compiler_config->include_path, include_insert_iter, (void *) include_dir));
        if (include_insert_iter == NULL) {
            include_insert_iter = kefir_list_head(&compiler_config->include_path);
        } else {
            kefir_list_next((const struct kefir_list_entry **) &include_insert_iter);
        }
    }
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->include_files); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, include_file, iter->value);
        include_file = kefir_string_pool_insert(mem, symbols, include_file, NULL);
        REQUIRE(include_file != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include file into string pool"));
        REQUIRE_OK(kefir_list_insert_after(mem, &compiler_config->include_files,
                                           kefir_list_tail(&compiler_config->include_files), (void *) include_file));
    }
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->undefines); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, iter->value);
        identifier = kefir_string_pool_insert(mem, symbols, identifier, NULL);
        REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to identifier into string pool"));
        REQUIRE_OK(kefir_list_insert_after(mem, &compiler_config->undefines,
                                           kefir_list_tail(&compiler_config->undefines), (void *) identifier));
        if (kefir_hashtree_has(&compiler_config->defines, (kefir_hashtree_key_t) identifier)) {
            REQUIRE_OK(kefir_hashtree_delete(mem, &compiler_config->defines, (kefir_hashtree_key_t) identifier));
        }
    }
    struct kefir_hashtree_node_iterator define_iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&config->defines, &define_iter); node != NULL;
         node = kefir_hashtree_next(&define_iter)) {
        REQUIRE_OK(kefir_compiler_runner_configuration_define(mem, compiler_config, (const char *) node->key,
                                                              (const char *) node->value));
    }

    if (compiler_config->features.declare_atomic_support) {
        compiler_config->features.declare_atomic_support = config->flags.soft_atomics;
    }
    switch (config->compiler.optimization_level) {
        case 0:
            compiler_config->optimizer_pipeline_spec = NULL;
            compiler_config->codegen.pipeline_spec = NULL;
            break;

        default:
            if (config->compiler.optimization_level > 0) {
                compiler_config->optimizer_pipeline_spec = KEFIR_OPTIMIZER_PIPELINE_FULL_SPEC;
                if (config->flags.omit_frame_pointer == KEFIR_DRIVER_FRAME_POINTER_OMISSION_UNSPECIFIED) {
                    compiler_config->codegen.omit_frame_pointer = true;
                }

                if (config->target.arch == KEFIR_DRIVER_TARGET_ARCH_X86_64) {
                    compiler_config->codegen.pipeline_spec = KEFIR_CODEGEN_AMD64_PIPELINE_FULL_SPEC;
                }
            }
            break;
    }

    switch (config->assembler.target) {
        case KEFIR_DRIVER_ASSEMBLER_GAS_ATT:
            compiler_config->codegen.syntax = "x86_64-att";
            break;

        case KEFIR_DRIVER_ASSEMBLER_GAS_INTEL:
            compiler_config->codegen.syntax = "x86_64-intel_noprefix";
            break;

        case KEFIR_DRIVER_ASSEMBLER_GAS_INTEL_PREFIX:
            compiler_config->codegen.syntax = "x86_64-intel_prefix";
            break;

        case KEFIR_DRIVER_ASSEMBLER_YASM:
            compiler_config->codegen.syntax = "x86_64-yasm";
            break;
    }

    struct kefir_string_array extra_args_buf;
    REQUIRE_OK(kefir_string_array_init(mem, &extra_args_buf));
    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &extra_args_buf, ""));
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->compiler_arguments);
         res == KEFIR_OK && iter != NULL; kefir_list_next(&iter)) {
        REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &extra_args_buf, iter->value));
    }
    kefir_size_t positional_args = extra_args_buf.length;
    REQUIRE_CHAIN(&res,
                  kefir_parse_cli_options(mem, symbols, compiler_config, &positional_args,
                                          KefirCompilerConfigurationOptions, KefirCompilerConfigurationOptionCount,
                                          extra_args_buf.array, extra_args_buf.length, stderr));
    REQUIRE_CHAIN_SET(
        &res, positional_args == extra_args_buf.length,
        KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Passing positional arguments directly to compiler is not permitted"));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_array_free(mem, &extra_args_buf);
        return res;
    });
    REQUIRE_OK(kefir_string_array_free(mem, &extra_args_buf));
    return KEFIR_OK;
}

static kefir_result_t driver_update_compiler_config(struct kefir_compiler_runner_configuration *compiler_config,
                                                    struct kefir_driver_argument *argument) {
    switch (argument->type) {
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED_ASSEMBLY:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY:
            if (strcmp(argument->value, "-") == 0) {
                compiler_config->input_filepath = NULL;
                if (compiler_config->source_id == NULL) {
                    compiler_config->source_id = "<stdin>";
                }
            } else {
                compiler_config->input_filepath = argument->value;
                compiler_config->source_id = argument->value;
            }
            break;

        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY:
        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH:
        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT:
        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP:
        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC:
        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL:
        case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t generate_asm_name(struct kefir_mem *mem, const struct kefir_driver_external_resources *externals,
                                        const char **output_filename) {
    REQUIRE_OK(kefir_tempfile_manager_create_file(mem, externals->tmpfile_manager, "asm-file", output_filename));
    return KEFIR_OK;
}

static kefir_result_t driver_compile_and_assemble(struct kefir_mem *mem,
                                                  const struct kefir_driver_external_resources *externals,
                                                  struct kefir_driver_assembler_configuration *assembler_config,
                                                  struct kefir_compiler_runner_configuration *compiler_config,
                                                  struct kefir_driver_argument *argument, const char *object_filename) {
    struct kefir_process compiler_process, assembler_process;
    REQUIRE_OK(driver_update_compiler_config(compiler_config, argument));

    const char *asm_filename;
    REQUIRE_OK(generate_asm_name(mem, externals, &asm_filename));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_OK(kefir_process_init(&compiler_process));
    REQUIRE_CHAIN(&res, kefir_process_redirect_stdout_to_file(&compiler_process, asm_filename));
    REQUIRE_CHAIN(&res, kefir_driver_run_compiler(compiler_config, &compiler_process));
    REQUIRE_CHAIN(&res, kefir_process_wait(&compiler_process));
    REQUIRE_CHAIN_SET(&res, compiler_process.status.exited && compiler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_INTERRUPT);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&compiler_process);
        remove(asm_filename);
        return res;
    });

    res = KEFIR_OK;
    REQUIRE_OK(kefir_process_init(&assembler_process));
    REQUIRE_CHAIN(&res, kefir_driver_run_assembler(mem, object_filename, asm_filename, assembler_config, externals,
                                                   &assembler_process));
    REQUIRE_CHAIN(&res, kefir_process_wait(&assembler_process));
    REQUIRE_CHAIN_SET(&res, assembler_process.status.exited && assembler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_SET_ERRORF(KEFIR_SUBPROCESS_ERROR, "Failed to assemble '%s'", argument->value));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&assembler_process);
        remove(object_filename);
        remove(asm_filename);
        return res;
    });
    remove(asm_filename);
    return KEFIR_OK;
}

static kefir_result_t driver_preprocess_and_assemble(struct kefir_mem *mem,
                                                  const struct kefir_driver_external_resources *externals,
                                                  struct kefir_driver_assembler_configuration *assembler_config,
                                                  struct kefir_compiler_runner_configuration *compiler_config,
                                                  struct kefir_driver_argument *argument, const char *object_filename) {
    struct kefir_process compiler_process, assembler_process;
    REQUIRE_OK(driver_update_compiler_config(compiler_config, argument));

    struct kefir_compiler_runner_configuration local_compiler_config = *compiler_config;
    local_compiler_config.action = KEFIR_COMPILER_RUNNER_ACTION_PREPROCESS;

    const char *asm_filename;
    REQUIRE_OK(generate_asm_name(mem, externals, &asm_filename));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_OK(kefir_process_init(&compiler_process));
    REQUIRE_CHAIN(&res, kefir_process_redirect_stdout_to_file(&compiler_process, asm_filename));
    REQUIRE_CHAIN(&res, kefir_driver_run_compiler(&local_compiler_config, &compiler_process));
    REQUIRE_CHAIN(&res, kefir_process_wait(&compiler_process));
    REQUIRE_CHAIN_SET(&res, compiler_process.status.exited && compiler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_INTERRUPT);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&compiler_process);
        remove(asm_filename);
        return res;
    });

    res = KEFIR_OK;
    REQUIRE_OK(kefir_process_init(&assembler_process));
    REQUIRE_CHAIN(&res, kefir_driver_run_assembler(mem, object_filename, asm_filename, assembler_config, externals,
                                                   &assembler_process));
    REQUIRE_CHAIN(&res, kefir_process_wait(&assembler_process));
    REQUIRE_CHAIN_SET(&res, assembler_process.status.exited && assembler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_SET_ERRORF(KEFIR_SUBPROCESS_ERROR, "Failed to assemble '%s'", argument->value));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&assembler_process);
        remove(object_filename);
        remove(asm_filename);
        return res;
    });
    remove(asm_filename);
    return KEFIR_OK;
}

static kefir_result_t driver_compile(struct kefir_compiler_runner_configuration *compiler_config,
                                     struct kefir_driver_argument *argument, const char *output_filename) {
    struct kefir_process compiler_process;
    if (argument != NULL) {
        REQUIRE_OK(driver_update_compiler_config(compiler_config, argument));
    }

    REQUIRE_OK(kefir_process_init(&compiler_process));
    kefir_result_t res = KEFIR_OK;

    if (output_filename != NULL) {
        REQUIRE_CHAIN(&res, kefir_process_redirect_stdout_to_file(&compiler_process, output_filename));
    }

    REQUIRE_CHAIN(&res, kefir_driver_run_compiler(compiler_config, &compiler_process));

    REQUIRE_CHAIN(&res, kefir_process_wait(&compiler_process));
    REQUIRE_CHAIN_SET(&res, compiler_process.status.exited && compiler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_INTERRUPT);

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&compiler_process);
        remove(output_filename);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t driver_preprocess(struct kefir_compiler_runner_configuration *compiler_config,
                                     struct kefir_driver_argument *argument, const char *output_filename) {
    struct kefir_process compiler_process;
    if (argument != NULL) {
        REQUIRE_OK(driver_update_compiler_config(compiler_config, argument));
    }
    struct kefir_compiler_runner_configuration local_compiler_config = *compiler_config;
    local_compiler_config.action = KEFIR_COMPILER_RUNNER_ACTION_PREPROCESS;

    REQUIRE_OK(kefir_process_init(&compiler_process));
    kefir_result_t res = KEFIR_OK;

    if (output_filename != NULL) {
        REQUIRE_CHAIN(&res, kefir_process_redirect_stdout_to_file(&compiler_process, output_filename));
    }

    REQUIRE_CHAIN(&res, kefir_driver_run_compiler(&local_compiler_config, &compiler_process));

    REQUIRE_CHAIN(&res, kefir_process_wait(&compiler_process));
    REQUIRE_CHAIN_SET(&res, compiler_process.status.exited && compiler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_INTERRUPT);

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&compiler_process);
        remove(output_filename);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t driver_assemble(struct kefir_mem *mem, const struct kefir_driver_external_resources *externals,
                                      struct kefir_driver_assembler_configuration *assembler_config,
                                      struct kefir_driver_argument *argument, const char *object_filename) {
    struct kefir_process assembler_process;

    REQUIRE_OK(kefir_process_init(&assembler_process));
    kefir_result_t res = KEFIR_OK;

    REQUIRE_CHAIN(&res, kefir_driver_run_assembler(mem, object_filename, argument->value, assembler_config, externals,
                                                   &assembler_process));

    REQUIRE_CHAIN(&res, kefir_process_wait(&assembler_process));
    REQUIRE_CHAIN_SET(&res, assembler_process.status.exited && assembler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_SET_ERRORF(KEFIR_SUBPROCESS_ERROR, "Failed to assemble '%s'", argument->value));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&assembler_process);
        remove(object_filename);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t generate_object_name(struct kefir_mem *mem,
                                           const struct kefir_driver_external_resources *externals,
                                           const char **output_filename) {
    REQUIRE_OK(kefir_tempfile_manager_create_file(mem, externals->tmpfile_manager, "object-file", output_filename));
    return KEFIR_OK;
}

static kefir_result_t get_file_basename(char *buffer, kefir_size_t buffer_size, const char *filename, char **result) {
    if (strcmp(filename, "-") == 0) {
        *result = "out";
        return KEFIR_OK;
    }
    strncpy(buffer, filename, buffer_size);
    char *input_basename = basename(buffer);
    REQUIRE(input_basename != NULL, KEFIR_SET_OS_ERROR("Failed to retrieve input file base name"));
    for (char *c = input_basename + strlen(input_basename); c > input_basename;
         c--) {  // Strip extension from file name
        if (*c == '.') {
            *c = '\0';
            break;
        }
    }
    *result = input_basename;
    return KEFIR_OK;
}

static kefir_result_t driver_run_argument(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                          const struct kefir_driver_external_resources *externals,
                                          struct kefir_driver_assembler_configuration *assembler_config,
                                          struct kefir_driver_linker_configuration *linker_config,
                                          struct kefir_compiler_runner_configuration *compiler_config,
                                          struct kefir_driver_argument *argument) {
    const char *output_filename = NULL;
    switch (config->stage) {
        case KEFIR_DRIVER_STAGE_LINK:
        case KEFIR_DRIVER_STAGE_RUN:
            switch (argument->type) {
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED:
                    REQUIRE_OK(generate_object_name(mem, externals, &output_filename));
                    REQUIRE_OK(driver_compile_and_assemble(mem, externals, assembler_config, compiler_config, argument,
                                                           output_filename));
                    REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY:
                    REQUIRE_OK(generate_object_name(mem, externals, &output_filename));
                    REQUIRE_OK(driver_assemble(mem, externals, assembler_config, argument, output_filename));
                    REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED_ASSEMBLY:
                    REQUIRE_OK(generate_object_name(mem, externals, &output_filename));
                    REQUIRE_OK(driver_preprocess_and_assemble(mem, externals, assembler_config, compiler_config, argument, output_filename));
                    REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY:
                    output_filename = argument->value;
                    REQUIRE_OK(kefir_driver_linker_configuration_add_argument(mem, linker_config, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA:
                    REQUIRE_OK(driver_handle_linker_argument(mem, argument, linker_config));
                    break;
            }
            break;

        case KEFIR_DRIVER_STAGE_ASSEMBLE: {
            char object_filename[PATH_MAX + 1];
            if (config->output_file != NULL) {
                output_filename = config->output_file;
            } else {
                char input_basename_buf[PATH_MAX + 1];
                char *input_basename = NULL;
                REQUIRE_OK(get_file_basename(input_basename_buf, sizeof(input_basename_buf) - 1, argument->value,
                                             &input_basename));
                snprintf(object_filename, sizeof(object_filename) - 1, "%s%s", input_basename,
                         externals->extensions.object_file[0]);
                output_filename = object_filename;
            }
            switch (argument->type) {
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED:
                    REQUIRE_OK(driver_compile_and_assemble(mem, externals, assembler_config, compiler_config, argument,
                                                           output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY:
                    REQUIRE_OK(driver_assemble(mem, externals, assembler_config, argument, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED_ASSEMBLY:
                    REQUIRE_OK(driver_preprocess_and_assemble(mem, externals, assembler_config, compiler_config, argument, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY:
                    // Intentionally left blank
                    break;

                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA:
                    // Intentionally left blank
                    break;
            }
        } break;

        case KEFIR_DRIVER_STAGE_COMPILE: {
            char object_filename[PATH_MAX + 1];
            if (config->output_file != NULL && strcmp(config->output_file, "-") != 0) {
                output_filename = config->output_file;
            } else if (config->output_file == NULL) {
                char input_basename_buf[PATH_MAX + 1];
                char *input_basename = NULL;
                REQUIRE_OK(get_file_basename(input_basename_buf, sizeof(input_basename_buf) - 1, argument->value,
                                             &input_basename));
                snprintf(object_filename, sizeof(object_filename) - 1, "%s%s", input_basename,
                         externals->extensions.assembly_file[0]);
                output_filename = object_filename;
            }
            switch (argument->type) {
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED:
                    REQUIRE_OK(driver_compile(compiler_config, argument, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED_ASSEMBLY:
                    REQUIRE_OK(driver_preprocess(compiler_config, argument, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY:
                    // Intentionally left blank
                    break;

                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA:
                    // Intentionally left blank
                    break;
            }
        } break;

        case KEFIR_DRIVER_STAGE_PREPROCESS:
        case KEFIR_DRIVER_STAGE_PREPROCESS_SAVE:
        case KEFIR_DRIVER_STAGE_DEPENDENCY_OUTPUT:
        case KEFIR_DRIVER_STAGE_PRINT_TOKENS:
        case KEFIR_DRIVER_STAGE_PRINT_AST:
        case KEFIR_DRIVER_STAGE_PRINT_IR:
        case KEFIR_DRIVER_STAGE_PRINT_OPT: {
            char object_filename[PATH_MAX + 1];
            if (config->output_file != NULL && strcmp(config->output_file, "-") != 0) {
                output_filename = config->output_file;
            } else if (config->stage == KEFIR_DRIVER_STAGE_PREPROCESS_SAVE && config->output_file == NULL) {
                char input_basename_buf[PATH_MAX + 1];
                char *input_basename = NULL;
                REQUIRE_OK(get_file_basename(input_basename_buf, sizeof(input_basename_buf) - 1, argument->value,
                                             &input_basename));
                snprintf(object_filename, sizeof(object_filename) - 1, "%s%s", input_basename,
                         externals->extensions.preprocessed_file[0]);
                output_filename = object_filename;
            }

            if (config->stage == KEFIR_DRIVER_STAGE_DEPENDENCY_OUTPUT &&
                config->dependency_output.target_name == NULL) {
                char input_basename_buf[PATH_MAX + 1];
                char *input_basename = NULL;
                REQUIRE_OK(get_file_basename(input_basename_buf, sizeof(input_basename_buf) - 1, argument->value,
                                             &input_basename));
                snprintf(object_filename, sizeof(object_filename) - 1, "%s%s", input_basename,
                         externals->extensions.object_file[0]);
                compiler_config->dependency_output.target_name = object_filename;
            }
            switch (argument->type) {
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED:
                    REQUIRE_OK(driver_compile(compiler_config, argument, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED_ASSEMBLY:
                    REQUIRE_OK(driver_preprocess(compiler_config, argument, output_filename));
                    break;

                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY:
                    // Intentionally left blank
                    break;

                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL:
                case KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA:
                    // Intentionally left blank
                    break;
            }
            compiler_config->dependency_output.target_name = NULL;
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t driver_run_linker(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                        struct kefir_driver_configuration *config,
                                        const struct kefir_driver_external_resources *externals,
                                        struct kefir_driver_linker_configuration *linker_config) {
    REQUIRE_OK(
        kefir_driver_apply_target_linker_final_configuration(mem, symbols, externals, linker_config, &config->target));

    struct kefir_process linker_process;
    REQUIRE_OK(kefir_process_init(&linker_process));
    const char *output_file = config->output_file != NULL ? config->output_file : "a.out";
    REQUIRE_OK(kefir_driver_run_linker(mem, output_file, linker_config, externals, &linker_process));
    REQUIRE_OK(kefir_process_wait(&linker_process));
    REQUIRE_ELSE(linker_process.status.exited && linker_process.status.exit_code == EXIT_SUCCESS, {
        remove(output_file);
        return KEFIR_INTERRUPT;
    });
    return KEFIR_OK;
}

static kefir_result_t driver_run_impl(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                      struct kefir_driver_configuration *config,
                                      const struct kefir_driver_external_resources *externals,
                                      struct kefir_driver_assembler_configuration *assembler_config,
                                      struct kefir_driver_linker_configuration *linker_config,
                                      struct kefir_compiler_runner_configuration *compiler_config) {
    REQUIRE(kefir_list_length(&config->arguments) > 0,
            KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Selected operation requires non-empty argument list"));
    switch (config->stage) {
        case KEFIR_DRIVER_STAGE_LINK:
        case KEFIR_DRIVER_STAGE_RUN:
            REQUIRE_OK(driver_generate_linker_config(mem, symbols, config, externals, linker_config));
            // Fallthrough

        case KEFIR_DRIVER_STAGE_ASSEMBLE:
            REQUIRE_OK(driver_generate_asm_config(mem, symbols, config, externals, assembler_config));
            // Fallthrough

        case KEFIR_DRIVER_STAGE_PREPROCESS:
        case KEFIR_DRIVER_STAGE_PREPROCESS_SAVE:
        case KEFIR_DRIVER_STAGE_DEPENDENCY_OUTPUT:
        case KEFIR_DRIVER_STAGE_PRINT_TOKENS:
        case KEFIR_DRIVER_STAGE_PRINT_AST:
        case KEFIR_DRIVER_STAGE_PRINT_IR:
        case KEFIR_DRIVER_STAGE_PRINT_OPT:
        case KEFIR_DRIVER_STAGE_COMPILE:
            REQUIRE_OK(kefir_driver_generate_compiler_config(mem, symbols, config, externals, compiler_config));
            // Intentionally left blank
            break;
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&config->arguments); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_driver_argument *, argument, iter->value);
        REQUIRE_OK(
            driver_run_argument(mem, config, externals, assembler_config, linker_config, compiler_config, argument));
    }

    if (config->stage == KEFIR_DRIVER_STAGE_LINK) {

        REQUIRE_OK(driver_run_linker(mem, symbols, config, externals, linker_config));
    } else if (config->stage == KEFIR_DRIVER_STAGE_RUN) {
        if (config->output_file == NULL) {
            REQUIRE_OK(
                kefir_tempfile_manager_create_file(mem, externals->tmpfile_manager, "exe", &config->output_file));
        }

        REQUIRE_OK(driver_run_linker(mem, symbols, config, externals, linker_config));

        struct kefir_string_array argv;
        REQUIRE_OK(kefir_string_array_init(mem, &argv));
        REQUIRE_OK(kefir_string_array_append(mem, &argv, config->output_file));
        for (const struct kefir_list_entry *iter = kefir_list_head(&config->run.args); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(const char *, arg, iter->value);
            REQUIRE_OK(kefir_string_array_append(mem, &argv, arg));
        }

        struct kefir_process process;
        REQUIRE_OK(kefir_process_init(&process));
        kefir_result_t res = KEFIR_OK;
        if (config->run.file_stdin != NULL) {
            REQUIRE_CHAIN(&res, kefir_process_redirect_stdin_from_file(&process, config->run.file_stdin));
        }
        if (config->run.file_stdout != NULL) {
            REQUIRE_CHAIN(&res, kefir_process_redirect_stdout_to_file(&process, config->run.file_stdout));
        }
        if (config->run.file_stderr != NULL) {
            REQUIRE_CHAIN(&res, kefir_process_redirect_stderr_to_file(&process, config->run.file_stderr));
        } else if (config->run.stderr_to_stdout) {
            REQUIRE_CHAIN(&res, kefir_process_redirect_stderr_to_stdout(&process));
        }
        REQUIRE_CHAIN(&res, kefir_process_self_execute(&process, config->output_file, argv.array));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_process_close(&process);
            return res;
        });
    }

    return KEFIR_OK;
}

kefir_result_t kefir_driver_run(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                struct kefir_driver_configuration *config,
                                const struct kefir_driver_external_resources *externals) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(symbols != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbol table"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(externals != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver external resources"));

    struct kefir_driver_assembler_configuration assembler_config;
    struct kefir_driver_linker_configuration linker_config;
    struct kefir_compiler_runner_configuration compiler_config;

    REQUIRE_OK(kefir_driver_assembler_configuration_init(&assembler_config));
    REQUIRE_OK(kefir_driver_linker_configuration_init(&linker_config));
    REQUIRE_OK(kefir_compiler_runner_configuration_init(&compiler_config));

    kefir_result_t res =
        driver_run_impl(mem, symbols, config, externals, &assembler_config, &linker_config, &compiler_config);

    REQUIRE_ELSE(res == KEFIR_OK, {
        if (config->output_file != NULL) {
            remove(config->output_file);
        }
        kefir_compiler_runner_configuration_free(mem, &compiler_config);
        kefir_driver_assembler_configuration_free(mem, &assembler_config);
        kefir_driver_linker_configuration_free(mem, &linker_config);
        return res;
    });

    res = kefir_compiler_runner_configuration_free(mem, &compiler_config);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_driver_assembler_configuration_free(mem, &assembler_config);
        kefir_driver_linker_configuration_free(mem, &linker_config);
        return res;
    });

    res = kefir_driver_assembler_configuration_free(mem, &assembler_config);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_driver_linker_configuration_free(mem, &linker_config);
        return res;
    });

    REQUIRE_OK(kefir_driver_linker_configuration_free(mem, &linker_config));
    return KEFIR_OK;
}
