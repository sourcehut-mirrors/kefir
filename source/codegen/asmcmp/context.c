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

#include "kefir/codegen/asmcmp/context.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define VALID_INSTR_IDX(_ctx, _idx) ((_idx) < (_ctx)->code_length)
#define VALID_LABEL_IDX(_ctx, _idx) ((_idx) < (_ctx)->labels_length)
#define VALID_VREG_IDX(_ctx, _idx) ((_idx) < (_ctx)->virtual_register_length)
#define HANDLE_AT(_ctx, _idx) ((_idx) != KEFIR_ASMCMP_INDEX_NONE ? &(_ctx)->code_content[(_idx)] : NULL)

static kefir_result_t free_stash(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                 kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_asmcmp_stash *, stash, value);
    REQUIRE(stash != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp stash"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &stash->stashed_physical_regs));
    memset(stash, 0, sizeof(struct kefir_asmcmp_stash));
    KEFIR_FREE(mem, stash);
    return KEFIR_OK;
}

static kefir_result_t free_inline_asm(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                      kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_asmcmp_inline_assembly *, inline_asm, value);
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp inline assembly"));

    REQUIRE_OK(kefir_list_free(mem, &inline_asm->fragments));
    memset(inline_asm, 0, sizeof(struct kefir_asmcmp_inline_assembly));
    KEFIR_FREE(mem, inline_asm);
    return KEFIR_OK;
}

static kefir_result_t kefir_asmcmp_format_impl(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                               const char **result, const char *format, va_list args) {
    va_list args2;
    va_copy(args2, args);

    int length = vsnprintf(NULL, 0, format, args);
    REQUIRE(length >= 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Formatting error"));
    ++length;

    char stack_buf[128];
    char *buf;
    if ((unsigned long) length < sizeof(stack_buf)) {
        buf = stack_buf;
    } else {
        buf = KEFIR_MALLOC(mem, length);
        REQUIRE(buf != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate formatting buffer"));
    }

    vsnprintf(buf, length, format, args2);
    va_end(args2);
    *result = kefir_string_pool_insert(mem, &context->strings, buf, NULL);
    REQUIRE_ELSE(*result != NULL, {
        if (buf != stack_buf) {
            KEFIR_FREE(mem, buf);
        }
        return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert formatted string into pool");
    });
    if (buf != stack_buf) {
        KEFIR_FREE(mem, buf);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_init(const struct kefir_asmcmp_context_class *klass, void *payload,
                                         struct kefir_asmcmp_context *context) {
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context class"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen context"));

    REQUIRE_OK(kefir_hashtree_init(&context->label_positions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_string_pool_init(&context->strings));
    REQUIRE_OK(kefir_hashtree_init(&context->stashes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&context->stashes, free_stash, NULL));
    REQUIRE_OK(kefir_hashtree_init(&context->inline_assembly, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&context->inline_assembly, free_inline_asm, NULL));
    REQUIRE_OK(kefir_asmcmp_liveness_map_init(&context->vreg_liveness));
    context->klass = klass;
    context->payload = payload;
    context->code_content = NULL;
    context->code_capacity = 0;
    context->code_length = 0;
    context->labels = NULL;
    context->labels_length = 0;
    context->labels_capacity = 0;
    context->code.head = KEFIR_ASMCMP_INDEX_NONE;
    context->code.tail = KEFIR_ASMCMP_INDEX_NONE;
    context->virtual_registers = NULL;
    context->virtual_register_length = 0;
    context->virtual_register_capacity = 0;
    context->next_stash_idx = 0;
    context->next_inline_asm_idx = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_free(struct kefir_mem *mem, struct kefir_asmcmp_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));

    REQUIRE_OK(kefir_asmcmp_liveness_map_free(mem, &context->vreg_liveness));
    REQUIRE_OK(kefir_hashtree_free(mem, &context->inline_assembly));
    REQUIRE_OK(kefir_hashtree_free(mem, &context->stashes));
    REQUIRE_OK(kefir_string_pool_free(mem, &context->strings));
    REQUIRE_OK(kefir_hashtree_free(mem, &context->label_positions));
    if (context->code_content != NULL) {
        memset(context->code_content, 0, sizeof(struct kefir_asmcmp_instruction) * context->code_length);
    }
    KEFIR_FREE(mem, context->code_content);
    if (context->labels != NULL) {
        memset(context->labels, 0, sizeof(struct kefir_asmcmp_label) * context->labels_length);
    }
    KEFIR_FREE(mem, context->labels);
    if (context->virtual_registers != NULL) {
        memset(context->virtual_registers, 0,
               sizeof(struct kefir_asmcmp_virtual_register) * context->virtual_register_length);
    }
    KEFIR_FREE(mem, context->virtual_registers);
    memset(context, 0, sizeof(struct kefir_asmcmp_context));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_instr_at(const struct kefir_asmcmp_context *context,
                                             kefir_asmcmp_instruction_index_t index,
                                             const struct kefir_asmcmp_instruction **instr_ptr) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_INSTR_IDX(context, index),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen instruction index is out of context bounds"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen instruction"));

    *instr_ptr = &context->code_content[index].instr;
    return KEFIR_OK;
}

kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_prev(const struct kefir_asmcmp_context *context,
                                                                 kefir_asmcmp_instruction_index_t index) {
    REQUIRE(context != NULL, KEFIR_ASMCMP_INDEX_NONE);
    REQUIRE(VALID_INSTR_IDX(context, index), KEFIR_ASMCMP_INDEX_NONE);

    return context->code_content[index].siblings.prev;
}

kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_next(const struct kefir_asmcmp_context *context,
                                                                 kefir_asmcmp_instruction_index_t index) {
    REQUIRE(context != NULL, KEFIR_ASMCMP_INDEX_NONE);
    REQUIRE(VALID_INSTR_IDX(context, index), KEFIR_ASMCMP_INDEX_NONE);

    return context->code_content[index].siblings.next;
}

kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_head(const struct kefir_asmcmp_context *context) {
    REQUIRE(context != NULL, KEFIR_ASMCMP_INDEX_NONE);

    return context->code.head;
}

kefir_asmcmp_instruction_index_t kefir_asmcmp_context_instr_tail(const struct kefir_asmcmp_context *context) {
    REQUIRE(context != NULL, KEFIR_ASMCMP_INDEX_NONE);

    return context->code.tail;
}

#define MIN_CAPACITY_INCREASE 64
static kefir_result_t ensure_availability(struct kefir_mem *mem, void **array, kefir_size_t *length,
                                          kefir_size_t *capacity, kefir_size_t elt_size,
                                          kefir_size_t required_elements) {
    REQUIRE(*capacity - *length < required_elements, KEFIR_OK);

    const kefir_size_t new_capacity = *capacity + MAX(MIN_CAPACITY_INCREASE, required_elements);
    void *const new_array = KEFIR_MALLOC(mem, elt_size * new_capacity);
    REQUIRE(new_array != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmgen array"));

    if (*array != NULL) {
        memcpy(new_array, *array, elt_size * (*length));
        KEFIR_FREE(mem, *array);
    }

    *array = new_array;
    *capacity = new_capacity;
    return KEFIR_OK;
}

static kefir_result_t validate_value(struct kefir_asmcmp_context *context, const struct kefir_asmcmp_value *value) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_X87:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER: {
            const struct kefir_asmcmp_virtual_register *vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(context, value->vreg.index, &vreg));
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS: {
                    const struct kefir_asmcmp_virtual_register *vreg;
                    REQUIRE_OK(kefir_asmcmp_virtual_register_get(context, value->indirect.base.vreg, &vreg));
                } break;

                case KEFIR_ASMCMP_INDIRECT_LABEL_BASIS:
                    REQUIRE(value->indirect.base.label != NULL,
                            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-NULL label indirection basis"));
                    break;

                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_VARARG_SAVE_AREA_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT:
            REQUIRE(value->rip_indirection.base != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-NULL rip indirection basis"));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_LABEL:
            REQUIRE(value->label.symbolic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-NULL label"));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
            REQUIRE(kefir_hashtree_has(&context->stashes, (kefir_hashtree_key_t) value->stash_idx),
                    KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown asmcmp stash index"));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            REQUIRE(kefir_hashtree_has(&context->inline_assembly, (kefir_hashtree_key_t) value->inline_asm_idx),
                    KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown asmcmp inline assembly index"));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t attach_label_to_instr(struct kefir_mem *, struct kefir_asmcmp_context *,
                                            kefir_asmcmp_instruction_index_t, struct kefir_asmcmp_label *);

static kefir_result_t detach_label(struct kefir_mem *, struct kefir_asmcmp_context *, struct kefir_asmcmp_label *);

kefir_result_t kefir_asmcmp_context_instr_insert_after(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                       kefir_asmcmp_instruction_index_t after_index,
                                                       const struct kefir_asmcmp_instruction *instr,
                                                       kefir_asmcmp_instruction_index_t *index_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_INSTR_IDX(context, after_index) || after_index == KEFIR_ASMCMP_INDEX_NONE,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen index is out of context bounds"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen instruction"));

    const kefir_bool_t tail_instr = after_index == kefir_asmcmp_context_instr_tail(context);

    REQUIRE_OK(validate_value(context, &instr->args[0]));
    REQUIRE_OK(validate_value(context, &instr->args[1]));
    REQUIRE_OK(validate_value(context, &instr->args[2]));

    REQUIRE_OK(ensure_availability(mem, (void **) &context->code_content, &context->code_length,
                                   &context->code_capacity, sizeof(struct kefir_asmcmp_instruction_handle), 1));

    const kefir_asmcmp_instruction_index_t index = context->code_length;
    struct kefir_asmcmp_instruction_handle *const handle = &context->code_content[index];
    handle->index = index;
    memcpy(&handle->instr, instr, sizeof(struct kefir_asmcmp_instruction));

    struct kefir_asmcmp_instruction_handle *const prev_handle = HANDLE_AT(context, after_index);
    struct kefir_asmcmp_instruction_handle *const next_handle =
        prev_handle != NULL ? HANDLE_AT(context, prev_handle->siblings.next) : HANDLE_AT(context, context->code.head);

    handle->siblings.prev = after_index;
    handle->siblings.next = next_handle != NULL ? next_handle->index : KEFIR_ASMCMP_INDEX_NONE;

    if (prev_handle != NULL) {
        prev_handle->siblings.next = index;
    }
    if (next_handle != NULL) {
        next_handle->siblings.prev = index;
    }

    if (context->code.tail == handle->siblings.prev) {
        context->code.tail = index;
    }
    if (context->code.head == handle->siblings.next) {
        context->code.head = handle->index;
    }

    context->code_length++;

    if (tail_instr) {
        struct kefir_hashtree_node *label_node;
        kefir_result_t res =
            kefir_hashtree_at(&context->label_positions, (kefir_hashtree_key_t) KEFIR_ASMCMP_INDEX_NONE, &label_node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);

            ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, label_idx, label_node->value);
            while (label_idx != KEFIR_ASMCMP_INDEX_NONE) {
                struct kefir_asmcmp_label *label = &context->labels[label_idx];
                label_idx = label->siblings.next;

                REQUIRE_OK(detach_label(mem, context, label));
                REQUIRE_OK(attach_label_to_instr(mem, context, index, label));
            }
        }
    }

    ASSIGN_PTR(index_ptr, index);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_instr_drop(struct kefir_asmcmp_context *context,
                                               kefir_asmcmp_instruction_index_t instr_idx) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_INSTR_IDX(context, instr_idx),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen instruction index is out of context bounds"));

    struct kefir_asmcmp_instruction_handle *const handle = &context->code_content[instr_idx];
    REQUIRE(!kefir_hashtree_has(&context->label_positions, (kefir_hashtree_key_t) instr_idx),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to drop asmgen instruction with attached labels"));

    struct kefir_asmcmp_instruction_handle *const prev_handle = HANDLE_AT(context, handle->siblings.prev);
    struct kefir_asmcmp_instruction_handle *const next_handle = HANDLE_AT(context, handle->siblings.next);

    if (prev_handle != NULL) {
        prev_handle->siblings.next = handle->siblings.next;
    } else {
        REQUIRE(context->code.head == handle->index,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected asmgen code link"));

        context->code.head = handle->siblings.next;
    }

    if (next_handle != NULL) {
        next_handle->siblings.prev = handle->siblings.prev;
    } else {
        REQUIRE(context->code.tail == handle->index,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected asmgen code link"));

        context->code.tail = handle->siblings.prev;
    }

    handle->siblings.prev = KEFIR_ASMCMP_INDEX_NONE;
    handle->siblings.next = KEFIR_ASMCMP_INDEX_NONE;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_instr_replace(struct kefir_asmcmp_context *context,
                                                  kefir_asmcmp_instruction_index_t instr_idx,
                                                  const struct kefir_asmcmp_instruction *instr) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_INSTR_IDX(context, instr_idx),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen instruction index is out of context bounds"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen instruction"));

    struct kefir_asmcmp_instruction_handle *const handle = &context->code_content[instr_idx];
    handle->instr = *instr;
    return KEFIR_OK;
}

static kefir_result_t attach_label_to_instr(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                            kefir_asmcmp_instruction_index_t target_instr,
                                            struct kefir_asmcmp_label *label) {
    REQUIRE(!label->attached, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected asmgen label to be detached"));
    REQUIRE(label->position == KEFIR_ASMCMP_INDEX_NONE && label->siblings.prev == KEFIR_ASMCMP_INDEX_NONE &&
                label->siblings.next == KEFIR_ASMCMP_INDEX_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected asmcmp label state"));

    kefir_asmcmp_label_index_t prev_label_idx = KEFIR_ASMCMP_INDEX_NONE;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->label_positions, (kefir_hashtree_key_t) target_instr, &node);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &context->label_positions, (kefir_hashtree_key_t) target_instr,
                                         (kefir_hashtree_value_t) label->label));
    } else {
        REQUIRE_OK(res);
        struct kefir_asmcmp_label *prev_label = &context->labels[(kefir_asmcmp_label_index_t) node->value];
        for (; prev_label->siblings.next != KEFIR_ASMCMP_INDEX_NONE;
             prev_label = &context->labels[prev_label->siblings.next])
            ;
        prev_label->siblings.next = label->label;
    }

    label->siblings.prev = prev_label_idx;
    label->position = target_instr;
    label->attached = true;
    return KEFIR_OK;
}

