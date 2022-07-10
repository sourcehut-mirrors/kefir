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

kefir_result_t kefir_driver_configuration_init(struct kefir_driver_configuration *config) {
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver configuration"));

    config->stage = KEFIR_DRIVER_STAGE_LINK;
    config->output_file = NULL;

    REQUIRE_OK(kefir_list_init(&config->input_files));
    REQUIRE_OK(kefir_list_on_remove(&config->input_files, list_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->assembler_flags));
    REQUIRE_OK(kefir_list_init(&config->linker_flags));
    REQUIRE_OK(kefir_list_init(&config->compiler_flags));
    REQUIRE_OK(kefir_list_on_remove(&config->linker_flags, list_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->defines));
    REQUIRE_OK(kefir_list_on_remove(&config->defines, list_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->undefines));
    REQUIRE_OK(kefir_list_init(&config->include_directories));
    REQUIRE_OK(kefir_list_init(&config->include_files));
    REQUIRE_OK(kefir_driver_target_default(&config->target));

    return KEFIR_OK;
}

kefir_result_t kefir_driver_configuration_free(struct kefir_mem *mem, struct kefir_driver_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));

    REQUIRE_OK(kefir_list_free(mem, &config->input_files));
    REQUIRE_OK(kefir_list_free(mem, &config->assembler_flags));
    REQUIRE_OK(kefir_list_free(mem, &config->linker_flags));
    REQUIRE_OK(kefir_list_free(mem, &config->compiler_flags));
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

kefir_result_t kefir_driver_configuration_add_compiler_flag(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                            struct kefir_driver_configuration *config,
                                                            const char *flag) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(flag != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler flag"));

    if (symbols != NULL) {
        flag = kefir_symbol_table_insert(mem, symbols, flag, NULL);
        REQUIRE(flag != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert compiler flag into symbol table"));
    }

    REQUIRE_OK(
        kefir_list_insert_after(mem, &config->compiler_flags, kefir_list_tail(&config->compiler_flags), (void *) flag));
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

    REQUIRE_OK(
        kefir_list_insert_after(mem, &config->include_files, kefir_list_tail(&config->include_files), (void *) file));
    return KEFIR_OK;
}
