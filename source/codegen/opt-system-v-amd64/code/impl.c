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

#include "kefir/codegen/opt-system-v-amd64/code_impl.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation(
    const struct kefir_codegen_opt_amd64_sysv_storage_location *location, kefir_bool_t *success, void *payload) {
    REQUIRE(location != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage location"));
    REQUIRE(success != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to payload"));

    *success = true;
    REQUIRE(location->type == KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_REGISTER, KEFIR_OK);

    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_sysv_amd64_register_allocation **, allocation_iter, payload);
    for (; *allocation_iter != NULL; ++allocation_iter) {
        if (((*allocation_iter)->result.type ==
                 KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
             (*allocation_iter)->result.type ==
                 KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) &&
            (*allocation_iter)->result.reg == location->reg) {
            *success = false;
            return KEFIR_OK;
        }
    }
    return KEFIR_OK;
}
