/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "kefir/compiler/configuration.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

static kefir_result_t free_define_identifier(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                             kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(char *, identifier, key);
    ASSIGN_DECL_CAST(char *, id_value, value);
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));

    KEFIR_FREE(mem, identifier);
    if (id_value != NULL) {
        KEFIR_FREE(mem, id_value);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_compiler_runner_configuration_init(struct kefir_compiler_runner_configuration *options) {
    REQUIRE(options != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to cli options"));

    *options = (struct kefir_compiler_runner_configuration){
        .action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_ASSEMBLY,
        .error_report_type = KEFIR_COMPILER_RUNNER_ERROR_REPORT_TABULAR,
        .skip_preprocessor = false,
        .default_pp_timestamp = true,
        .verbose = false,
        .features = {false},
        .codegen = {.emulated_tls = false,
                    .position_independent_code = false,
                    .omit_frame_pointer = false,
                    .syntax = NULL,
                    .print_details = NULL,
                    .pipeline_spec = NULL},
        .optimizer_pipeline_spec = NULL,
        .dependency_output = {.target_name = NULL, .output_system_deps = true}};
    REQUIRE_OK(kefir_list_init(&options->include_path));
    REQUIRE_OK(kefir_hashtreeset_init(&options->system_include_directories, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_list_init(&options->include_files));
    REQUIRE_OK(kefir_hashtree_init(&options->defines, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&options->defines, free_define_identifier, NULL));
    REQUIRE_OK(kefir_list_init(&options->undefines));
    return KEFIR_OK;
}

kefir_result_t kefir_compiler_runner_configuration_free(struct kefir_mem *mem,
                                                        struct kefir_compiler_runner_configuration *options) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(options != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to cli options"));

    REQUIRE_OK(kefir_list_free(mem, &options->include_files));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &options->system_include_directories));
    REQUIRE_OK(kefir_list_free(mem, &options->include_path));
    REQUIRE_OK(kefir_hashtree_free(mem, &options->defines));
    REQUIRE_OK(kefir_list_free(mem, &options->undefines));
    return KEFIR_OK;
}

kefir_result_t kefir_compiler_runner_configuration_define(struct kefir_mem *mem,
                                                          struct kefir_compiler_runner_configuration *options,
                                                          const char *identifier, const char *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(options != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to cli options"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid macro identifier"));

    if (kefir_hashtree_has(&options->defines, (kefir_hashtree_key_t) identifier)) {
        REQUIRE_OK(kefir_hashtree_delete(mem, &options->defines, (kefir_hashtree_key_t) identifier));
    }

    kefir_size_t identifier_length = strlen(identifier);
    char *identifier_copy = KEFIR_MALLOC(mem, identifier_length + 1);
    REQUIRE(identifier_copy != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate definition name copy"));
    strcpy(identifier_copy, identifier);

    char *value_copy = NULL;
    if (value != NULL) {
        value_copy = KEFIR_MALLOC(mem, strlen(value) + 1);
        REQUIRE_ELSE(value_copy != NULL, {
            KEFIR_FREE(mem, identifier_copy);
            return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate definition value copy");
        });
        strcpy(value_copy, value);
    }

    kefir_result_t res = kefir_hashtree_insert(mem, &options->defines, (kefir_hashtree_key_t) identifier_copy,
                                               (kefir_hashtree_value_t) value_copy);
    REQUIRE_ELSE(res == KEFIR_OK, {
        if (value_copy != NULL) {
            KEFIR_FREE(mem, value_copy);
        }
        KEFIR_FREE(mem, identifier_copy);
        return res;
    });
    return KEFIR_OK;
}
