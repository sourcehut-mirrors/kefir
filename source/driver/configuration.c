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

#include "kefir/driver/configuration.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

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

    config->verbose = false;
    REQUIRE_OK(kefir_list_init(&config->arguments));
    REQUIRE_OK(kefir_list_on_remove(&config->arguments, list_entry_free, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_assembler_configuration_free(struct kefir_mem *mem,
                                                         struct kefir_driver_assembler_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));

    REQUIRE_OK(kefir_list_free(mem, &config->arguments));
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

kefir_result_t kefir_driver_assembler_configuration_add_argument(struct kefir_mem *mem,
                                                                 struct kefir_driver_assembler_configuration *config,
                                                                 const char *arg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler extra argument"));

    REQUIRE_OK(string_list_append(mem, &config->arguments, arg));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_linker_configuration_init(struct kefir_driver_linker_configuration *config) {
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));

    REQUIRE_OK(kefir_list_init(&config->arguments));
    REQUIRE_OK(kefir_list_on_remove(&config->arguments, list_entry_free, NULL));

    config->flags.static_linking = false;
    config->flags.shared_linking = false;
    config->flags.pie_linking = false;
    config->flags.link_start_files = true;
    config->flags.link_default_libs = true;
    config->flags.link_libc = true;
    config->flags.link_atomics = true;
    config->flags.verbose = false;
    return KEFIR_OK;
}

