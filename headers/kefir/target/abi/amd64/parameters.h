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

#ifndef KEFIR_TARGET_ABI_AMD64_PARAMETERS_H_
#define KEFIR_TARGET_ABI_AMD64_PARAMETERS_H_

#include <stdbool.h>
#include "kefir/core/basic-types.h"
#include "kefir/core/vector.h"
#include "kefir/core/mem.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/target/abi/amd64/system-v/qwords.h"
#include "kefir/target/asm/amd64/xasmgen.h"

kefir_size_t kefir_abi_amd64_num_of_general_purpose_parameter_registers(kefir_abi_amd64_variant_t);
kefir_result_t kefir_abi_amd64_general_purpose_parameter_register(kefir_abi_amd64_variant_t, kefir_size_t,
                                                                  kefir_asm_amd64_xasmgen_register_t *);
kefir_size_t kefir_abi_amd64_num_of_sse_parameter_registers(kefir_abi_amd64_variant_t);
kefir_result_t kefir_abi_amd64_sse_parameter_register(kefir_abi_amd64_variant_t, kefir_size_t,
                                                      kefir_asm_amd64_xasmgen_register_t *);

typedef enum kefir_abi_amd64_function_parameter_location {
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE,
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER,
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER,
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87,
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP,
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS,
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY,
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED
} kefir_abi_amd64_function_parameter_location_t;

typedef struct kefir_abi_amd64_function_parameter {
    kefir_bool_t preliminary_classification;
    kefir_abi_amd64_function_parameter_location_t location;
    union {
        kefir_asm_amd64_xasmgen_register_t direct_reg;
        kefir_int64_t tos_offset;
        void *multi_reg;
        struct {
            kefir_abi_amd64_function_parameter_location_t location;
            kefir_size_t index;
            kefir_size_t allocation;
            kefir_size_t offset;
        } nested;
    };
    struct {
        kefir_size_t general_purpose_regs;
        kefir_size_t sse_regs;
        kefir_size_t sseup_regs;
        kefir_size_t x87;
        kefir_size_t memory_size;
        kefir_size_t memory_alignment;
    } requirements;
    void *payload;
} kefir_abi_amd64_function_parameter_t;

typedef enum kefir_abi_amd64_function_parameter_memory_basis {
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER,
    KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE
} kefir_abi_amd64_function_parameter_memory_basis_t;

typedef struct kefir_abi_amd64_function_parameter_requirements {
    kefir_size_t general_purpose_regs;
    kefir_size_t sse_regs;
    kefir_size_t stack;
} kefir_abi_amd64_function_parameter_requirements_t;

typedef struct kefir_abi_amd64_function_parameters {
    void *payload;
} kefir_abi_amd64_function_parameters_t;

kefir_result_t kefir_abi_amd64_function_parameters_classify(struct kefir_mem *, kefir_abi_amd64_variant_t,
                                                            const struct kefir_ir_type *,
                                                            const struct kefir_abi_amd64_type_layout *,
                                                            struct kefir_abi_amd64_function_parameters *);
kefir_result_t kefir_abi_amd64_function_parameters_allocate(struct kefir_mem *,
                                                            struct kefir_abi_amd64_function_parameters *);
kefir_result_t kefir_abi_amd64_function_parameters_allocate_return(struct kefir_mem *,
                                                                   struct kefir_abi_amd64_function_parameters *);
kefir_result_t kefir_abi_amd64_function_parameters_reserve(
    const struct kefir_abi_amd64_function_parameters *, const struct kefir_abi_amd64_function_parameter_requirements *);
kefir_result_t kefir_abi_amd64_function_parameters_free(struct kefir_mem *,
                                                        struct kefir_abi_amd64_function_parameters *);

kefir_result_t kefir_abi_amd64_function_parameters_at(const struct kefir_abi_amd64_function_parameters *, kefir_size_t,
                                                      struct kefir_abi_amd64_function_parameter *);
kefir_result_t kefir_abi_amd64_function_parameters_requirements(
    const struct kefir_abi_amd64_function_parameters *, struct kefir_abi_amd64_function_parameter_requirements *);
kefir_result_t kefir_abi_amd64_function_parameter_multireg_length(const struct kefir_abi_amd64_function_parameter *,
                                                                  kefir_size_t *);
kefir_result_t kefir_abi_amd64_function_parameter_multireg_at(const struct kefir_abi_amd64_function_parameter *,
                                                              kefir_size_t,
                                                              struct kefir_abi_amd64_function_parameter *);
kefir_result_t kefir_abi_amd64_function_parameter_memory_location(const struct kefir_abi_amd64_function_parameter *,
                                                                  kefir_abi_amd64_function_parameter_memory_basis_t,
                                                                  kefir_asm_amd64_xasmgen_register_t *,
                                                                  kefir_int64_t *);

#endif
