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

#include "kefir/driver/externals.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_driver_external_resources_init_from_env(struct kefir_mem *mem,
                                                             struct kefir_driver_external_resources *externals,
                                                             struct kefir_tempfile_manager *tmpmgr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(externals != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver external resources"));
    REQUIRE(tmpmgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile manager"));

    externals->tmpfile_manager = tmpmgr;

    externals->assembler_path = getenv("KEFIR_AS");
    if (externals->assembler_path == NULL) {
        externals->assembler_path = getenv("AS");
    }
    if (externals->assembler_path == NULL) {
        externals->assembler_path = "as";
    }

    externals->linker_path = getenv("KEFIR_LD");
    if (externals->linker_path == NULL) {
        externals->linker_path = getenv("LD");
    }
    if (externals->linker_path == NULL) {
        externals->linker_path = "ld";
    }

    externals->runtime_library = getenv("KEFIR_RTLIB");
    REQUIRE(externals->runtime_library != NULL,
            KEFIR_SET_ERROR(KEFIR_ENVIRONMENT_ERROR, "Expected KEFIR_RTLIB to contain path to runtime library"));

    externals->work_dir = getenv("KEFIR_WORKDIR");
    if (externals->work_dir == NULL) {
        REQUIRE_OK(kefir_tempfile_manager_tmpdir(mem, tmpmgr, "kefir-workdir-XXXXXX", &externals->work_dir));
    }
    return KEFIR_OK;
}
