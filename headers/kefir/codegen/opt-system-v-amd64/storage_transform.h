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

#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_STORAGE_TRANSFORM_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_STORAGE_TRANSFORM_H_

#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/codegen/opt-system-v-amd64/storage.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/hashtreeset.h"

typedef struct kefir_codegen_opt_amd64_sysv_storage_transform {
    struct kefir_hashtree map;
    struct kefir_hashtreeset active_regs;
} kefir_codegen_opt_amd64_sysv_storage_transform_t;

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_init(
    struct kefir_codegen_opt_amd64_sysv_storage_transform *);
kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_free(
    struct kefir_mem *, struct kefir_codegen_opt_amd64_sysv_storage_transform *);

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_insert(
    struct kefir_mem *, struct kefir_codegen_opt_amd64_sysv_storage_transform *,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *,
    const struct kefir_codegen_opt_amd64_sysv_storage_location *);

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_reset(
    struct kefir_mem *, struct kefir_codegen_opt_amd64_sysv_storage_transform *);

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_operations(
    const struct kefir_codegen_opt_amd64_sysv_storage_transform *, kefir_size_t *);

kefir_result_t kefir_codegen_opt_amd64_sysv_storage_transform_perform(
    struct kefir_mem *, struct kefir_codegen_opt_amd64 *, struct kefir_codegen_opt_sysv_amd64_storage *,
    const struct kefir_codegen_opt_sysv_amd64_stack_frame_map *,
    const struct kefir_codegen_opt_amd64_sysv_storage_transform *);

#endif
