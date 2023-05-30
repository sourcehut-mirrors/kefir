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

#include "kefir/codegen/opt-common/virtual_register_allocator.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

kefir_result_t kefir_codegen_opt_virtual_register_allocator_init(
    struct kefir_mem *mem, struct kefir_codegen_opt_virtual_register_allocator *regs, kefir_size_t total_registers) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen register allocator"));

    regs->register_allocation = KEFIR_MALLOC(mem, sizeof(kefir_bool_t) * total_registers);
    REQUIRE(regs->register_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer codegen register state"));
    memset(regs->register_allocation, 0, sizeof(kefir_bool_t) * total_registers);
    regs->storage_allocation = NULL;
    regs->total_registers = total_registers;
    regs->total_storage = 0;
    regs->storage_capacity = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_virtual_register_allocator_free(
    struct kefir_mem *mem, struct kefir_codegen_opt_virtual_register_allocator *regs) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocator"));

    KEFIR_FREE(mem, regs->register_allocation);
    KEFIR_FREE(mem, regs->storage_allocation);
    memset(regs, 0, sizeof(struct kefir_codegen_opt_virtual_register_allocator));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_virtual_register_allocator_is_available(
    const struct kefir_codegen_opt_virtual_register_allocator *regs, kefir_codegen_opt_virtual_register_t vreg,
    kefir_bool_t *available) {
    REQUIRE(regs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocator"));
    REQUIRE(vreg < regs->total_registers + regs->total_storage,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided optimizer codegen virtual register is out of bounds"));
    REQUIRE(available != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to a boolean"));

    if (vreg < regs->total_registers) {
        *available = regs->register_allocation[vreg];
    } else {
        *available = regs->storage_allocation[vreg - regs->total_registers];
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_virtual_register_allocator_allocate(
    const struct kefir_codegen_opt_virtual_register_allocator *regs, kefir_codegen_opt_virtual_register_t vreg,
    kefir_bool_t *success) {
    REQUIRE(regs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocator"));
    REQUIRE(vreg < regs->total_registers + regs->total_storage,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided optimizer codegen virtual register is out of bounds"));
    REQUIRE(success != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to a boolean"));

    *success = false;
    if (vreg < regs->total_registers) {
        if (!regs->register_allocation[vreg]) {
            regs->register_allocation[vreg] = true;
            *success = true;
        }
    } else {
        vreg -= regs->total_registers;
        if (!regs->storage_allocation[vreg]) {
            regs->storage_allocation[vreg] = true;
            *success = true;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_virtual_register_allocator_allocate_register(
    const struct kefir_codegen_opt_virtual_register_allocator *regs, kefir_codegen_opt_virtual_register_t *vreg,
    kefir_result_t (*filter)(kefir_codegen_opt_virtual_register_t, kefir_bool_t *, void *), void *filter_payload) {
    REQUIRE(regs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocator"));
    REQUIRE(vreg != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen virtual register"));

    for (kefir_size_t i = 0; i < regs->total_registers; i++) {
        if (!regs->register_allocation[i]) {
            kefir_bool_t fits = true;
            if (filter != NULL) {
                REQUIRE_OK(filter(i, &fits, filter_payload));
            }
            if (fits) {
                regs->register_allocation[i] = true;
                *vreg = i;
                return KEFIR_OK;
            }
        }
    }
    return KEFIR_SET_ERROR(KEFIR_OUT_OF_SPACE, "Unable to allocate an optimizer codegen virtual register");
}

kefir_result_t kefir_codegen_opt_virtual_register_allocator_allocate_storage(
    struct kefir_mem *mem, struct kefir_codegen_opt_virtual_register_allocator *regs,
    kefir_codegen_opt_virtual_register_t *vreg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocator"));
    REQUIRE(vreg != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen virtual register"));

    for (kefir_size_t i = 0; i < regs->total_storage; i++) {
        if (!regs->storage_allocation[i]) {
            regs->storage_allocation[i] = true;
            *vreg = i + regs->total_registers;
            return KEFIR_OK;
        }
    }

    if (regs->total_storage == regs->storage_capacity) {
        kefir_size_t new_storage_capacity = regs->storage_capacity + 32;
        kefir_bool_t *new_storage = KEFIR_MALLOC(mem, sizeof(kefir_bool_t) * new_storage_capacity);
        REQUIRE(new_storage != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE,
                                                     "Failed to allocate optimizer codegen virtual register storage"));
        memcpy(new_storage, regs->storage_allocation, sizeof(kefir_bool_t) * regs->storage_capacity);
        memset(&new_storage[regs->storage_capacity], 0,
               sizeof(kefir_bool_t) * (new_storage_capacity - regs->storage_capacity));
        KEFIR_FREE(mem, regs->storage_allocation);
        regs->storage_allocation = new_storage;
        regs->storage_capacity = new_storage_capacity;
    }

    regs->storage_allocation[regs->total_storage] = true;
    *vreg = regs->total_storage + regs->total_registers;
    regs->total_storage++;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_virtual_register_allocator_allocate_any(
    struct kefir_mem *mem, struct kefir_codegen_opt_virtual_register_allocator *regs,
    kefir_codegen_opt_virtual_register_t *vreg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(regs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocator"));
    REQUIRE(vreg != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer codegen virtual register"));

    kefir_result_t res = kefir_codegen_opt_virtual_register_allocator_allocate_register(regs, vreg, NULL, NULL);
    if (res == KEFIR_OUT_OF_SPACE) {
        res = kefir_codegen_opt_virtual_register_allocator_allocate_storage(mem, regs, vreg);
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_virtual_register_allocator_deallocate(
    const struct kefir_codegen_opt_virtual_register_allocator *regs, kefir_codegen_opt_virtual_register_t vreg) {
    REQUIRE(regs != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen register allocator"));
    REQUIRE(vreg < regs->total_registers + regs->total_storage,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided optimizer codegen virtual register is out of bounds"));

    if (vreg < regs->total_registers) {
        regs->register_allocation[vreg] = false;
    } else {
        regs->storage_allocation[vreg - regs->total_registers] = false;
    }
    return KEFIR_OK;
}
