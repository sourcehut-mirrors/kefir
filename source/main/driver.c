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
#include "kefir/compiler/compiler.h"
#include "kefir/main/runner.h"

static kefir_result_t extra_args_free(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
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
    REQUIRE_OK(kefir_list_on_remove(&config->extra_args, extra_args_free, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_assembler_configuration_free(struct kefir_mem *mem,
                                                         struct kefir_driver_assembler_configuration *config) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));

    REQUIRE_OK(kefir_list_free(mem, &config->extra_args));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_assembler_configuration_add_extra_argument(
    struct kefir_mem *mem, struct kefir_driver_assembler_configuration *config, const char *arg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler configuration"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver assembler extra argument"));

    char *arg_copy = KEFIR_MALLOC(mem, strlen(arg) + 1);
    REQUIRE(arg_copy != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate driver assembler extra argument"));
    strcpy(arg_copy, arg);
    kefir_result_t res =
        kefir_list_insert_after(mem, &config->extra_args, kefir_list_tail(&config->extra_args), arg_copy);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, arg_copy);
        return res;
    });
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

static void free_argv(struct kefir_mem *mem, char **argv, kefir_size_t index) {
    for (kefir_size_t j = 0; j < index; j++) {
        KEFIR_FREE(mem, argv[j]);
    }
    KEFIR_FREE(mem, argv);
}

kefir_result_t kefir_driver_run_assembler(struct kefir_mem *mem, const char *output_file,
                                          const struct kefir_driver_assembler_configuration *config,
                                          struct kefir_process *process) {
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

    const char *base_argv[] = {as_path, "-o", output_file};
    kefir_size_t base_argc = sizeof(base_argv) / sizeof(base_argv[0]);

    kefir_size_t argc = base_argc + kefir_list_length(&config->extra_args);
    char **argv = KEFIR_MALLOC(mem, sizeof(char *) * (argc + 1));
    REQUIRE(argv != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate assembler command line arguments"));
    kefir_size_t i = 0;
    for (; i < base_argc; i++) {
        kefir_size_t arg_len = strlen(base_argv[i]);
        argv[i] = KEFIR_MALLOC(mem, arg_len + 1);
        REQUIRE_ELSE(argv[i] != NULL, {
            free_argv(mem, argv, i);
            return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate assembler command line arguments");
        });
        strcpy(argv[i], base_argv[i]);
    }
    for (const struct kefir_list_entry *iter = kefir_list_head(&config->extra_args); iter != NULL;
         kefir_list_next(&iter), i++) {
        ASSIGN_DECL_CAST(const char *, extra_arg, iter->value);
        kefir_size_t arg_len = strlen(extra_arg);
        argv[i] = KEFIR_MALLOC(mem, arg_len + 1);
        REQUIRE_ELSE(argv[i] != NULL, {
            free_argv(mem, argv, i);
            return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate assembler command line arguments");
        });
        strcpy(argv[i], extra_arg);
    }
    argv[i] = NULL;

    kefir_result_t res = kefir_process_execute(process, as_path, argv);
    REQUIRE_ELSE(res == KEFIR_OK, {
        free_argv(mem, argv, i);
        return res;
    });

    free_argv(mem, argv, i);
    return KEFIR_OK;
}
