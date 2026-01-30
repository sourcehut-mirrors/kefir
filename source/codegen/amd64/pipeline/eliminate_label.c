/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/codegen/asmcmp/transform.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t alive_label(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                  kefir_asmcmp_label_index_t *label_idx, struct kefir_hashtable *label_mapping) {
    const struct kefir_asmcmp_label *label;
    REQUIRE_OK(kefir_asmcmp_context_get_label(context, *label_idx, &label));
    REQUIRE(label->attached, KEFIR_OK);

    kefir_hashtree_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(label_mapping, (kefir_hashtable_key_t) label->position, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        if (label->external_dependencies) {
            REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(mem, context,
                                                                             (kefir_asmcmp_label_index_t) table_value));
        }

        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&label->public_labels, &iter); res == KEFIR_OK;
             res = kefir_hashtreeset_next(&iter)) {
            REQUIRE_OK(kefir_asmcmp_context_label_add_public_name(
                mem, context, (kefir_asmcmp_label_index_t) table_value, (const char *) iter.entry));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        *label_idx = (kefir_asmcmp_label_index_t) table_value;
    } else {
        REQUIRE_OK(kefir_hashtable_insert(mem, label_mapping, (kefir_hashtable_key_t) label->position,
                                          (kefir_hashtable_value_t) *label_idx));
    }
    return KEFIR_OK;
}

