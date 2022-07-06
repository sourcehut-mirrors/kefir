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

#include "kefir/driver/tools.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/string_array.h"
#include "kefir/util/process.h"
#include "kefir/driver/runner.h"
#include "kefir/compiler/compiler.h"
#include <stdio.h>

static int run_compiler(void *payload) {
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
