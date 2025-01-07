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

#include "kefir/target/abi/amd64/system-v/qwords.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct qword_counter {
    const struct kefir_vector *layout;
    kefir_size_t count;
};

static kefir_result_t count_qwords_visitor(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(typeentry);
    struct qword_counter *counter = (struct qword_counter *) payload;
    ASSIGN_DECL_CAST(const struct kefir_abi_amd64_typeentry_layout *, layout, kefir_vector_at(counter->layout, index));
    kefir_size_t count = layout->size / KEFIR_AMD64_ABI_QWORD;
    if (layout->size % KEFIR_AMD64_ABI_QWORD != 0) {
        count++;
    }
    counter->count += count;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_count(const struct kefir_ir_type *type, const struct kefir_vector *layout,
                                                 kefir_size_t *count) {
    struct kefir_ir_type_visitor visitor;
    kefir_ir_type_visitor_init(&visitor, count_qwords_visitor);
    struct qword_counter counter = {.layout = layout, .count = 0};
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &counter, 0, kefir_ir_type_children(type)));
    *count = counter.count;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_alloc(struct kefir_abi_amd64_sysv_qwords *qwords, struct kefir_mem *mem,
                                                 kefir_size_t qword_count) {
    REQUIRE(qwords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord vector"));
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    REQUIRE_OK(kefir_vector_alloc(mem, sizeof(struct kefir_abi_amd64_sysv_qword), qword_count, &qwords->qwords));
    kefir_vector_extend(&qwords->qwords, qword_count);
    for (kefir_size_t i = 0; i < qword_count; i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_amd64_sysv_qword *, qword, kefir_vector_at(&qwords->qwords, i));
        qword->klass = KEFIR_AMD64_SYSV_PARAM_NO_CLASS;
        qword->location = 0;
        qword->index = i;
        qword->current_offset = 0;
    }
    qwords->current = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_free(struct kefir_abi_amd64_sysv_qwords *qwords, struct kefir_mem *mem) {
    REQUIRE(qwords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord vector"));
    return kefir_vector_free(mem, &qwords->qwords);
}

static kefir_abi_amd64_sysv_data_class_t derive_dataclass(kefir_abi_amd64_sysv_data_class_t first,
                                                          kefir_abi_amd64_sysv_data_class_t second) {
    if (first == second) {
        return first;
    }
    if (first == KEFIR_AMD64_SYSV_PARAM_NO_CLASS) {
        return second;
    }
    if (second == KEFIR_AMD64_SYSV_PARAM_NO_CLASS) {
        return first;
    }
#define ANY_OF(x, y, a) ((x) == (a) || (y) == (a))
    if (ANY_OF(first, second, KEFIR_AMD64_SYSV_PARAM_MEMORY)) {
        return KEFIR_AMD64_SYSV_PARAM_MEMORY;
    }
    if (ANY_OF(first, second, KEFIR_AMD64_SYSV_PARAM_INTEGER)) {
        return KEFIR_AMD64_SYSV_PARAM_INTEGER;
    }
    if (ANY_OF(first, second, KEFIR_AMD64_SYSV_PARAM_X87) || ANY_OF(first, second, KEFIR_AMD64_SYSV_PARAM_X87UP) ||
        ANY_OF(first, second, KEFIR_AMD64_SYSV_PARAM_COMPLEX_X87)) {
        return KEFIR_AMD64_SYSV_PARAM_MEMORY;
    }
#undef ANY_OF
    return KEFIR_AMD64_SYSV_PARAM_SSE;
}

static struct kefir_abi_amd64_sysv_qword *next_qword(struct kefir_abi_amd64_sysv_qwords *qwords,
                                                     kefir_size_t alignment) {
    ASSIGN_DECL_CAST(struct kefir_abi_amd64_sysv_qword *, qword, kefir_vector_at(&qwords->qwords, qwords->current));
    kefir_size_t unalign = qword->current_offset % alignment;
    kefir_size_t pad = unalign > 0 ? alignment - unalign : 0;
    if (alignment == 0 || qword->current_offset + pad >= KEFIR_AMD64_ABI_QWORD) {
        qwords->current++;
        qword = (struct kefir_abi_amd64_sysv_qword *) kefir_vector_at(&qwords->qwords, qwords->current);
    } else {
        qword->current_offset += pad;
    }
    return qword;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_next(struct kefir_abi_amd64_sysv_qwords *qwords,
                                                kefir_abi_amd64_sysv_data_class_t dataclass, kefir_size_t size,
                                                kefir_size_t alignment, struct kefir_abi_amd64_sysv_qword_ref *ref) {
    REQUIRE(qwords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord vector"));
    REQUIRE(size > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-zero data size"));
    REQUIRE(ref != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord reference"));
    struct kefir_abi_amd64_sysv_qword *first = NULL;
    while (size > 0) {
        struct kefir_abi_amd64_sysv_qword *current = next_qword(qwords, alignment);
        REQUIRE(current != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to obtain next qword"));
        if (first == NULL) {
            first = current;
            ref->qword = current;
            ref->offset = current->current_offset;
        }
        kefir_size_t available = KEFIR_AMD64_ABI_QWORD - current->current_offset;
        kefir_size_t chunk = MIN(available, size);
        current->current_offset += chunk;
        size -= chunk;
        current->klass = derive_dataclass(current->klass, dataclass);
        alignment = 1;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_align(struct kefir_abi_amd64_sysv_qwords *qwords, kefir_size_t alignment) {
    REQUIRE(qwords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord vector"));

    UNUSED(next_qword(qwords, alignment));
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_next_bitfield(struct kefir_abi_amd64_sysv_qwords *qwords,
                                                         kefir_abi_amd64_sysv_data_class_t dataclass,
                                                         kefir_size_t width,
                                                         struct kefir_abi_amd64_sysv_qword_ref *ref) {
    REQUIRE(qwords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord vector"));
    REQUIRE(width > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-zero bitfield width"));
    REQUIRE(ref != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord reference"));

    kefir_size_t size = width / 8 + (width % 8 != 0 ? 1 : 0);
    struct kefir_abi_amd64_sysv_qword *first = NULL;
    while (size > 0) {
        struct kefir_abi_amd64_sysv_qword *current = next_qword(qwords, 1);
        REQUIRE(current != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to obtain next qword"));
        if (first == NULL) {
            first = current;
            ref->qword = current;
            ref->offset = current->current_offset;
        }
        kefir_size_t available = KEFIR_AMD64_ABI_QWORD - current->current_offset;
        kefir_size_t chunk = MIN(available, size);
        current->current_offset += chunk;
        size -= chunk;
        current->klass = derive_dataclass(current->klass, dataclass);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_reset_class(struct kefir_abi_amd64_sysv_qwords *qwords,
                                                       kefir_abi_amd64_sysv_data_class_t dataclass, kefir_size_t begin,
                                                       kefir_size_t count) {
    REQUIRE(qwords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord vector"));
    const kefir_size_t length = kefir_vector_length(&qwords->qwords);
    REQUIRE(begin < length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Index exceeds QWord vector length"));
    REQUIRE(count > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-zero number of QWords"));
    for (kefir_size_t i = begin; i < MIN(length, begin + count); i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_amd64_sysv_qword *, qword, kefir_vector_at(&qwords->qwords, i));
        qword->klass = dataclass;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_save_position(const struct kefir_abi_amd64_sysv_qwords *qwords,
                                                         struct kefir_abi_amd64_sysv_qword_position *position) {
    REQUIRE(qwords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord vector"));
    REQUIRE(position != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord position"));
    position->index = qwords->current;
    ASSIGN_DECL_CAST(struct kefir_abi_amd64_sysv_qword *, qword, kefir_vector_at(&qwords->qwords, qwords->current));
    position->offset = qword->current_offset;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_restore_position(
    struct kefir_abi_amd64_sysv_qwords *qwords, const struct kefir_abi_amd64_sysv_qword_position *position) {
    REQUIRE(qwords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord vector"));
    REQUIRE(position != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid QWord position"));
    const kefir_size_t length = kefir_vector_length(&qwords->qwords);
    REQUIRE(position->index <= length,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Position index exceeds QWord vector length"));
    REQUIRE((position->offset <= 8 && position->index < length) || (position->offset == 0 && position->index == length),
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Position offset exceeds boundaries"));
    for (kefir_size_t i = position->index; i < length; i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_amd64_sysv_qword *, qword, kefir_vector_at(&qwords->qwords, i));
        if (i == position->index) {
            qword->current_offset = position->offset;
        } else {
            qword->current_offset = 0;
        }
    }
    qwords->current = position->index;
    return KEFIR_OK;
}

kefir_result_t kefir_abi_amd64_sysv_qwords_max_position(const struct kefir_abi_amd64_sysv_qword_position *first,
                                                        const struct kefir_abi_amd64_sysv_qword_position *second,
                                                        struct kefir_abi_amd64_sysv_qword_position *result) {
    REQUIRE(first != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid first position argument"));
    REQUIRE(second != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid second position argument"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid result pointer"));
    if (first->index > second->index || (first->index == second->index && first->offset >= second->offset)) {
        *result = *first;
    } else {
        *result = *second;
    }
    return KEFIR_OK;
}
