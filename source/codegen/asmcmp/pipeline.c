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

#define KEFIR_CODEGEN_ASMCMP_PIPELINE_INTERNAL
#include "kefir/codegen/asmcmp/pipeline.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static const struct kefir_asmcmp_pipeline_pass *Passes[] = {
#define REF_PASS(_id) &KefirAsmcmp##_id##Pass
    KEFIR_CODEGEN_ASMCMP_PIPELINE_PASSES(REF_PASS, COMMA)
#undef REF_PASS
};

kefir_result_t kefir_asmcmp_pipeline_pass_resolve(const char *name,
                                                  const struct kefir_asmcmp_pipeline_pass **pass_ptr) {
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp pipeline pass name"));
    REQUIRE(pass_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp pipeline pass"));

    for (kefir_size_t i = 0; i < sizeof(Passes) / sizeof(Passes[0]); i++) {
        if (strcmp(name, Passes[i]->name) == 0) {
            *pass_ptr = Passes[i];
            return KEFIR_OK;
        }
    }
    return KEFIR_SET_ERRORF(KEFIR_NOT_FOUND, "Unable to find requested pipeline pass '%s'", name);
}

kefir_result_t kefir_asmcmp_pipeline_init(struct kefir_asmcmp_pipeline *pipeline) {
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp pipeline"));

    REQUIRE_OK(kefir_list_init(&pipeline->pipeline));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_pipeline_free(struct kefir_mem *mem, struct kefir_asmcmp_pipeline *pipeline) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp pipeline"));

    REQUIRE_OK(kefir_list_free(mem, &pipeline->pipeline));
    memset(pipeline, 0, sizeof(struct kefir_asmcmp_pipeline));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_pipeline_add(struct kefir_mem *mem, struct kefir_asmcmp_pipeline *pipeline,
                                         const struct kefir_asmcmp_pipeline_pass *pass) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp pipeline"));
    REQUIRE(pass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp pipeline pass"));

    REQUIRE_OK(kefir_list_insert_after(mem, &pipeline->pipeline, kefir_list_tail(&pipeline->pipeline), (void *) pass));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_pipeline_apply(struct kefir_mem *mem, const struct kefir_asmcmp_pipeline *pipeline,
                                           kefir_asmcmp_pipeline_pass_type_t type,
                                           struct kefir_asmcmp_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(pipeline != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp pipeline"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp context"));

    for (const struct kefir_list_entry *iter = kefir_list_head(&pipeline->pipeline); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_asmcmp_pipeline_pass *, pass, iter->value);
        switch (type) {
            case KEFIR_ASMCMP_PIPELINE_PASS_VIRTUAL:
                if (pass->type != KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL) {
                    REQUIRE_OK(pass->apply(mem, context, pass));
                }
                break;

            case KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL:
                if (pass->type != KEFIR_ASMCMP_PIPELINE_PASS_VIRTUAL) {
                    REQUIRE_OK(pass->apply(mem, context, pass));
                }
                break;

            case KEFIR_ASMCMP_PIPELINE_PASS_BOTH:
                REQUIRE_OK(pass->apply(mem, context, pass));
                break;
        }
    }
    return KEFIR_OK;
}