static kefir_result_t collect_labels(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                     struct kefir_asmcmp_value *value, struct kefir_hashtable *label_mapping) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
            REQUIRE_OK(alive_label(mem, context, &value->internal_label, label_mapping));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
            REQUIRE_OK(alive_label(mem, context, &value->rip_indirection.internal, label_mapping));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS:
                    REQUIRE_OK(alive_label(mem, context, &value->indirect.base.internal_label, label_mapping));
                    break;

                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX: {
            struct kefir_asmcmp_inline_assembly_fragment_iterator iter;
            kefir_result_t res;
            for (res = kefir_asmcmp_inline_assembly_fragment_iter(context, value->inline_asm_idx, &iter);
                 res == KEFIR_OK && iter.fragment != NULL; res = kefir_asmcmp_inline_assembly_fragment_next(&iter)) {

                switch (iter.fragment->type) {
                    case KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                        // Intentionally left blank
                        break;

                    case KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_VALUE:
                        if (iter.fragment->value.type == KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL) {
                            REQUIRE_OK(alive_label(mem, context, &iter.fragment->value.internal_label, label_mapping));
                        } else if (iter.fragment->value.type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL) {
                            REQUIRE_OK(alive_label(mem, context, &iter.fragment->value.rip_indirection.internal,
                                                   label_mapping));
                        } else if (iter.fragment->value.type == KEFIR_ASMCMP_VALUE_TYPE_INDIRECT &&
                                   iter.fragment->value.indirect.type == KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS) {
                            REQUIRE_OK(alive_label(mem, context, &iter.fragment->value.indirect.base.internal_label,
                                                   label_mapping));
                        }
                        break;
                }
            }
            REQUIRE_OK(res);
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_X87:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t eliminate_label_impl(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                           struct kefir_hashtable *label_mapping) {
    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(context);
         instr_index != KEFIR_ASMCMP_INDEX_NONE; instr_index = kefir_asmcmp_context_instr_next(context, instr_index)) {

        struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(context, instr_index, &instr));

        REQUIRE_OK(collect_labels(mem, context, &instr->args[0], label_mapping));
        REQUIRE_OK(collect_labels(mem, context, &instr->args[1], label_mapping));
        REQUIRE_OK(collect_labels(mem, context, &instr->args[2], label_mapping));
    }

    kefir_result_t res;
    struct kefir_asmcmp_code_map_iterator code_map_iter;
    kefir_asmcmp_debug_info_code_reference_t code_ref;
    for (res = kefir_asmcmp_code_map_iter(&context->debug_info.code_map, &code_map_iter, &code_ref); res == KEFIR_OK;
         res = kefir_asmcmp_code_map_next(&code_map_iter, &code_ref)) {
        struct kefir_asmcmp_code_map_fragment_iterator fragment_iter;
        const struct kefir_asmcmp_debug_info_code_fragment *code_fragment;
        for (res = kefir_asmcmp_code_map_fragment_iter(&context->debug_info.code_map, code_ref, &fragment_iter,
                                                       &code_fragment);
             res == KEFIR_OK; res = kefir_asmcmp_code_map_fragment_next(&fragment_iter, &code_fragment)) {
            REQUIRE_OK(
                alive_label(mem, context, (kefir_asmcmp_label_index_t *) &code_fragment->begin_label, label_mapping));
            REQUIRE_OK(
                alive_label(mem, context, (kefir_asmcmp_label_index_t *) &code_fragment->end_label, label_mapping));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_asmcmp_value_map_iterator value_map_iter;
    kefir_asmcmp_debug_info_value_reference_t value_ref;
    for (res = kefir_asmcmp_value_map_iter(&context->debug_info.value_map, &value_map_iter, &value_ref);
         res == KEFIR_OK; res = kefir_asmcmp_value_map_next(&value_map_iter, &value_ref)) {
        struct kefir_asmcmp_value_map_fragment_iterator fragment_iter;
        const struct kefir_asmcmp_debug_info_value_fragment *value_fragment;
        for (res = kefir_asmcmp_value_map_fragment_iter(&context->debug_info.value_map, value_ref, &fragment_iter,
                                                        &value_fragment);
             res == KEFIR_OK; res = kefir_asmcmp_value_map_fragment_next(&fragment_iter, &value_fragment)) {
            REQUIRE_OK(
                alive_label(mem, context, (kefir_asmcmp_label_index_t *) &value_fragment->begin_label, label_mapping));
            REQUIRE_OK(
                alive_label(mem, context, (kefir_asmcmp_label_index_t *) &value_fragment->end_label, label_mapping));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_asmcmp_label_index_t label_index;
    for (res = kefir_asmcmp_context_label_head(context, &label_index);
         res == KEFIR_OK && label_index != KEFIR_ASMCMP_INDEX_NONE;
         res = kefir_asmcmp_context_label_next(context, label_index, &label_index)) {

        const struct kefir_asmcmp_label *label;
        REQUIRE_OK(kefir_asmcmp_context_get_label(context, label_index, &label));
        if (!label->attached) {
            continue;
        }

        kefir_hashtable_value_t value;
        res = kefir_hashtable_at(label_mapping, (kefir_hashtable_key_t) label->position, &value);
        if (res == KEFIR_NOT_FOUND) {
            if (kefir_hashtreeset_empty(&label->public_labels)) {
                REQUIRE_OK(kefir_asmcmp_context_unbind_label(mem, context, label_index));
            }
            continue;
        }
        REQUIRE_OK(res);

        if (label_index != value) {
            if (label->external_dependencies) {
                REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(mem, context,
                                                                                 (kefir_asmcmp_label_index_t) value));
            }
            struct kefir_hashtreeset_iterator iter;
            for (res = kefir_hashtreeset_iter(&label->public_labels, &iter); res == KEFIR_OK;
                 res = kefir_hashtreeset_next(&iter)) {
                REQUIRE_OK(kefir_asmcmp_context_label_add_public_name(mem, context, (kefir_asmcmp_label_index_t) value,
                                                                      (const char *) iter.entry));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            REQUIRE_OK(kefir_asmcmp_context_unbind_label(mem, context, label_index));
        }
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_compact_labels(struct kefir_mem *mem, struct kefir_asmcmp_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp context"));

    struct kefir_hashtable label_mapping;
    REQUIRE_OK(kefir_hashtable_init(&label_mapping, &kefir_hashtable_uint_ops));
    kefir_result_t res = eliminate_label_impl(mem, context, &label_mapping);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &label_mapping);
        return res;
    });
    REQUIRE_OK(kefir_hashtable_free(mem, &label_mapping));
    return KEFIR_OK;
}
