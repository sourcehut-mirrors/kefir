/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifndef KEFIR_PLATFORM_TEMPFILE_H_
#define KEFIR_PLATFORM_TEMPFILE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/mem.h"

typedef struct kefir_tempfile_manager {
    const char *basedir;
    kefir_size_t file_index;
    struct kefir_hashtree tracked_files;
} kefir_tempfile_manager_t;

kefir_result_t kefir_tempfile_manager_init(struct kefir_tempfile_manager *);
kefir_result_t kefir_tempfile_manager_free(struct kefir_mem *, struct kefir_tempfile_manager *);

kefir_result_t kefir_tempfile_manager_create_file(struct kefir_mem *, struct kefir_tempfile_manager *, const char *,
                                                  const char **);

#endif
