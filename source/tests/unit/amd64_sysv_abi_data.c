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

#include <stdalign.h>
#include <inttypes.h>
#include "kefir/test/unit_test.h"
#include "kefir/ir/type.h"
#include "kefir/ir/builder.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/codegen/system-v-amd64/registers.h"

#define ASSERT_DATA_ALLOC(vector, index, _size, _alignment, offset)                                             \
    do {                                                                                                        \
        ASSIGN_DECL_CAST(struct kefir_abi_amd64_typeentry_layout *, param, kefir_vector_at((vector), (index))); \
        ASSERT(param->size == (_size));                                                                         \
        ASSERT(param->alignment == (_alignment));                                                               \
        ASSERT(param->relative_offset == (offset));                                                             \
    } while (0)

DEFINE_CASE(amd64_sysv_abi_data_test1, "AMD64 System V ABI - multiple scalars") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout vector;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 3, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &vector));
    ASSERT_DATA_ALLOC(&vector.layout, 0, 8, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 1, 1, 1, 8);
    ASSERT_DATA_ALLOC(&vector.layout, 2, 4, 4, 12);
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_data_test2, "AMD64 System V ABI - single struct") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout vector;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 5, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 4));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &vector));
    ASSERT_DATA_ALLOC(&vector.layout, 0, 16, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 1, 8, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 2, 1, 1, 8);
    ASSERT_DATA_ALLOC(&vector.layout, 3, 4, 4, 12);
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_data_test3, "AMD64 System V ABI - single union") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout vector;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 5, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_UNION, 0, 4));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &vector));
    ASSERT_DATA_ALLOC(&vector.layout, 0, 8, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 1, 8, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 2, 1, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 3, 4, 4, 0);
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_data_test4, "AMD64 System V ABI - nested structs") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout vector;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 11, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 4));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 3));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_WORD, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_UNION, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_CHAR, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &vector));
    ASSERT_DATA_ALLOC(&vector.layout, 0, 48, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 1, 1, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 2, 24, 8, 8);
    ASSERT_DATA_ALLOC(&vector.layout, 3, 8, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 4, 8, 8, 8);
    ASSERT_DATA_ALLOC(&vector.layout, 5, 4, 4, 16);
    ASSERT_DATA_ALLOC(&vector.layout, 6, 4, 4, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 7, 1, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 8, 8, 8, 32);
    ASSERT_DATA_ALLOC(&vector.layout, 9, 1, 1, 40);
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_data_test5, "AMD64 System V ABI - unaligned data") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout vector;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 6, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 3));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 1, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 1, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_WORD, 1, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_FLOAT64, 1, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &vector));
    ASSERT_DATA_ALLOC(&vector.layout, 0, 25, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 1, 1, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 2, 16, 1, 1);
    ASSERT_DATA_ALLOC(&vector.layout, 3, 8, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 4, 8, 1, 8);
    ASSERT_DATA_ALLOC(&vector.layout, 5, 8, 1, 17);
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_data_test6, "AMD64 System V ABI - array") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout vector;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 4, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 10));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_WORD, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &vector));
    ASSERT_DATA_ALLOC(&vector.layout, 0, 160, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 1, 16, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 2, 1, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 3, 8, 8, 8);
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_data_test7, "AMD64 System V ABI - array #2") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout vector;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 8, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 5));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_ARRAY, 0, 100));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_CHAR, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 3));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &vector));
    ASSERT_DATA_ALLOC(&vector.layout, 0, 600, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 1, 120, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 2, 100, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 3, 1, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 4, 16, 8, 104);
    ASSERT_DATA_ALLOC(&vector.layout, 5, 8, 8, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 6, 2, 2, 8);
    ASSERT_DATA_ALLOC(&vector.layout, 7, 2, 2, 10);
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_data_test8, "AMD64 System V ABI - eightbyte allocation") {
    struct kefir_ir_type type;
    struct kefir_abi_amd64_type_layout vector;
    struct kefir_abi_amd64_sysv_qwords qwords;
    struct kefir_abi_amd64_sysv_qword_ref ref;
    kefir_size_t qword_count;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 11, &type));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_STRUCT, 0, 10));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT16, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT64, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT32, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type, &vector));
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_count(&type, &vector.layout, &qword_count));
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_alloc(&qwords, &kft_mem, qword_count));
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 1, 1, &ref));
    ASSERT(ref.qword->index == 0 && ref.offset == 0);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 1, 1, &ref));
    ASSERT(ref.qword->index == 0 && ref.offset == 1);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 4, 4, &ref));
    ASSERT(ref.qword->index == 0 && ref.offset == 4);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 2, 2, &ref));
    ASSERT(ref.qword->index == 1 && ref.offset == 0);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 8, 8, &ref));
    ASSERT(ref.qword->index == 2 && ref.offset == 0);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 4, 4, &ref));
    ASSERT(ref.qword->index == 3 && ref.offset == 0);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 1, 1, &ref));
    ASSERT(ref.qword->index == 3 && ref.offset == 4);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 4, 4, &ref));
    ASSERT(ref.qword->index == 4 && ref.offset == 0);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 1, 1, &ref));
    ASSERT(ref.qword->index == 4 && ref.offset == 4);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_next(&qwords, KEFIR_AMD64_SYSV_PARAM_INTEGER, 1, 1, &ref));
    ASSERT(ref.qword->index == 4 && ref.offset == 5);
    ASSERT_OK(kefir_abi_amd64_sysv_qwords_free(&qwords, &kft_mem));
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type));
}
END_CASE

DEFINE_CASE(amd64_sysv_abi_data_test9, "AMD64 System V ABI - external structure definitions") {
    struct kefir_ir_type type1, type2;
    struct kefir_abi_amd64_type_layout vector;
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 3, &type1));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_STRUCT, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_INT8, 0, 0));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type1, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    ASSERT_OK(kefir_ir_type_alloc(&kft_mem, 4, &type2));
    ASSERT_OK(kefir_irbuilder_type_append(&kft_mem, &type2, KEFIR_IR_TYPE_ARRAY, 0, 2));
    ASSERT_OK(kefir_irbuilder_type_append_from(&kft_mem, &type2, &type1, 0));
    ASSERT_OK(kefir_abi_amd64_type_layout(&kft_mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, &type2, &vector));
    ASSERT_DATA_ALLOC(&vector.layout, 0, 16, 4, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 1, 8, 4, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 2, 1, 1, 0);
    ASSERT_DATA_ALLOC(&vector.layout, 3, 4, 4, 4);
    ASSERT_OK(kefir_abi_amd64_type_layout_free(&kft_mem, &vector));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type2));
    ASSERT_OK(kefir_ir_type_free(&kft_mem, &type1));
}
END_CASE