kefir_result_t kefir_driver_linker_configuration_free(struct kefir_mem *mem,
                                                      struct kefir_driver_linker_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));

    REQUIRE_OK(kefir_list_free(mem, &config->arguments));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_linker_configuration_add_argument(struct kefir_mem *mem,
                                                              struct kefir_driver_linker_configuration *config,
                                                              const char *arg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker extra argument"));

    REQUIRE_OK(string_list_append(mem, &config->arguments, arg));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_init(struct kefir_driver_configuration *config) {
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver configuration"));

    config->stage = KEFIR_DRIVER_STAGE_LINK;
    config->output_file = NULL;
    config->standard_version = KEFIR_DEFAULT_STANDARD_VERSION;

    REQUIRE_OK(kefir_list_init(&config->arguments));
    REQUIRE_OK(kefir_list_on_remove(&config->arguments, list_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->assembler_arguments));
    REQUIRE_OK(kefir_list_init(&config->compiler_arguments));
    REQUIRE_OK(kefir_hashtree_init(&config->defines, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_list_init(&config->undefines));
    REQUIRE_OK(kefir_list_init(&config->include_directories));
    REQUIRE_OK(kefir_list_init(&config->quote_include_directories));
    REQUIRE_OK(kefir_list_init(&config->system_include_directories));
    REQUIRE_OK(kefir_list_init(&config->after_include_directories));
    REQUIRE_OK(kefir_list_init(&config->embed_directories));
    REQUIRE_OK(kefir_list_init(&config->include_files));
    REQUIRE_OK(kefir_driver_target_default(&config->target));
    REQUIRE_OK(kefir_list_init(&config->run.args));

    config->compiler.optimization_level = -1;
    config->compiler.char_signedness = KEFIR_DRIVER_CHAR_SIGNEDNESS_DEFAULT;

    config->assembler.target = KEFIR_DRIVER_ASSEMBLER_GAS_ATT;

    config->flags.skip_preprocessor = false;
    config->flags.restrictive_mode = false;
    config->flags.static_linking = false;
    config->flags.shared_linking = false;
    config->flags.export_dynamic = false;
    config->flags.position_independent_code = true;
    config->flags.position_independent_executable = false;
    config->flags.debug_info = false;
    config->flags.omit_frame_pointer = KEFIR_DRIVER_FRAME_POINTER_OMISSION_UNSPECIFIED;
    config->flags.link_start_files = true;
    config->flags.link_default_libs = true;
    config->flags.include_stdinc = true;
    config->flags.link_libc = true;
    config->flags.include_rtinc = true;
    config->flags.enable_atomics = true;
    config->flags.verbose = false;
    config->flags.pthread = false;

    config->dependency_output.output_dependencies = false;
    config->dependency_output.output_system_deps = true;
    config->dependency_output.add_phony_targets = false;
    config->dependency_output.target_name = NULL;
    config->dependency_output.output_filename = NULL;

    config->run.file_stdin = NULL;
    config->run.file_stdout = NULL;
    config->run.stderr_to_stdout = false;
    config->run.file_stderr = NULL;

    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_free(struct kefir_mem *mem, struct kefir_driver_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));

    REQUIRE_OK(kefir_list_free(mem, &config->run.args));
    REQUIRE_OK(kefir_list_free(mem, &config->arguments));
    REQUIRE_OK(kefir_list_free(mem, &config->assembler_arguments));
    REQUIRE_OK(kefir_list_free(mem, &config->compiler_arguments));
    REQUIRE_OK(kefir_hashtree_free(mem, &config->defines));
    REQUIRE_OK(kefir_list_free(mem, &config->undefines));
    REQUIRE_OK(kefir_list_free(mem, &config->include_directories));
    REQUIRE_OK(kefir_list_free(mem, &config->quote_include_directories));
    REQUIRE_OK(kefir_list_free(mem, &config->system_include_directories));
    REQUIRE_OK(kefir_list_free(mem, &config->after_include_directories));
    REQUIRE_OK(kefir_list_free(mem, &config->embed_directories));
    REQUIRE_OK(kefir_list_free(mem, &config->include_files));

    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_argument(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                       struct kefir_driver_configuration *config, const char *file,
                                                       kefir_driver_argument_type_t type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid argument"));

    if (symbols != NULL) {
        file = kefir_string_pool_insert(mem, symbols, file, NULL);
        REQUIRE(file != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert driver argument name into symbol table"));
    }

    struct kefir_driver_argument *argument = KEFIR_MALLOC(mem, sizeof(struct kefir_driver_argument));
    REQUIRE(argument != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate driver argument"));
    argument->value = file;
    argument->type = type;

    kefir_result_t res =
        kefir_list_insert_after(mem, &config->arguments, kefir_list_tail(&config->arguments), argument);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, argument);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_assembler_argument(struct kefir_mem *mem,
                                                                 struct kefir_string_pool *symbols,
                                                                 struct kefir_driver_configuration *config,
                                                                 const char *argument) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(argument != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid assembler argument"));

    if (symbols != NULL) {
        argument = kefir_string_pool_insert(mem, symbols, argument, NULL);
        REQUIRE(argument != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert assembler argument into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->assembler_arguments, kefir_list_tail(&config->assembler_arguments),
                                       (void *) argument));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_compiler_argument(struct kefir_mem *mem,
                                                                struct kefir_string_pool *symbols,
                                                                struct kefir_driver_configuration *config,
                                                                const char *argument) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(argument != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler argument"));

    if (symbols != NULL) {
        argument = kefir_string_pool_insert(mem, symbols, argument, NULL);
        REQUIRE(argument != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert compiler argument into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->compiler_arguments, kefir_list_tail(&config->compiler_arguments),
                                       (void *) argument));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_define(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                     struct kefir_driver_configuration *config, const char *name,
                                                     const char *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid define name"));

    if (symbols != NULL) {
        name = kefir_string_pool_insert(mem, symbols, name, NULL);
        REQUIRE(name != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert define name into symbol table"));

        if (value != NULL) {
            value = kefir_string_pool_insert(mem, symbols, value, NULL);
            REQUIRE(value != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert define value into symbol table"));
        }
    }

    if (kefir_hashtree_has(&config->defines, (kefir_hashtree_key_t) name)) {
        REQUIRE_OK(kefir_hashtree_delete(mem, &config->defines, (kefir_hashtree_key_t) name));
    }
    REQUIRE_OK(
        kefir_hashtree_insert(mem, &config->defines, (kefir_hashtree_key_t) name, (kefir_hashtree_value_t) value));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_undefine(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                       struct kefir_driver_configuration *config, const char *name) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid undefine name"));

    if (symbols != NULL) {
        name = kefir_string_pool_insert(mem, symbols, name, NULL);
        REQUIRE(name != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert undefine name into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->undefines, kefir_list_tail(&config->undefines), (void *) name));
    if (kefir_hashtree_has(&config->defines, (kefir_hashtree_key_t) name)) {
        REQUIRE_OK(kefir_hashtree_delete(mem, &config->defines, (kefir_hashtree_key_t) name));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_include_directory(struct kefir_mem *mem,
                                                                struct kefir_string_pool *symbols,
                                                                struct kefir_driver_configuration *config,
                                                                const char *dir) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(dir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid include directory"));

    if (symbols != NULL) {
        dir = kefir_string_pool_insert(mem, symbols, dir, NULL);
        REQUIRE(dir != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include directory into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->include_directories, kefir_list_tail(&config->include_directories),
                                       (void *) dir));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_system_include_directory(struct kefir_mem *mem,
                                                                       struct kefir_string_pool *symbols,
                                                                       struct kefir_driver_configuration *config,
                                                                       const char *dir) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(dir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid include directory"));

    if (symbols != NULL) {
        dir = kefir_string_pool_insert(mem, symbols, dir, NULL);
        REQUIRE(dir != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include directory into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->system_include_directories,
                                       kefir_list_tail(&config->system_include_directories), (void *) dir));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_quote_include_directory(struct kefir_mem *mem,
                                                                      struct kefir_string_pool *symbols,
                                                                      struct kefir_driver_configuration *config,
                                                                      const char *dir) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(dir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid include directory"));

    if (symbols != NULL) {
        dir = kefir_string_pool_insert(mem, symbols, dir, NULL);
        REQUIRE(dir != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include directory into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->quote_include_directories,
                                       kefir_list_tail(&config->quote_include_directories), (void *) dir));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_after_include_directory(struct kefir_mem *mem,
                                                                      struct kefir_string_pool *symbols,
                                                                      struct kefir_driver_configuration *config,
                                                                      const char *dir) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(dir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid include directory"));

    if (symbols != NULL) {
        dir = kefir_string_pool_insert(mem, symbols, dir, NULL);
        REQUIRE(dir != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include directory into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->after_include_directories,
                                       kefir_list_tail(&config->after_include_directories), (void *) dir));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_embed_directory(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                              struct kefir_driver_configuration *config,
                                                              const char *dir) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(dir != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid include directory"));

    if (symbols != NULL) {
        dir = kefir_string_pool_insert(mem, symbols, dir, NULL);
        REQUIRE(dir != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include directory into symbol table"));
    }

    REQUIRE_OK(kefir_list_insert_after(mem, &config->embed_directories, kefir_list_tail(&config->embed_directories),
                                       (void *) dir));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_add_include_file(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                           struct kefir_driver_configuration *config,
                                                           const char *file) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid include file"));

    if (symbols != NULL) {
        file = kefir_string_pool_insert(mem, symbols, file, NULL);
        REQUIRE(file != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert include file into symbol table"));
    }

    REQUIRE_OK(
        kefir_list_insert_after(mem, &config->include_files, kefir_list_tail(&config->include_files), (void *) file));
    return KEFIR_OK;
}
