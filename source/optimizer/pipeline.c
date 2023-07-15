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

#define KEFIR_OPTIMIZER_PIPELINE_INTERNAL
#include "kefir/optimizer/pipeline.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_optimizer_pass_resolve(const char *name, const struct kefir_optimizer_pass **pass_ptr) {
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer pass name"));
    REQUIRE(pass_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer pass"));

    *pass_ptr = NULL;

#define PASS(_id)                                              \
    do {                                                       \
        if (strcmp(KefirOptimizerPass##_id.name, name) == 0) { \
            *pass_ptr = &KefirOptimizerPass##_id;              \
            return KEFIR_OK;                                   \
        }                                                      \
    } while (0)

    PASS(Noop);
    PASS(CompareBranchFuse);
    PASS(OpSimplify);
    PASS(ConstFold);
#undef PASS
    return KEFIR_SET_ERRORF(KEFIR_NOT_FOUND, "Unable to find optimizer pass '%s'", name);
}

kefir_result_t kefir_optimizer_pipeline_init(struct kefir_optimizer_pipeline *pipeline) {
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer pipeline"));

    REQUIRE_OK(kefir_list_init(&pipeline->pipeline));
    return KEFIR_OK;
}

kefir_result_t kefir_optimizer_pipeline_free(struct kefir_mem *mem, struct kefir_optimizer_pipeline *pipeline) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer pipeline"));

    REQUIRE_OK(kefir_list_free(mem, &pipeline->pipeline));
    return KEFIR_OK;
}

kefir_result_t kefir_optimizer_pipeline_add(struct kefir_mem *mem, struct kefir_optimizer_pipeline *pipeline,
                                            const struct kefir_optimizer_pass *pass) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer pipeline"));
    REQUIRE(pass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer pipeline pass"));

    REQUIRE_OK(kefir_list_insert_after(mem, &pipeline->pipeline, kefir_list_tail(&pipeline->pipeline), (void *) pass));
    return KEFIR_OK;
}

kefir_result_t kefir_optimizer_pipeline_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                              const struct kefir_optimizer_pipeline *pipeline) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer pipeline"));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *ir_func = kefir_ir_module_function_iter(module->ir_module, &iter);
         ir_func != NULL; ir_func = kefir_ir_module_function_next(&iter)) {

        REQUIRE_OK(kefir_optimizer_pipeline_apply_function(mem, module, ir_func->declaration->id, pipeline));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_optimizer_pipeline_apply_function(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                       kefir_id_t function_id,
                                                       const struct kefir_optimizer_pipeline *pipeline) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer pipeline"));

    struct kefir_opt_function *func;
    REQUIRE_OK(kefir_opt_module_get_function(module, function_id, &func));

    for (const struct kefir_list_entry *iter = kefir_list_head(&pipeline->pipeline); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_optimizer_pass *, pass, iter->value);

        REQUIRE_OK(pass->apply(mem, module, func, pass));
    }
    return KEFIR_OK;
}
