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

#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_PARAMETERS_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_PARAMETERS_H_

#include "kefir/target/abi/amd64/function.h"
#include "kefir/core/hashtree.h"

typedef enum kefir_codegen_opt_amd64_sysv_function_parameter_location_type {
    KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_DIRECT,
    KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_FLOATING_POINT_DIRECT,
    KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_INDIRECT,
    KEFIR_CODEGEN_OPT_AMD64_SYSV_FUNCTION_PARAMETER_LOCATION_REGISTER_AGGREGATE
} kefir_codegen_opt_amd64_sysv_function_parameter_location_type_t;

typedef struct kefir_codegen_opt_amd64_sysv_function_parameter_location {
    kefir_codegen_opt_amd64_sysv_function_parameter_location_type_t type;
    union {
        kefir_asm_amd64_xasmgen_register_t direct;
        struct {
            kefir_asm_amd64_xasmgen_register_t base;
            kefir_int64_t offset;
            kefir_bool_t aggregate;
        } indirect;
    };
    struct kefir_abi_amd64_function_parameter parameter;
    struct {
        kefir_size_t size;
        kefir_size_t alignment;
    } register_aggregate_props;
} kefir_codegen_opt_amd64_sysv_function_parameter_location_t;

typedef struct kefir_codegen_opt_amd64_sysv_function_parameters {
    const struct kefir_abi_amd64_function_decl *function_decl;
    struct kefir_hashtree parameters;
} kefir_codegen_opt_amd64_sysv_function_parameters_t;

kefir_result_t kefir_codegen_opt_amd64_sysv_function_parameters_init(
    struct kefir_mem *, const struct kefir_abi_amd64_function_decl *,
    struct kefir_codegen_opt_amd64_sysv_function_parameters *);
kefir_result_t kefir_codegen_opt_amd64_sysv_function_parameters_free(
    struct kefir_mem *, struct kefir_codegen_opt_amd64_sysv_function_parameters *);

kefir_result_t kefir_codegen_opt_amd64_sysv_function_parameter_location_of(
    const struct kefir_codegen_opt_amd64_sysv_function_parameters *, kefir_size_t,
    const struct kefir_codegen_opt_amd64_sysv_function_parameter_location **);

#endif
