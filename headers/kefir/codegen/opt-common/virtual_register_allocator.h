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

#ifndef KEFIR_CODEGEN_OPT_COMMON_VIRTUAL_REGISTER_ALLOCATOR_H_
#define KEFIR_CODEGEN_OPT_COMMON_VIRTUAL_REGISTER_ALLOCATOR_H_

#include "kefir/optimizer/analysis.h"

typedef kefir_size_t kefir_codegen_opt_virtual_register_t;

typedef struct kefir_codegen_opt_virtual_register_allocator {
    kefir_size_t total_registers;
    kefir_size_t total_storage;
    kefir_size_t storage_capacity;
    kefir_bool_t *register_allocation;
    kefir_bool_t *storage_allocation;
} kefir_codegen_opt_virtual_register_allocator_t;

kefir_result_t kefir_codegen_opt_virtual_register_allocator_init(struct kefir_mem *,
                                                                 struct kefir_codegen_opt_virtual_register_allocator *,
                                                                 kefir_size_t);
kefir_result_t kefir_codegen_opt_virtual_register_allocator_free(struct kefir_mem *,
                                                                 struct kefir_codegen_opt_virtual_register_allocator *);

kefir_result_t kefir_codegen_opt_virtual_register_allocator_is_available(
    const struct kefir_codegen_opt_virtual_register_allocator *, kefir_codegen_opt_virtual_register_t, kefir_bool_t *);
kefir_result_t kefir_codegen_opt_virtual_register_allocator_allocate(
    const struct kefir_codegen_opt_virtual_register_allocator *, kefir_codegen_opt_virtual_register_t, kefir_bool_t *);
kefir_result_t kefir_codegen_opt_virtual_register_allocator_allocate_register(
    const struct kefir_codegen_opt_virtual_register_allocator *, kefir_codegen_opt_virtual_register_t *,
    kefir_result_t (*)(kefir_codegen_opt_virtual_register_t, kefir_bool_t *, void *), void *);
kefir_result_t kefir_codegen_opt_virtual_register_allocator_allocate_storage(
    struct kefir_mem *, struct kefir_codegen_opt_virtual_register_allocator *, kefir_codegen_opt_virtual_register_t *);
kefir_result_t kefir_codegen_opt_virtual_register_allocator_allocate_any(
    struct kefir_mem *, struct kefir_codegen_opt_virtual_register_allocator *, kefir_codegen_opt_virtual_register_t *);
kefir_result_t kefir_codegen_opt_virtual_register_allocator_deallocte(
    const struct kefir_codegen_opt_virtual_register_allocator *, kefir_codegen_opt_virtual_register_t);

#endif
