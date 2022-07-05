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
#include "kefir/main/driver.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/os_error.h"
#include "kefir/core/string_array.h"
#include "kefir/compiler/compiler.h"
#include "kefir/main/runner.h"

static kefir_result_t list_string_entry_free(struct kefir_mem *mem, struct kefir_list *list,
                                             struct kefir_list_entry *entry, void *payload) {
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
    REQUIRE_OK(kefir_list_on_remove(&config->extra_args, list_string_entry_free, NULL));
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
    REQUIRE_OK(kefir_list_on_remove(&config->linked_files, list_string_entry_free, NULL));
    REQUIRE_OK(kefir_list_init(&config->extra_args));
    REQUIRE_OK(kefir_list_on_remove(&config->extra_args, list_string_entry_free, NULL));
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

kefir_result_t kefir_driver_run_assembler(struct kefir_mem *mem, const char *output_file,
                                          const struct kefir_driver_assembler_configuration *config,
                                          struct kefir_process *process) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(output_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output file"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    const char *as_path = getenv("KEFIR_AS");
    if (as_path == NULL) {
        as_path = getenv("AS");
    }
    if (as_path == NULL) {
        as_path = "as";
    }

    struct kefir_string_array argv;
    REQUIRE_OK(kefir_string_array_init(mem, &argv));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, as_path));
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, "-o"));
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, output_file));

    for (const struct kefir_list_entry *iter = kefir_list_head(&config->extra_args); res == KEFIR_OK && iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, extra_arg, iter->value);
        REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, extra_arg));
    }

    REQUIRE_CHAIN(&res, kefir_process_execute(process, as_path, argv.array));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_array_free(mem, &argv);
        return res;
    });

    REQUIRE_OK(kefir_string_array_free(mem, &argv));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_run_linker(struct kefir_mem *mem, const char *output,
                                       const struct kefir_driver_linker_configuration *config,
                                       struct kefir_process *process) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid linker output file"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver linker configuration"));
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    const char *ld_path = getenv("KEFIR_LD");
    if (ld_path == NULL) {
        ld_path = getenv("LD");
    }
    if (ld_path == NULL) {
        ld_path = "ld";
    }

    struct kefir_string_array argv;
    REQUIRE_OK(kefir_string_array_init(mem, &argv));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, ld_path));

    for (const struct kefir_list_entry *iter = kefir_list_head(&config->linked_files); res == KEFIR_OK && iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, linked_file, iter->value);
        REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, linked_file));
    }
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->extra_args); res == KEFIR_OK && iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, extra_arg, iter->value);
        REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, extra_arg));
    }
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, "-o"));
    REQUIRE_CHAIN(&res, kefir_string_array_append(mem, &argv, output));

    REQUIRE_CHAIN(&res, kefir_process_execute(process, ld_path, argv.array));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_array_free(mem, &argv);
        return res;
    });

    REQUIRE_OK(kefir_string_array_free(mem, &argv));
    return KEFIR_OK;
}
