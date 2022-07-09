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

#include "kefir/compiler/configuration.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t free_define_identifier(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                             kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(value);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(char *, identifier, key);
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));

    KEFIR_FREE(mem, identifier);
    return KEFIR_OK;
}

kefir_result_t kefir_compiler_runner_configuration_init(struct kefir_compiler_runner_configuration *options) {
    REQUIRE(options != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to cli options"));

    *options =
        (struct kefir_compiler_runner_configuration){.action = KEFIR_COMPILER_RUNNER_ACTION_DUMP_ASSEMBLY,
                                                     .error_report_type = KEFIR_COMPILER_RUNNER_ERROR_REPORT_TABULAR,
                                                     .skip_preprocessor = false,
                                                     .default_pp_timestamp = true,
                                                     .features = {false},
                                                     .codegen = {false}};
    REQUIRE_OK(kefir_list_init(&options->include_path));
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
    REQUIRE_OK(kefir_list_free(mem, &options->include_path));
    REQUIRE_OK(kefir_hashtree_free(mem, &options->defines));
    REQUIRE_OK(kefir_list_free(mem, &options->undefines));
    return KEFIR_OK;
}