static kefir_result_t detach_label(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                   struct kefir_asmcmp_label *label) {
    REQUIRE(label->attached, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected asmgen label to be attached"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->label_positions, (kefir_hashtree_key_t) label->position, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find attached asmcmp label position");
    }
    REQUIRE_OK(res);

    kefir_asmcmp_label_index_t head_label_idx = node->value;

    kefir_asmcmp_label_index_t prev_idx = label->siblings.prev, next_idx = label->siblings.next;

    if (prev_idx != KEFIR_ASMCMP_INDEX_NONE) {
        struct kefir_asmcmp_label *const prev_label = &context->labels[prev_idx];
        prev_label->siblings.next = next_idx;
        label->siblings.prev = KEFIR_ASMCMP_INDEX_NONE;
    } else {
        REQUIRE(head_label_idx == label->label,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected asmgen instruction label list"));

        if (next_idx != KEFIR_ASMCMP_INDEX_NONE) {
            node->value = (kefir_hashtree_value_t) next_idx;
        } else {
            REQUIRE_OK(kefir_hashtree_delete(mem, &context->label_positions, (kefir_hashtree_key_t) label->position));
        }
    }

    if (next_idx != KEFIR_ASMCMP_INDEX_NONE) {
        struct kefir_asmcmp_label *const next_label = &context->labels[next_idx];
        next_label->siblings.prev = prev_idx;
        label->siblings.next = KEFIR_ASMCMP_INDEX_NONE;
    }

    label->position = KEFIR_ASMCMP_INDEX_NONE;
    label->attached = false;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_new_label(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                              kefir_asmcmp_instruction_index_t target_instr,
                                              kefir_asmcmp_label_index_t *index_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_INSTR_IDX(context, target_instr) || target_instr == KEFIR_ASMCMP_INDEX_NONE,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided target asmgen instruction index is out of context bounds"));
    REQUIRE(target_instr != KEFIR_ASMCMP_INDEX_NONE || index_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen label index"));

    kefir_asmcmp_label_index_t index = context->labels_length;
    REQUIRE_OK(ensure_availability(mem, (void **) &context->labels, &context->labels_length, &context->labels_capacity,
                                   sizeof(struct kefir_asmcmp_label), 1));
    struct kefir_asmcmp_label *const label = &context->labels[index];

    label->label = index;
    label->attached = false;
    label->position = KEFIR_ASMCMP_INDEX_NONE;
    label->siblings.prev = KEFIR_ASMCMP_INDEX_NONE;
    label->siblings.next = KEFIR_ASMCMP_INDEX_NONE;

    if (target_instr != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(attach_label_to_instr(mem, context, target_instr, label));
    }

    context->labels_length++;
    ASSIGN_PTR(index_ptr, index);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_label_at(const struct kefir_asmcmp_context *context,
                                             kefir_asmcmp_label_index_t label_index,
                                             kefir_asmcmp_instruction_index_t *instr_idx_ptr) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_LABEL_IDX(context, label_index),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen label index is out of context bounds"));
    REQUIRE(instr_idx_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen instruction index"));

    const struct kefir_asmcmp_label *const label = &context->labels[label_index];
    *instr_idx_ptr = label->position;
    return KEFIR_OK;
}

kefir_asmcmp_label_index_t kefir_asmcmp_context_instr_label_head(const struct kefir_asmcmp_context *context,
                                                                 kefir_asmcmp_instruction_index_t instr_index) {
    REQUIRE(context != NULL, KEFIR_ASMCMP_INDEX_NONE);
    REQUIRE(VALID_INSTR_IDX(context, instr_index), KEFIR_ASMCMP_INDEX_NONE);

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->label_positions, (kefir_hashtree_key_t) instr_index, &node);
    if (res == KEFIR_NOT_FOUND) {
        return KEFIR_ASMCMP_INDEX_NONE;
    }
    REQUIRE_OK(res);

    return (kefir_asmcmp_label_index_t) node->value;
}

kefir_asmcmp_label_index_t kefir_asmcmp_context_instr_label_prev(const struct kefir_asmcmp_context *context,
                                                                 kefir_asmcmp_label_index_t label_index) {
    REQUIRE(context != NULL, KEFIR_ASMCMP_INDEX_NONE);
    REQUIRE(VALID_LABEL_IDX(context, label_index), KEFIR_ASMCMP_INDEX_NONE);

    const struct kefir_asmcmp_label *const label = &context->labels[label_index];
    return label->siblings.prev;
}

kefir_asmcmp_label_index_t kefir_asmcmp_context_instr_label_next(const struct kefir_asmcmp_context *context,
                                                                 kefir_asmcmp_label_index_t label_index) {
    REQUIRE(context != NULL, KEFIR_ASMCMP_INDEX_NONE);
    REQUIRE(VALID_LABEL_IDX(context, label_index), KEFIR_ASMCMP_INDEX_NONE);

    const struct kefir_asmcmp_label *const label = &context->labels[label_index];
    return label->siblings.next;
}

kefir_result_t kefir_asmcmp_context_bind_label(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                               kefir_asmcmp_instruction_index_t target_instr,
                                               kefir_asmcmp_label_index_t label_index) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_INSTR_IDX(context, target_instr),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided target asmgen instruction index is out of context bounds"));
    REQUIRE(VALID_LABEL_IDX(context, label_index),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen label index is out of context bounds"));

    struct kefir_asmcmp_label *const label = &context->labels[label_index];
    REQUIRE_OK(attach_label_to_instr(mem, context, target_instr, label));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_bind_label_after_tail(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                          kefir_asmcmp_label_index_t label_index) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_LABEL_IDX(context, label_index),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen label index is out of context bounds"));

    struct kefir_asmcmp_label *const label = &context->labels[label_index];
    REQUIRE_OK(attach_label_to_instr(mem, context, KEFIR_ASMCMP_INDEX_NONE, label));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_unbind_label(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                 kefir_asmcmp_label_index_t label_index) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_LABEL_IDX(context, label_index),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen label index is out of context bounds"));

    struct kefir_asmcmp_label *const label = &context->labels[label_index];
    REQUIRE_OK(detach_label(mem, context, label));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_move_labels(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                kefir_asmcmp_virtual_register_index_t dst_idx,
                                                kefir_asmcmp_virtual_register_index_t src_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_INSTR_IDX(context, dst_idx),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen instruction index is out of context bounds"));
    REQUIRE(VALID_INSTR_IDX(context, src_idx),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen instruction index is out of context bounds"));

    struct kefir_hashtree_node *label_node;
    kefir_result_t res = kefir_hashtree_at(&context->label_positions, (kefir_hashtree_key_t) src_idx, &label_node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);

        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, label_idx, label_node->value);
        while (label_idx != KEFIR_ASMCMP_INDEX_NONE) {
            struct kefir_asmcmp_label *label = &context->labels[label_idx];
            label_idx = label->siblings.next;

            REQUIRE_OK(detach_label(mem, context, label));
            REQUIRE_OK(attach_label_to_instr(mem, context, dst_idx, label));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_number_of_virtual_registers(const struct kefir_asmcmp_context *context,
                                                        kefir_size_t *length) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(length != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to length"));

    *length = context->virtual_register_length;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_virtual_register_get(const struct kefir_asmcmp_context *context,
                                                 kefir_asmcmp_virtual_register_index_t idx,
                                                 const struct kefir_asmcmp_virtual_register **reg_alloc) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_VREG_IDX(context, idx),
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested asmgen register allocation"));
    REQUIRE(reg_alloc != NULL,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Expected valid pointer to asmgen register allocation"));

    *reg_alloc = &context->virtual_registers[idx];
    return KEFIR_OK;
}

static kefir_result_t new_virtual_register(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                           kefir_asmcmp_virtual_register_type_t type,
                                           kefir_asmcmp_virtual_register_index_t *reg_alloc_idx) {
    REQUIRE_OK(ensure_availability(mem, (void **) &context->virtual_registers, &context->virtual_register_length,
                                   &context->virtual_register_capacity, sizeof(struct kefir_asmcmp_virtual_register),
                                   1));

    struct kefir_asmcmp_virtual_register *reg_alloc = &context->virtual_registers[context->virtual_register_length];
    reg_alloc->index = context->virtual_register_length;
    reg_alloc->type = type;

    *reg_alloc_idx = reg_alloc->index;
    context->virtual_register_length++;

    REQUIRE_OK(kefir_asmcmp_liveness_map_resize(mem, &context->vreg_liveness, context->virtual_register_length));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_virtual_register_new(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                 kefir_asmcmp_virtual_register_type_t type,
                                                 kefir_asmcmp_virtual_register_index_t *reg_alloc_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(reg_alloc_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen register allocation index"));

    switch (type) {
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED:
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE:
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_DIRECT_SPILL_SPACE:
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_INDIRECT_SPILL_SPACE:
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_EXTERNAL_MEMORY:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                   "Specialized virtual register construction functions shall be used");
    }

    REQUIRE_OK(new_virtual_register(mem, context, type, reg_alloc_idx));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_virtual_register_new_direct_spill_space_allocation(
    struct kefir_mem *mem, struct kefir_asmcmp_context *context, kefir_size_t length,
    kefir_asmcmp_virtual_register_index_t *reg_alloc_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(reg_alloc_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen register allocation index"));
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(reg_alloc_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen register allocation index"));

    REQUIRE_OK(new_virtual_register(mem, context, KEFIR_ASMCMP_VIRTUAL_REGISTER_DIRECT_SPILL_SPACE, reg_alloc_idx));
    struct kefir_asmcmp_virtual_register *reg_alloc = &context->virtual_registers[*reg_alloc_idx];
    reg_alloc->parameters.spill_space_allocation_length = length;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
    struct kefir_mem *mem, struct kefir_asmcmp_context *context, kefir_size_t length,
    kefir_asmcmp_virtual_register_index_t *reg_alloc_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(reg_alloc_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen register allocation index"));
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(reg_alloc_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen register allocation index"));

    REQUIRE_OK(new_virtual_register(mem, context, KEFIR_ASMCMP_VIRTUAL_REGISTER_INDIRECT_SPILL_SPACE, reg_alloc_idx));
    struct kefir_asmcmp_virtual_register *reg_alloc = &context->virtual_registers[*reg_alloc_idx];
    reg_alloc->parameters.spill_space_allocation_length = length;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_virtual_register_new_memory_pointer(struct kefir_mem *mem,
                                                                struct kefir_asmcmp_context *context,
                                                                kefir_asmcmp_physical_register_index_t base_reg,
                                                                kefir_int64_t offset,
                                                                kefir_asmcmp_virtual_register_index_t *reg_alloc_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(reg_alloc_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen register allocation index"));
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(reg_alloc_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen register allocation index"));

    REQUIRE_OK(new_virtual_register(mem, context, KEFIR_ASMCMP_VIRTUAL_REGISTER_EXTERNAL_MEMORY, reg_alloc_idx));
    struct kefir_asmcmp_virtual_register *reg_alloc = &context->virtual_registers[*reg_alloc_idx];
    reg_alloc->parameters.memory.base_reg = base_reg;
    reg_alloc->parameters.memory.offset = offset;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_virtual_set_spill_space_size(const struct kefir_asmcmp_context *context,
                                                         kefir_asmcmp_virtual_register_index_t reg_idx,
                                                         kefir_size_t length) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_VREG_IDX(context, reg_idx),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen virtual register index"));

    struct kefir_asmcmp_virtual_register *const vreg = &context->virtual_registers[reg_idx];
    REQUIRE(vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_INDIRECT_SPILL_SPACE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Virtual register type mismatch"));

    vreg->parameters.spill_space_allocation_length = length;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_virtual_register_specify_type(const struct kefir_asmcmp_context *context,
                                                          kefir_asmcmp_virtual_register_index_t reg_idx,
                                                          kefir_asmcmp_virtual_register_type_t type) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_VREG_IDX(context, reg_idx),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen virtual register index"));

    struct kefir_asmcmp_virtual_register *const vreg = &context->virtual_registers[reg_idx];
    REQUIRE(vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Virtual register type has already been specified"));

    vreg->type = type;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_register_stash_new(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                               kefir_asmcmp_stash_index_t *stash_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(stash_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen stash index"));

    struct kefir_asmcmp_stash *stash = KEFIR_MALLOC(mem, sizeof(struct kefir_asmcmp_stash));
    REQUIRE(stash != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp stash"));
    stash->index = context->next_stash_idx;
    stash->liveness_instr_index = KEFIR_ASMCMP_INDEX_NONE;

    kefir_result_t res = kefir_hashtreeset_init(&stash->stashed_physical_regs, &kefir_hashtree_uint_ops);
    REQUIRE_CHAIN(&res, kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(mem, context, 0,
                                                                                          &stash->stash_area_vreg));
    REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &context->stashes, (kefir_hashtree_key_t) stash->index,
                                              (kefir_hashtree_value_t) stash));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, stash);
        return res;
    });

    ++context->next_stash_idx;
    *stash_idx = stash->index;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_register_stash_add(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                               kefir_asmcmp_stash_index_t stash_idx,
                                               kefir_asmcmp_physical_register_index_t phreg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->stashes, (kefir_hashtree_key_t) stash_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find asmcmp stash");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_asmcmp_stash *, stash, node->value);
    REQUIRE_OK(kefir_hashtreeset_add(mem, &stash->stashed_physical_regs, (kefir_hashtreeset_entry_t) phreg));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_register_stash_set_liveness_index(const struct kefir_asmcmp_context *context,
                                                              kefir_asmcmp_stash_index_t stash_idx,
                                                              kefir_asmcmp_instruction_index_t instr_idx) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(VALID_INSTR_IDX(context, instr_idx) || instr_idx == KEFIR_ASMCMP_INDEX_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen instruction index"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->stashes, (kefir_hashtree_key_t) stash_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find asmcmp stash");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_asmcmp_stash *, stash, node->value);
    stash->liveness_instr_index = instr_idx;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_register_stash_has(const struct kefir_asmcmp_context *context,
                                               kefir_asmcmp_stash_index_t stash_idx,
                                               kefir_asmcmp_physical_register_index_t phreg, kefir_bool_t *contains) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(contains != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->stashes, (kefir_hashtree_key_t) stash_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find asmcmp stash");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_asmcmp_stash *, stash, node->value);
    *contains = kefir_hashtreeset_has(&stash->stashed_physical_regs, (kefir_hashtreeset_entry_t) phreg);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_register_stash_vreg(const struct kefir_asmcmp_context *context,
                                                kefir_asmcmp_stash_index_t stash_idx,
                                                kefir_asmcmp_virtual_register_index_t *vreg_idx) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(vreg_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen stash virtual register"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->stashes, (kefir_hashtree_key_t) stash_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find asmcmp stash");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_asmcmp_stash *, stash, node->value);
    *vreg_idx = stash->stash_area_vreg;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_register_stash_liveness_index(const struct kefir_asmcmp_context *context,
                                                          kefir_asmcmp_stash_index_t stash_idx,
                                                          kefir_asmcmp_instruction_index_t *instr_idx) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(instr_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen instruction index"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->stashes, (kefir_hashtree_key_t) stash_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find asmcmp stash");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_asmcmp_stash *, stash, node->value);
    *instr_idx = stash->liveness_instr_index;
    return KEFIR_OK;
}

static kefir_result_t free_inline_asm_fragment(struct kefir_mem *mem, struct kefir_list *list,
                                               struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_asmcmp_inline_assembly_fragment *, fragment, entry->value);
    REQUIRE(fragment != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid inline assembly fragment"));

    memset(fragment, 0, sizeof(struct kefir_asmcmp_inline_assembly_fragment));
    KEFIR_FREE(mem, fragment);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_inline_assembly_new(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                const char *template,
                                                kefir_asmcmp_inline_assembly_index_t *inline_asm_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(template != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen inline assembly template"));
    REQUIRE(inline_asm_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp inline assembly index"));

    struct kefir_asmcmp_inline_assembly *inline_asm = KEFIR_MALLOC(mem, sizeof(struct kefir_asmcmp_inline_assembly));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate asmcmp inline assembly"));
    inline_asm->index = context->next_stash_idx;
    inline_asm->template = kefir_string_pool_insert(mem, &context->strings, template, NULL);
    REQUIRE_ELSE(inline_asm->template != NULL, {
        KEFIR_FREE(mem, inline_asm);
        return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert inline assembly template into string pool");
    });
    inline_asm->template_length = strlen(inline_asm->template);

    kefir_result_t res = kefir_list_init(&inline_asm->fragments);
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&inline_asm->fragments, free_inline_asm_fragment, NULL));
    REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &context->inline_assembly, (kefir_hashtree_key_t) inline_asm->index,
                                              (kefir_hashtree_value_t) inline_asm));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, inline_asm);
        return res;
    });

    ++context->next_inline_asm_idx;
    *inline_asm_idx = inline_asm->index;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_inline_assembly_add_text(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                     kefir_asmcmp_inline_assembly_index_t inline_asm_idx,
                                                     const char *format, ...) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(format != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid format string"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->inline_assembly, (kefir_hashtree_key_t) inline_asm_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested inline assembly");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_asmcmp_inline_assembly *, inline_asm, node->value);

    va_list args;
    va_start(args, format);

    const char *result;
    res = kefir_asmcmp_format_impl(mem, context, &result, format, args);
    va_end(args);
    REQUIRE_OK(res);

    struct kefir_asmcmp_inline_assembly_fragment *fragment =
        KEFIR_MALLOC(mem, sizeof(struct kefir_asmcmp_inline_assembly_fragment));
    REQUIRE(fragment != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate inline assembly fragment"));

    fragment->type = KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_TEXT;
    fragment->text = result;

    res = kefir_list_insert_after(mem, &inline_asm->fragments, kefir_list_tail(&inline_asm->fragments), fragment);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, fragment);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_inline_assembly_add_value(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                      kefir_asmcmp_inline_assembly_index_t inline_asm_idx,
                                                      const struct kefir_asmcmp_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(value != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp value"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->inline_assembly, (kefir_hashtree_key_t) inline_asm_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested inline assembly");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_asmcmp_inline_assembly *, inline_asm, node->value);

    REQUIRE_OK(validate_value(context, value));

    struct kefir_asmcmp_inline_assembly_fragment *fragment =
        KEFIR_MALLOC(mem, sizeof(struct kefir_asmcmp_inline_assembly_fragment));
    REQUIRE(fragment != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate inline assembly fragment"));

    fragment->type = KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_VALUE;
    fragment->value = *value;

    res = kefir_list_insert_after(mem, &inline_asm->fragments, kefir_list_tail(&inline_asm->fragments), fragment);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, fragment);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_inline_assembly_fragment_iter(const struct kefir_asmcmp_context *context,
                                                          kefir_asmcmp_inline_assembly_index_t inline_asm_idx,
                                                          struct kefir_asmcmp_inline_assembly_fragment_iterator *iter) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp fragment iterator"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&context->inline_assembly, (kefir_hashtree_key_t) inline_asm_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested inline assembly");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct kefir_asmcmp_inline_assembly *, inline_asm, node->value);

    iter->iter = kefir_list_head(&inline_asm->fragments);
    if (iter->iter != NULL) {
        iter->fragment = (struct kefir_asmcmp_inline_assembly_fragment *) iter->iter->value;
    } else {
        iter->fragment = NULL;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_inline_assembly_fragment_next(struct kefir_asmcmp_inline_assembly_fragment_iterator *iter) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp fragment iterator"));

    kefir_list_next(&iter->iter);
    if (iter->iter != NULL) {
        iter->fragment = (struct kefir_asmcmp_inline_assembly_fragment *) iter->iter->value;
    } else {
        iter->fragment = NULL;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_format(struct kefir_mem *mem, struct kefir_asmcmp_context *context, const char **result,
                                   const char *format, ...) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to resulting string"));
    REQUIRE(format != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid format string"));

    va_list args;
    va_start(args, format);
    kefir_result_t res = kefir_asmcmp_format_impl(mem, context, result, format, args);
    va_end(args);
    REQUIRE_OK(res);

    return KEFIR_OK;
}
