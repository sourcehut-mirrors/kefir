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

#include "kefir/optimizer/module.h"
#include "kefir/optimizer/structure.h"
#include "kefir/optimizer/constructor.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_type_descriptor(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                           kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_type_descriptor *, type_descr, value);
    REQUIRE(type_descr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer type descriptor"));

    REQUIRE_OK(kefir_opt_type_descriptor_free(mem, type_descr));
    KEFIR_FREE(mem, type_descr);
    return KEFIR_OK;
}

static kefir_result_t free_function(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                    kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_function *, func, value);
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    REQUIRE_OK(kefir_opt_function_free(mem, func));
    KEFIR_FREE(mem, func);
    return KEFIR_OK;
}

static kefir_result_t add_type_descr(struct kefir_mem *mem, struct kefir_opt_module *module,
                                     const struct kefir_ir_target_platform *target_platform, kefir_id_t ir_type_id,
                                     const struct kefir_ir_type *ir_type) {
    struct kefir_opt_type_descriptor *descr = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_type_descriptor));
    REQUIRE(descr != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer type descriptor"));

    kefir_result_t res = kefir_opt_type_descriptor_init(mem, target_platform, ir_type_id, ir_type, descr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, descr);
        return res;
    });

    res = kefir_hashtree_insert(mem, &module->type_descriptors, (kefir_hashtree_key_t) ir_type_id,
                                (kefir_hashtree_value_t) descr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_type_descriptor_free(mem, descr);
        KEFIR_FREE(mem, descr);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t add_func(struct kefir_mem *mem, struct kefir_opt_module *module,
                               const struct kefir_ir_function *ir_func) {
    struct kefir_opt_function *func = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_function));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer function"));

    kefir_result_t res = kefir_opt_function_init(module, ir_func, func);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, func);
        return res;
    });

    res = kefir_hashtree_insert(mem, &module->functions, (kefir_hashtree_key_t) func->ir_func->declaration->id,
                                (kefir_hashtree_value_t) func);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_function_free(mem, func);
        KEFIR_FREE(mem, func);
        return res;
    });

    REQUIRE_OK(kefir_opt_construct_function_code(mem, module, func));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_init(struct kefir_mem *mem, struct kefir_ir_module *ir_module,
                                     struct kefir_opt_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer module"));

    REQUIRE_OK(kefir_hashtree_init(&module->type_descriptors, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->type_descriptors, free_type_descriptor, NULL));

    REQUIRE_OK(kefir_hashtree_init(&module->functions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&module->functions, free_function, NULL));

    module->ir_module = ir_module;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_construct(struct kefir_mem *mem, const struct kefir_ir_target_platform *target_platform,
                                          struct kefir_opt_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target_platform != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR target platform"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer module"));

    struct kefir_hashtree_node_iterator iter;

    kefir_id_t ir_type_id;
    for (const struct kefir_ir_type *ir_type = kefir_ir_module_named_type_iter(module->ir_module, &iter, &ir_type_id);
         ir_type != NULL; ir_type = kefir_ir_module_named_type_next(&iter, &ir_type_id)) {

        REQUIRE_OK(add_type_descr(mem, module, target_platform, ir_type_id, ir_type));
    }

    for (const struct kefir_ir_function *ir_func = kefir_ir_module_function_iter(module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {

        REQUIRE_OK(add_func(mem, module, ir_func));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_free(struct kefir_mem *mem, struct kefir_opt_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer module"));

    REQUIRE_OK(kefir_hashtree_free(mem, &module->type_descriptors));
    REQUIRE_OK(kefir_hashtree_free(mem, &module->functions));

    module->ir_module = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_get_type(const struct kefir_opt_module *module, kefir_id_t ir_type_id,
                                         const struct kefir_opt_type_descriptor **type_descr_ptr) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer module"));
    REQUIRE(type_descr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer type descriptor"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&module->type_descriptors, (kefir_hashtree_key_t) ir_type_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer type descriptor in the registry");
    }
    REQUIRE_OK(res);

    *type_descr_ptr = (const struct kefir_opt_type_descriptor *) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_get_function(const struct kefir_opt_module *module, kefir_id_t identifier,
                                             struct kefir_opt_function **function_ptr) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer module"));
    REQUIRE(function_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer function"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&module->functions, (kefir_hashtree_key_t) identifier, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer type descriptor in the registry");
    }
    REQUIRE_OK(res);

    *function_ptr = (struct kefir_opt_function *) node->value;
    return KEFIR_OK;
}
