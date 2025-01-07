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
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t collect_labels(struct kefir_mem *mem, const struct kefir_asmcmp_value *value,
                                     struct kefir_hashtreeset *alive_labels) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
            REQUIRE_OK(kefir_hashtreeset_add(mem, alive_labels, (kefir_hashtreeset_entry_t) value->internal_label));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
            REQUIRE_OK(
                kefir_hashtreeset_add(mem, alive_labels, (kefir_hashtreeset_entry_t) value->rip_indirection.internal));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS:
                    REQUIRE_OK(kefir_hashtreeset_add(mem, alive_labels,
                                                     (kefir_hashtreeset_entry_t) value->indirect.base.internal_label));
                    break;

                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_VARARG_SAVE_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_X87:
        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t eliminate_label_impl(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                           struct kefir_hashtreeset *alive_labels) {
    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(context);
         instr_index != KEFIR_ASMCMP_INDEX_NONE; instr_index = kefir_asmcmp_context_instr_next(context, instr_index)) {

        struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(context, instr_index, &instr));

        REQUIRE_OK(collect_labels(mem, &instr->args[0], alive_labels));
        REQUIRE_OK(collect_labels(mem, &instr->args[1], alive_labels));
        REQUIRE_OK(collect_labels(mem, &instr->args[2], alive_labels));
    }

    kefir_result_t res;
    kefir_asmcmp_label_index_t label_index;
    const struct kefir_asmcmp_label *label;
    for (res = kefir_asmcmp_context_label_head(context, &label_index);
         res == KEFIR_OK && label_index != KEFIR_ASMCMP_INDEX_NONE;
         res = kefir_asmcmp_context_label_next(context, label_index, &label_index)) {

        REQUIRE_OK(kefir_asmcmp_context_get_label(context, label_index, &label));
        if (label->attached && !label->external_dependencies &&
            !kefir_hashtreeset_has(alive_labels, (kefir_hashtreeset_entry_t) label_index)) {
            REQUIRE_OK(kefir_asmcmp_context_unbind_label(mem, context, label_index));
        }
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

static kefir_result_t eliminate_label_apply(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                            const struct kefir_asmcmp_pipeline_pass *pass) {
    UNUSED(mem);
    UNUSED(context);
    UNUSED(pass);

    struct kefir_hashtreeset alive_labels;
    REQUIRE_OK(kefir_hashtreeset_init(&alive_labels, &kefir_hashtree_uint_ops));
    kefir_result_t res = eliminate_label_impl(mem, context, &alive_labels);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &alive_labels);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &alive_labels));
    return KEFIR_OK;
}

const struct kefir_asmcmp_pipeline_pass KefirAsmcmpAmd64EliminateLabelPass = {
    .name = "amd64-eliminate-label",
    .type = KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL,
    .apply = eliminate_label_apply,
    .payload = NULL};
