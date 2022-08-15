/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

static kefir_result_t driver_generate_asm_config(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
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
    return KEFIR_OK;
}

static kefir_result_t driver_generate_linker_config(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                    struct kefir_driver_configuration *config,
                                                    const struct kefir_driver_external_resources *externals,
                                                    struct kefir_driver_linker_configuration *linker_config) {
    linker_config->flags.static_linking = config->flags.static_linking;
    linker_config->flags.link_start_files = config->flags.link_start_files;
    linker_config->flags.link_default_libs = config->flags.link_default_libs;
    linker_config->flags.link_libc = config->flags.link_libc;
    linker_config->flags.link_rtlib = config->flags.link_rtlib;
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

static kefir_result_t driver_generate_compiler_config(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                      struct kefir_driver_configuration *config,
                                                      const struct kefir_driver_external_resources *externals,
                                                      struct kefir_compiler_runner_configuration *compiler_config) {
    REQUIRE_OK(kefir_compiler_runner_configuration_init(compiler_config));

    if (config->stage == KEFIR_DRIVER_STAGE_PRINT_RUNTIME_CODE) {
        REQUIRE_OK(kefir_driver_apply_target_profile_configuration(compiler_config, &config->target));
    } else {
        REQUIRE_OK(kefir_driver_apply_target_compiler_configuration(mem, symbols, externals, compiler_config,
                                                                    &config->target));
    }

    switch (config->stage) {
        case KEFIR_DRIVER_STAGE_PREPROCESS:
        case KEFIR_DRIVER_STAGE_PREPROCESS_SAVE:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_PREPROCESS;
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

        case KEFIR_DRIVER_STAGE_COMPILE:
        case KEFIR_DRIVER_STAGE_ASSEMBLE:
        case KEFIR_DRIVER_STAGE_LINK:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_ASSEMBLY;
            break;

        case KEFIR_DRIVER_STAGE_PRINT_RUNTIME_CODE:
            compiler_config->action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_RUNTIME_CODE;
            break;
    }

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
    }

    struct kefir_list_entry *include_insert_iter = NULL;
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->include_directories); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, include_dir, iter->value);
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
        REQUIRE_OK(kefir_list_insert_after(mem, &compiler_config->include_files,
                                           kefir_list_tail(&compiler_config->include_files), (void *) include_file));
    }
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->undefines); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, iter->value);
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
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT:
        case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY:
            if (strcmp(argument->value, "-") == 0) {
                compiler_config->input_filepath = NULL;
                compiler_config->source_id = "<stdin>";
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

static kefir_result_t driver_compile_and_assemble(struct kefir_mem *mem,
                                                  const struct kefir_driver_external_resources *externals,
                                                  struct kefir_driver_assembler_configuration *assembler_config,
                                                  struct kefir_compiler_runner_configuration *compiler_config,
                                                  struct kefir_driver_argument *argument, const char *object_filename) {
    struct kefir_process compiler_process, assembler_process;
    REQUIRE_OK(driver_update_compiler_config(compiler_config, argument));

    REQUIRE_OK(kefir_process_init(&compiler_process));
    kefir_result_t res = kefir_process_init(&assembler_process);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&compiler_process);
        return res;
    });

    REQUIRE_CHAIN(&res, kefir_process_pipe(&compiler_process, &assembler_process));

    REQUIRE_CHAIN(&res, kefir_driver_run_compiler(compiler_config, &compiler_process));
    REQUIRE_CHAIN(&res,
                  kefir_driver_run_assembler(mem, object_filename, assembler_config, externals, &assembler_process));

    REQUIRE_CHAIN(&res, kefir_process_wait(&compiler_process));
    REQUIRE_CHAIN_SET(&res, compiler_process.status.exited && compiler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_INTERRUPT);
    REQUIRE_CHAIN(&res, kefir_process_wait(&assembler_process));
    REQUIRE_CHAIN_SET(&res, assembler_process.status.exited && assembler_process.status.exit_code == EXIT_SUCCESS,
                      KEFIR_SET_ERRORF(KEFIR_SUBPROCESS_ERROR, "Failed to assemble '%s'", argument->value));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_process_kill(&compiler_process);
        kefir_process_kill(&assembler_process);
        remove(object_filename);
        return res;
    });
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

