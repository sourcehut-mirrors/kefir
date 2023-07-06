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

#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_FUNCTION_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_FUNCTION_H_

#include "kefir/codegen/opt-system-v-amd64.h"
#include "kefir/codegen/opt-system-v-amd64/register_allocator.h"
#include "kefir/codegen/opt-system-v-amd64/parameters.h"
#include "kefir/codegen/opt-system-v-amd64/stack_frame.h"
#include "kefir/codegen/opt-system-v-amd64/storage.h"
#include "kefir/target/abi/system-v-amd64/function.h"
#include "kefir/target/abi/system-v-amd64/data_layout.h"

typedef struct kefir_opt_sysv_amd64_function {
    struct kefir_abi_amd64_sysv_function_decl declaration;
    struct kefir_codegen_opt_amd64_sysv_function_parameters parameters;
    struct kefir_codegen_opt_sysv_amd64_stack_frame stack_frame;
    struct kefir_codegen_opt_sysv_amd64_register_allocator register_allocator;
    struct kefir_abi_sysv_amd64_type_layout locals_layout;

    struct kefir_codegen_opt_sysv_amd64_stack_frame_map stack_frame_map;
    struct kefir_codegen_opt_sysv_amd64_storage storage;
    struct kefir_list alive_instr;
    kefir_id_t nonblock_labels;
    kefir_id_t inline_asm_id;
} kefir_opt_sysv_amd64_function_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_function(struct kefir_mem *, struct kefir_codegen_opt_amd64 *,
                                                               struct kefir_opt_module *,
                                                               const struct kefir_opt_function *,
                                                               const struct kefir_opt_code_analysis *);

#endif
