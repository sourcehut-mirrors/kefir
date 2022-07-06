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

#ifndef KEFIR_DRIVER_EXTERNALS_H_
#define KEFIR_DRIVER_EXTERNALS_H_

#include "kefir/core/mem.h"
#include "kefir/util/tempfile.h"

typedef struct kefir_driver_external_resources {
    const char *assembler_path;
    const char *linker_path;

    const char *runtime_library;

    const char *work_dir;

    struct kefir_tempfile_manager *tmpfile_manager;
} kefir_driver_external_resources_t;

kefir_result_t kefir_driver_external_resources_init_from_env(struct kefir_mem *,
                                                             struct kefir_driver_external_resources *,
                                                             struct kefir_tempfile_manager *);

#endif