static kefir_result_t driver_assemble(struct kefir_mem *mem, const struct kefir_driver_external_resources *externals,
                                      struct kefir_driver_assembler_configuration *assembler_config,
                                      struct kefir_driver_argument *argument, const char *object_filename) {
    struct kefir_process assembler_process;

    REQUIRE_OK(kefir_process_init(&assembler_process));
    kefir_result_t res = KEFIR_OK;

    REQUIRE_CHAIN(&res, kefir_process_redirect_stdin_from_file(&assembler_process, argument->value));

    REQUIRE_CHAIN(&res,
                  kefir_driver_run_assembler(mem, object_filename, assembler_config, externals, &assembler_process));

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
    char object_filename_template[PATH_MAX + 1];
    snprintf(object_filename_template, sizeof(object_filename_template) - 1, "%s/object-file-XXXXXXX",
             externals->work_dir);
    REQUIRE_OK(
        kefir_tempfile_manager_create_file(mem, externals->tmpfile_manager, object_filename_template, output_filename));
    return KEFIR_OK;
}

static kefir_result_t get_file_basename(char *buffer, kefir_size_t buffer_size, const char *filename, char **result) {
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
                         externals->extensions.object_file);
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
                         externals->extensions.assembly_file);
                output_filename = object_filename;
            }
            switch (argument->type) {
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED:
                    REQUIRE_OK(driver_compile(compiler_config, argument, output_filename));
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
        case KEFIR_DRIVER_STAGE_PRINT_TOKENS:
        case KEFIR_DRIVER_STAGE_PRINT_AST:
        case KEFIR_DRIVER_STAGE_PRINT_IR: {
            char object_filename[PATH_MAX + 1];
            if (config->output_file != NULL && strcmp(config->output_file, "-") != 0) {
                output_filename = config->output_file;
            } else if (config->stage == KEFIR_DRIVER_STAGE_PREPROCESS_SAVE && config->output_file == NULL) {
                char input_basename_buf[PATH_MAX + 1];
                char *input_basename = NULL;
                REQUIRE_OK(get_file_basename(input_basename_buf, sizeof(input_basename_buf) - 1, argument->value,
                                             &input_basename));
                snprintf(object_filename, sizeof(object_filename) - 1, "%s%s", input_basename,
                         externals->extensions.preprocessed_file);
                output_filename = object_filename;
            }
            switch (argument->type) {
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE:
                case KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED:
                    REQUIRE_OK(driver_compile(compiler_config, argument, output_filename));
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

        case KEFIR_DRIVER_STAGE_PRINT_RUNTIME_CODE:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t driver_run_linker(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
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

static kefir_result_t driver_print_runtime_code(struct kefir_driver_configuration *config,
                                                struct kefir_compiler_runner_configuration *compiler_config) {
    REQUIRE_OK(driver_compile(compiler_config, NULL, config->output_file));
    return KEFIR_OK;
}

static kefir_result_t driver_run_impl(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                      struct kefir_driver_configuration *config,
                                      const struct kefir_driver_external_resources *externals,
                                      struct kefir_driver_assembler_configuration *assembler_config,
                                      struct kefir_driver_linker_configuration *linker_config,
                                      struct kefir_compiler_runner_configuration *compiler_config) {
    REQUIRE(config->stage == KEFIR_DRIVER_STAGE_PRINT_RUNTIME_CODE || kefir_list_length(&config->arguments) > 0,
            KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Selected operation requires non-empty argument list"));
    switch (config->stage) {
        case KEFIR_DRIVER_STAGE_LINK:
            REQUIRE_OK(driver_generate_linker_config(mem, symbols, config, externals, linker_config));
            // Fallthrough

        case KEFIR_DRIVER_STAGE_ASSEMBLE:
            REQUIRE_OK(driver_generate_asm_config(mem, symbols, config, externals, assembler_config));
            // Fallthrough

        case KEFIR_DRIVER_STAGE_PREPROCESS:
        case KEFIR_DRIVER_STAGE_PREPROCESS_SAVE:
        case KEFIR_DRIVER_STAGE_PRINT_TOKENS:
        case KEFIR_DRIVER_STAGE_PRINT_AST:
        case KEFIR_DRIVER_STAGE_PRINT_IR:
        case KEFIR_DRIVER_STAGE_COMPILE:
        case KEFIR_DRIVER_STAGE_PRINT_RUNTIME_CODE:
            REQUIRE_OK(driver_generate_compiler_config(mem, symbols, config, externals, compiler_config));
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
    } else if (config->stage == KEFIR_DRIVER_STAGE_PRINT_RUNTIME_CODE) {
        REQUIRE_OK(driver_print_runtime_code(config, compiler_config));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_driver_run(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
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
