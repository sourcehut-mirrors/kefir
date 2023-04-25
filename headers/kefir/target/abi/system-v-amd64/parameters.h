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

#ifndef KEFIR_TARGET_ABI_SYSTEM_V_AMD64_PARAMETERS_H_
#define KEFIR_TARGET_ABI_SYSTEM_V_AMD64_PARAMETERS_H_

#include <stdbool.h>
#include "kefir/core/basic-types.h"
#include "kefir/core/vector.h"
#include "kefir/core/mem.h"
#include "kefir/target/abi/system-v-amd64/data_layout.h"
#include "kefir/target/abi/system-v-amd64/qwords.h"
#include "kefir/target/asm/amd64/xasmgen.h"

extern kefir_asm_amd64_xasmgen_register_t KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[];
extern const kefir_size_t KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTER_COUNT;
extern kefir_asm_amd64_xasmgen_register_t KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS[];
extern const kefir_size_t KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTER_COUNT;

typedef enum kefir_abi_sysv_amd64_parameter_type {
    KEFIR_AMD64_SYSV_INPUT_PARAM_IMMEDIATE,
    KEFIR_AMD64_SYSV_INPUT_PARAM_NESTED,
    KEFIR_AMD64_SYSV_INPUT_PARAM_OWNING_CONTAINER,
    KEFIR_AMD64_SYSV_INPUT_PARAM_CONTAINER,
    KEFIR_AMD64_SYSV_INPUT_PARAM_SKIP
} kefir_abi_sysv_amd64_parameter_type_t;

#define KEFIR_AMD64_SYSV_PARAMETER_LOCATION_NONE KEFIR_SIZE_MAX

typedef struct kefir_abi_sysv_amd64_parameter_location {
    kefir_size_t integer_register;
    kefir_size_t sse_register;
    kefir_size_t stack_offset;
} kefir_abi_sysv_amd64_parameter_location_t;

typedef struct kefir_abi_sysv_amd64_parameter_location_requirements {
    kefir_size_t integer;
    kefir_size_t sse;
    kefir_size_t sseup;
    kefir_size_t x87;
    struct {
        kefir_size_t size;
        kefir_size_t alignment;
    } memory;
} kefir_abi_sysv_amd64_parameter_location_requirements_t;

typedef struct kefir_abi_sysv_amd64_parameter_allocation {
    kefir_abi_sysv_amd64_parameter_type_t type;
    kefir_abi_sysv_amd64_data_class_t klass;
    kefir_size_t index;
    union {
        struct kefir_abi_sysv_amd64_qwords container;
        struct kefir_abi_sysv_amd64_qword_ref container_reference;
    };
    struct kefir_abi_sysv_amd64_parameter_location_requirements requirements;
    struct kefir_abi_sysv_amd64_parameter_location location;
} kefir_abi_sysv_amd64_parameter_allocation_t;

kefir_result_t kefir_abi_sysv_amd64_parameter_classify(struct kefir_mem *, const struct kefir_ir_type *,
                                                       const struct kefir_abi_sysv_amd64_type_layout *,
                                                       struct kefir_vector *);

kefir_result_t kefir_abi_sysv_amd64_parameter_free(struct kefir_mem *, struct kefir_vector *);

kefir_result_t kefir_abi_sysv_amd64_parameter_allocate(struct kefir_mem *, const struct kefir_ir_type *,
                                                       const struct kefir_abi_sysv_amd64_type_layout *,
                                                       struct kefir_vector *,
                                                       struct kefir_abi_sysv_amd64_parameter_location *);

#endif
