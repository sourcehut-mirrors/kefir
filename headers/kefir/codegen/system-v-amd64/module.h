/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#ifndef KEFIR_CODEGEN_SYSTEM_V_AMD64_MODULE_H_
#define KEFIR_CODEGEN_SYSTEM_V_AMD64_MODULE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/codegen/codegen.h"
#include "kefir/core/hashtree.h"

typedef struct kefir_codegen_amd64_sysv_module {
    struct kefir_ir_module *module;
    struct kefir_hashtree function_gates;
    struct kefir_hashtree function_vgates;
    struct kefir_hashtree type_layouts;
    struct kefir_hashtree tls_entries;
    struct kefir_hashtree inline_assembly;
    kefir_id_t inline_assembly_next_id;
} kefir_codegen_amd64_sysv_module_t;

kefir_result_t kefir_codegen_amd64_sysv_module_alloc(struct kefir_mem *, struct kefir_codegen_amd64_sysv_module *,
                                                     struct kefir_ir_module *);
kefir_result_t kefir_codegen_amd64_sysv_module_free(struct kefir_mem *, struct kefir_codegen_amd64_sysv_module *);
struct kefir_amd64_sysv_function_decl *kefir_codegen_amd64_sysv_module_function_decl(
    struct kefir_mem *, struct kefir_codegen_amd64_sysv_module *, kefir_id_t, bool);
struct kefir_abi_sysv_amd64_type_layout *kefir_codegen_amd64_sysv_module_type_layout(
    struct kefir_mem *, struct kefir_codegen_amd64_sysv_module *, kefir_id_t);
kefir_result_t kefir_codegen_amd64_sysv_module_declare_tls(struct kefir_mem *, struct kefir_codegen_amd64_sysv_module *,
                                                           const char *);

#endif
