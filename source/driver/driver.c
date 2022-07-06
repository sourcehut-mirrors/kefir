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

#include "kefir/driver/driver.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t driver_generate_asm_config(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                                 struct kefir_driver_assembler_configuration *assembler_config) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->assembler_flags); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, flag, iter->value);
        REQUIRE_OK(kefir_list_insert_after(mem, &assembler_config->extra_args,
                                           kefir_list_tail(&assembler_config->extra_args), (void *) flag));
    }
    return KEFIR_OK;
}

static kefir_result_t driver_generate_linker_config(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                                    struct kefir_driver_linker_configuration *linker_config) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->linker_flags); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, flag, iter->value);
        REQUIRE_OK(kefir_list_insert_after(mem, &linker_config->extra_args, kefir_list_tail(&linker_config->extra_args),
                                           (void *) flag));
    }
    return KEFIR_OK;
}

static kefir_result_t driver_generate_compiler_config(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                                      struct kefir_compiler_runner_configuration *compiler_config,
                                                      struct kefir_driver_input_file *input_file) {
    REQUIRE_OK(kefir_compiler_runner_configuration_init(compiler_config));

    compiler_config->input_filepath = input_file->file;
    compiler_config->source_id = input_file->file;
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->include_directories); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, include_dir, iter->value);
        REQUIRE_OK(kefir_list_insert_after(mem, &compiler_config->include_path,
                                           kefir_list_tail(&compiler_config->include_path), (void *) include_dir));
    }
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->include_files); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, include_file, iter->value);
        REQUIRE_OK(kefir_list_insert_after(mem, &compiler_config->include_files,
                                           kefir_list_tail(&compiler_config->include_files), (void *) include_file));
    }
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->defines); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_driver_definition *, definition, iter->value);
        REQUIRE_OK(kefir_hashtree_insert(mem, &compiler_config->defines, (kefir_hashtree_key_t) definition->name,
                                         (kefir_hashtree_value_t) definition->value));
    }
    return KEFIR_OK;
}

static kefir_result_t driver_run_input_file(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                            const struct kefir_driver_external_resources *externals,
                                            struct kefir_driver_assembler_configuration *assembler_config,
                                            struct kefir_driver_linker_configuration *linker_config,
                                            struct kefir_driver_input_file *input_file) {
    struct kefir_process compiler_process, assembler_process;
    switch (config->stage) {
        case KEFIR_DRIVER_STAGE_LINK:
            switch (input_file->type) {
                case KEFIR_DRIVER_INPUT_FILE_CODE:
                case KEFIR_DRIVER_INPUT_FILE_PREPROCESSED: {
                    struct kefir_compiler_runner_configuration compiler_config;
                    REQUIRE_OK(driver_generate_compiler_config(mem, config, &compiler_config, input_file));

                    REQUIRE_OK(kefir_process_init(&compiler_process));
                    REQUIRE_OK(kefir_process_init(&assembler_process));

                    REQUIRE_OK(kefir_process_pipe(&compiler_process, &assembler_process));

                    char object_filename_template[1025];
                    strcpy(object_filename_template, externals->work_dir);
                    strcat(object_filename_template, "/object-file-XXXXXXX");
                    const char *object_filename;
                    REQUIRE_OK(kefir_tempfile_manager_create_file(mem, externals->tmpfile_manager,
                                                                  object_filename_template, &object_filename));
                    REQUIRE_OK(kefir_driver_linker_configuration_add_linked_file(mem, linker_config, object_filename));

                    REQUIRE_OK(kefir_driver_run_compiler(&compiler_config, &compiler_process));
                    REQUIRE_OK(kefir_driver_run_assembler(mem, object_filename, assembler_config, externals,
                                                          &assembler_process));

                    REQUIRE_OK(kefir_process_wait(&compiler_process));
                    REQUIRE_OK(kefir_process_wait(&assembler_process));

                    REQUIRE_OK(kefir_compiler_runner_configuration_free(mem, &compiler_config));
                } break;

                case KEFIR_DRIVER_INPUT_FILE_ASSEMBLY:
                    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Assembly files are not supported by the driver yet");

                case KEFIR_DRIVER_INPUT_FILE_OBJECT:
                case KEFIR_DRIVER_INPUT_FILE_LIBRARY:
                    REQUIRE_OK(kefir_driver_linker_configuration_add_linked_file(mem, linker_config, input_file->file));
                    break;
            }
            break;

        case KEFIR_DRIVER_STAGE_ASSEMBLE:
        case KEFIR_DRIVER_STAGE_PREPROCESS:
        case KEFIR_DRIVER_STAGE_PREPROCESS_SAVE:
        case KEFIR_DRIVER_STAGE_COMPILE:
            return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Requested driver stage is not supported yet");
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t driver_run_impl(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                      const struct kefir_driver_external_resources *externals,
                                      struct kefir_driver_assembler_configuration *assembler_config,
                                      struct kefir_driver_linker_configuration *linker_config) {
    UNUSED(externals);

    switch (config->stage) {
        case KEFIR_DRIVER_STAGE_LINK:
            REQUIRE_OK(driver_generate_linker_config(mem, config, linker_config));
            // Fallthrough

        case KEFIR_DRIVER_STAGE_ASSEMBLE:
            REQUIRE_OK(driver_generate_asm_config(mem, config, assembler_config));
            break;

        case KEFIR_DRIVER_STAGE_PREPROCESS:
        case KEFIR_DRIVER_STAGE_PREPROCESS_SAVE:
        case KEFIR_DRIVER_STAGE_COMPILE:
            // Intentionally left blank
            break;
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&config->input_files); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_driver_input_file *, input_file, iter->value);
        REQUIRE_OK(driver_run_input_file(mem, config, externals, assembler_config, linker_config, input_file));
    }

    if (config->stage == KEFIR_DRIVER_STAGE_LINK) {
        struct kefir_process linker_process;
        REQUIRE_OK(kefir_process_init(&linker_process));
        const char *output_file = config->output_file != NULL ? config->output_file : "a.out";
        REQUIRE_OK(kefir_driver_run_linker(mem, output_file, linker_config, externals, &linker_process));
        REQUIRE_OK(kefir_process_wait(&linker_process));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_driver_run(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                const struct kefir_driver_external_resources *externals) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(externals != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver external resources"));

    struct kefir_driver_assembler_configuration assembler_config;
    struct kefir_driver_linker_configuration linker_config;

    REQUIRE_OK(kefir_driver_assembler_configuration_init(&assembler_config));
    REQUIRE_OK(kefir_driver_linker_configuration_init(&linker_config));

    kefir_result_t res = driver_run_impl(mem, config, externals, &assembler_config, &linker_config);

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
