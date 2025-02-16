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

#include "kefir/codegen/amd64/module.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t on_function_free(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                       kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64_function *, function, value);
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));

    REQUIRE_OK(kefir_codegen_amd64_function_free(mem, function));
    memset(function, 0, sizeof(struct kefir_codegen_amd64_function));
    KEFIR_FREE(mem, function);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_module_init(struct kefir_codegen_amd64_module *module,
                                               struct kefir_codegen_amd64 *codegen, struct kefir_opt_module *opt_module,
                                               struct kefir_opt_module_analysis *analysis) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 codegen module"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(opt_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module analysis"));

    memset(module, 0, sizeof(struct kefir_codegen_amd64_module));

    module->codegen = codegen;
    module->module = opt_module;
    module->analysis = analysis;
    REQUIRE_OK(kefir_hashtree_init(&module->functions, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->functions, on_function_free, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_module_free(struct kefir_mem *mem, struct kefir_codegen_amd64_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));

    REQUIRE_OK(kefir_hashtree_free(mem, &module->functions));
    memset(module, 0, sizeof(struct kefir_codegen_amd64_module));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_module_insert_function(struct kefir_mem *mem,
                                                          struct kefir_codegen_amd64_module *module,
                                                          const struct kefir_opt_function *opt_function,
                                                          const struct kefir_opt_code_analysis *code_analysis,
                                                          struct kefir_codegen_amd64_function **function_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(opt_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(code_analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(function_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 codegen function"));

    struct kefir_codegen_amd64_function *function = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_amd64_function));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AMD64 codegen function"));

    kefir_result_t res =
        kefir_codegen_amd64_function_init(mem, function, module, module->module, opt_function, code_analysis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, function);
        return res;
    });

    res = kefir_hashtree_insert(mem, &module->functions, (kefir_hashtree_key_t) opt_function->ir_func->name,
                                (kefir_hashtree_value_t) function);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS,
                              "AMD64 codegen function with specified name already exists in the module");
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_amd64_function_free(mem, function);
        KEFIR_FREE(mem, function);
        return res;
    });

    *function_ptr = function;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_module_function(const struct kefir_codegen_amd64_module *module,
                                                   const char *function_name,
                                                   struct kefir_codegen_amd64_function **function_ptr) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(function_name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid function name"));
    REQUIRE(function_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 codegen function"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&module->functions, (kefir_hashtree_key_t) function_name, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested AMD64 codegen function");
    }
    REQUIRE_OK(res);

    *function_ptr = (struct kefir_codegen_amd64_function *) node->value;
    return KEFIR_OK;
}
