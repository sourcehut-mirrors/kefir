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

#ifndef KEFIR_PLATFORM_FILESYSTEM_H_
#define KEFIR_PLATFORM_FILESYSTEM_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/string_pool.h"

typedef struct kefir_filesystem_path_list_iter {
    const char *current_list;
    char separator;
} kefir_filesystem_path_list_iter_t;

const char *kefir_filesystem_path_list_iter_init(struct kefir_filesystem_path_list_iter *, const char *, char,
                                                 kefir_size_t *);
const char *kefir_filesystem_path_list_iter_next(struct kefir_filesystem_path_list_iter *, kefir_size_t *);

kefir_result_t kefir_filesystem_find_in_path_list(struct kefir_mem *, struct kefir_string_pool *, const char *,
                                                  const char *, const char **result);

#endif
