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

#include "kefir/main/driver.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/os_error.h"
#include "kefir/core/string_array.h"
#include "kefir/compiler/compiler.h"
#include "kefir/main/runner.h"

kefir_result_t kefir_driver_external_resources_init_from_env(struct kefir_driver_external_resources *externals) {
    REQUIRE(externals != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver external resources"));

    externals->assembler_path = getenv("KEFIR_AS");
    if (externals->assembler_path == NULL) {
        externals->assembler_path = getenv("AS");
    }
    if (externals->assembler_path == NULL) {
        externals->assembler_path = "as";
    }

    externals->linker_path = getenv("KEFIR_LD");
    if (externals->linker_path == NULL) {
        externals->linker_path = getenv("LD");
    }
    if (externals->linker_path == NULL) {
        externals->linker_path = "ld";
    }

    externals->runtime_library = getenv("KEFIR_RTLIB");
    REQUIRE(externals->runtime_library != NULL,
            KEFIR_SET_ERROR(KEFIR_ENVIRONMENT_ERROR, "Expected KEFIR_RTLIB to contain path to runtime library"));
    return KEFIR_OK;
}

static kefir_result_t list_entry_free(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                      void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    KEFIR_FREE(mem, entry->value);
    return KEFIR_OK;
}

kefir_result_t kefir_driver_assembler_configuration_init(struct kefir_driver_assembler_configuration *config) {
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));

    REQUIRE_OK(kefir_list_init(&config->extra_args));
    REQUIRE_OK(kefir_list_on_remove(&config->extra_args, list_entry_free, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_assembler_configuration_free(struct kefir_mem *mem,
                                                         struct kefir_driver_assembler_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));

    REQUIRE_OK(kefir_list_free(mem, &config->extra_args));
    return KEFIR_OK;
}

static kefir_result_t string_list_append(struct kefir_mem *mem, struct kefir_list *list, const char *string) {
    char *arg_copy = KEFIR_MALLOC(mem, strlen(string) + 1);
    REQUIRE(arg_copy != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate driver extra argument"));
    strcpy(arg_copy, string);
    kefir_result_t res = kefir_list_insert_after(mem, list, kefir_list_tail(list), arg_copy);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, arg_copy);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_driver_assembler_configuration_add_extra_argument(
    struct kefir_mem *mem, struct kefir_driver_assembler_configuration *config, const char *arg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler extra argument"));

    REQUIRE_OK(string_list_append(mem, &config->extra_args, arg));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_linker_configuration_init(struct kefir_driver_linker_configuration *config) {
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));

    REQUIRE_OK(kefir_list_init(&config->linked_files));
    REQUIRE_OK(kefir_list_on_remove(&config->linked_files, list_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->extra_args));
    REQUIRE_OK(kefir_list_on_remove(&config->extra_args, list_entry_free, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_linker_configuration_free(struct kefir_mem *mem,
                                                      struct kefir_driver_linker_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));

    REQUIRE_OK(kefir_list_free(mem, &config->linked_files));
    REQUIRE_OK(kefir_list_free(mem, &config->extra_args));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_linker_configuration_add_linked_file(struct kefir_mem *mem,
                                                                 struct kefir_driver_linker_configuration *config,
                                                                 const char *linked_file) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));
    REQUIRE(linked_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker file"));

    REQUIRE_OK(string_list_append(mem, &config->linked_files, linked_file));
    return KEFIR_OK;
}
kefir_result_t kefir_driver_linker_configuration_add_extra_argument(struct kefir_mem *mem,
                                                                    struct kefir_driver_linker_configuration *config,
                                                                    const char *arg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker extra argument"));

    REQUIRE_OK(string_list_append(mem, &config->extra_args, arg));
    return KEFIR_OK;
}

static int run_compiler(void *payload) {
    ASSIGN_DECL_CAST(const struct kefir_compiler_runner_configuration *, configuration, payload);
    kefir_result_t res = kefir_run_compiler(kefir_system_memalloc(), configuration);
    return kefir_report_error(stderr, res, configuration) ? EXIT_SUCCESS : EXIT_FAILURE;
}

