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
#include <string.h>

#define VALID_INSTR_IDX(_ctx, _idx) ((_idx) < (_ctx)->code_length)
#define VALID_LABEL_IDX(_ctx, _idx) ((_idx) < (_ctx)->labels_length)
#define VALID_VREG_IDX(_ctx, _idx) ((_idx) < (_ctx)->virtual_register_length)
#define HANDLE_AT(_ctx, _idx) ((_idx) != KEFIR_ASMCMP_INDEX_NONE ? &(_ctx)->code_content[(_idx)] : NULL)

kefir_result_t kefir_asmcmp_context_init(const struct kefir_asmcmp_context_class *klass, void *payload,
                                         struct kefir_asmcmp_context *context) {
    REQUIRE(klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context class"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen context"));

    REQUIRE_OK(kefir_hashtree_init(&context->label_positions, &kefir_hashtree_uint_ops));
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
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_context_free(struct kefir_mem *mem, struct kefir_asmcmp_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));

    REQUIRE_OK(kefir_hashtree_free(mem, &context->label_positions));
    memset(context->code_content, 0, sizeof(struct kefir_asmcmp_instruction) * context->code_length);
    KEFIR_FREE(mem, context->code_content);
    memset(context->labels, 0, sizeof(struct kefir_asmcmp_label) * context->labels_length);
    KEFIR_FREE(mem, context->labels);
    memset(context->virtual_registers, 0,
           sizeof(struct kefir_asmcmp_virtual_register) * context->virtual_register_length);
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
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER: {
            const struct kefir_asmcmp_virtual_register *vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(context, value->vreg.index, &vreg));
        } break;
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
        prev_handle != NULL ? HANDLE_AT(context, prev_handle->siblings.next) : NULL;

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
        prev_label_idx = (kefir_asmcmp_label_index_t) node->value;

        struct kefir_asmcmp_label *const prev_label = &context->labels[prev_label_idx];
        REQUIRE(prev_label->siblings.next == KEFIR_ASMCMP_INDEX_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected asmcmp label state"));
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
    REQUIRE(VALID_INSTR_IDX(context, target_instr) || target_instr == KEFIR_ASMCMP_INDEX_NONE,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided target asmgen instruction index is out of context bounds"));
    REQUIRE(VALID_LABEL_IDX(context, label_index),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided asmgen label index is out of context bounds"));

    struct kefir_asmcmp_label *const label = &context->labels[label_index];
    REQUIRE_OK(attach_label_to_instr(mem, context, target_instr, label));
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

kefir_result_t kefir_asmcmp_virtual_register_new(struct kefir_mem *mem, struct kefir_asmcmp_context *context,
                                                 kefir_asmcmp_register_type_t type,
                                                 kefir_asmcmp_virtual_register_index_t *reg_alloc_idx) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen context"));
    REQUIRE(reg_alloc_idx != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen register allocation index"));

    REQUIRE_OK(ensure_availability(mem, (void **) &context->virtual_registers, &context->virtual_register_length,
                                   &context->virtual_register_capacity, sizeof(struct kefir_asmcmp_virtual_register),
                                   1));

    struct kefir_asmcmp_virtual_register *reg_alloc = &context->virtual_registers[context->virtual_register_length];
    reg_alloc->index = context->virtual_register_length;
    reg_alloc->type = type;

    *reg_alloc_idx = reg_alloc->index;
    context->virtual_register_length++;
    return KEFIR_OK;
}