kefir_result_t kefir_driver_run_compiler(const struct kefir_compiler_runner_configuration *configuration,
                                         struct kefir_process *process) {
    REQUIRE(configuration != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler runner configuration"));
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    REQUIRE_OK(kefir_process_run(process, run_compiler, (void *) configuration));
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

kefir_result_t kefir_driver_run_assembler(struct kefir_mem *mem, const char *output_file,
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

    REQUIRE_CHAIN(&res, copy_string_list_to_array(mem, &config->extra_args, &argv));

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

    REQUIRE_CHAIN(&res, copy_string_list_to_array(mem, &config->linked_files, &argv));
    REQUIRE_CHAIN(&res, copy_string_list_to_array(mem, &config->extra_args, &argv));
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, "-o"));
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, output));

    REQUIRE_CHAIN(&res, kefir_process_execute(process, resources->linker_path, argv.array));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_array_free(mem, &argv);
        return res;
    });

    REQUIRE_OK(kefir_string_array_free(mem, &argv));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_init(struct kefir_driver_configuration *config) {
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver configuration"));

    config->stage = KEFIR_DRIVER_STAGE_LINK;
    config->output_file = NULL;

    REQUIRE_OK(kefir_list_init(&config->input_files));
    REQUIRE_OK(kefir_list_on_remove(&config->input_files, list_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->assembler_flags));
    REQUIRE_OK(kefir_list_init(&config->linker_flags));
    REQUIRE_OK(kefir_list_on_remove(&config->linker_flags, list_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->defines));
    REQUIRE_OK(kefir_list_on_remove(&config->defines, list_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->undefines));
    REQUIRE_OK(kefir_list_init(&config->include_directories));
    REQUIRE_OK(kefir_list_init(&config->include_files));

    config->flags.strip = false;

    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_free(struct kefir_mem *mem, struct kefir_driver_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));

    REQUIRE_OK(kefir_list_free(mem, &config->input_files));
    REQUIRE_OK(kefir_list_free(mem, &config->assembler_flags));
    REQUIRE_OK(kefir_list_free(mem, &config->linker_flags));
    REQUIRE_OK(kefir_list_free(mem, &config->defines));
    REQUIRE_OK(kefir_list_free(mem, &config->undefines));
    REQUIRE_OK(kefir_list_free(mem, &config->include_directories));
    REQUIRE_OK(kefir_list_free(mem, &config->include_files));

    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_input(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                    struct kefir_driver_configuration *config, const char *file,
                                                    kefir_driver_input_file_type_t type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input file"));

    if (symbols != NULL) {
        file = kefir_symbol_table_insert(mem, symbols, file, NULL);
        REQUIRE(file != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert input file name into symbol table"));
    }

    struct kefir_driver_input_file *input_file = KEFIR_MALLOC(mem, sizeof(struct kefir_driver_input_file));
    REQUIRE(input_file != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate driver input file"));
    input_file->file = file;
    input_file->type = type;

    kefir_result_t res =
        kefir_list_insert_after(mem, &config->input_files, kefir_list_tail(&config->input_files), input_file);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, input_file);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_assembler_extra_flag(struct kefir_mem *mem,
                                                                   struct kefir_symbol_table *symbols,
                                                                   struct kefir_driver_configuration *config,
                                                                   const char *flag) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(flag != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid assembler flag"));

    if (symbols != NULL) {
        flag = kefir_symbol_table_insert(mem, symbols, flag, NULL);
        REQUIRE(flag != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert assembler flag into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->assembler_flags, kefir_list_tail(&config->assembler_flags),
                                       (void *) flag));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_linker_flag(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                          struct kefir_driver_configuration *config, const char *flag,
                                                          kefir_driver_linker_flag_type_t flag_type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(flag != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linker flag"));

    if (symbols != NULL) {
        flag = kefir_symbol_table_insert(mem, symbols, flag, NULL);
        REQUIRE(flag != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert linker flag into symbol table"));
    }

    struct kefir_driver_linker_flag *linker_flag = KEFIR_MALLOC(mem, sizeof(struct kefir_driver_linker_flag));
    REQUIRE(linker_flag != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate driver linker flag"));
    linker_flag->flag = flag;
    linker_flag->type = flag_type;

    kefir_result_t res =
        kefir_list_insert_after(mem, &config->linker_flags, kefir_list_tail(&config->linker_flags), linker_flag);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, linker_flag);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_define(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                     struct kefir_driver_configuration *config, const char *name,
                                                     const char *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid define name"));

    if (symbols != NULL) {
        name = kefir_symbol_table_insert(mem, symbols, name, NULL);
        REQUIRE(name != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert define name into symbol table"));

        if (value != NULL) {
            value = kefir_symbol_table_insert(mem, symbols, value, NULL);
            REQUIRE(value != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert define value into symbol table"));
        }
    }

    struct kefir_driver_definition *definition = KEFIR_MALLOC(mem, sizeof(struct kefir_driver_definition));
    REQUIRE(definition != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate driver define"));
    definition->name = name;
    definition->value = value;

    kefir_result_t res = kefir_list_insert_after(mem, &config->defines, kefir_list_tail(&config->defines), definition);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, definition);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_undefine(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                       struct kefir_driver_configuration *config, const char *name) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid undefine name"));

    if (symbols != NULL) {
        name = kefir_symbol_table_insert(mem, symbols, name, NULL);
        REQUIRE(name != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert undefine name into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->undefines, kefir_list_tail(&config->undefines), (void *) name));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_include_directory(struct kefir_mem *mem,
                                                                struct kefir_symbol_table *symbols,
                                                                struct kefir_driver_configuration *config,
                                                                const char *dir) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(dir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid include directory"));

    if (symbols != NULL) {
        dir = kefir_symbol_table_insert(mem, symbols, dir, NULL);
        REQUIRE(dir != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include directory into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->include_directories, kefir_list_tail(&config->include_directories),
                                       (void *) dir));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_include_file(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                           struct kefir_driver_configuration *config,
                                                           const char *file) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid include file"));

    if (symbols != NULL) {
        file = kefir_symbol_table_insert(mem, symbols, file, NULL);
        REQUIRE(file != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include file into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->include_directories, kefir_list_tail(&config->include_files),
                                       (void *) file));
    return KEFIR_OK;
}

static kefir_result_t driver_run_impl(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                      const struct kefir_driver_external_resources *externals,
                                      const struct kefir_compiler_runner_configuration *compiler_config,
                                      const struct kefir_driver_assembler_configuration *assembler_config,
                                      const struct kefir_driver_linker_configuration *linker_config) {
    UNUSED(mem);
    UNUSED(config);
    UNUSED(externals);
    UNUSED(compiler_config);
    UNUSED(assembler_config);
    UNUSED(linker_config);
    return KEFIR_OK;
}

kefir_result_t kefir_driver_run(struct kefir_mem *mem, struct kefir_driver_configuration *config,
                                const struct kefir_driver_external_resources *externals) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(externals != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver external resources"));

    struct kefir_compiler_runner_configuration compiler_config;
    struct kefir_driver_assembler_configuration assembler_config;
    struct kefir_driver_linker_configuration linker_config;

    REQUIRE_OK(kefir_compiler_runner_configuration_init(&compiler_config));
    REQUIRE_OK(kefir_driver_assembler_configuration_init(&assembler_config));
    REQUIRE_OK(kefir_driver_linker_configuration_init(&linker_config));

    kefir_result_t res = driver_run_impl(mem, config, externals, &compiler_config, &assembler_config, &linker_config);

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_compiler_runner_configuration_free(mem, &compiler_config);
        kefir_driver_assembler_configuration_free(mem, &assembler_config);
        kefir_driver_linker_configuration_free(mem, &linker_config);
        return res;
    });
    kefir_compiler_runner_configuration_free(mem, &compiler_config);
    kefir_driver_assembler_configuration_free(mem, &assembler_config);
    kefir_driver_linker_configuration_free(mem, &linker_config);
    return KEFIR_OK;
}
