/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#ifndef KEFIR_PLATFORM_FILESYSTEM_SOURCE_H_
#define KEFIR_PLATFORM_FILESYSTEM_SOURCE_H_

#include "kefir/preprocessor/source_file.h"
#include "kefir/core/list.h"
#include "kefir/core/string_pool.h"
#include "kefir/core/hashtreeset.h"

typedef struct kefir_preprocessor_filesystem_source_locator {
    struct kefir_preprocessor_source_locator locator;
    struct kefir_list include_roots;
    struct kefir_list embed_roots;
    struct kefir_hashtreeset include_root_set;
    struct kefir_hashtreeset embed_root_set;
    struct kefir_string_pool *symbols;
} kefir_preprocessor_filesystem_source_locator_t;

kefir_result_t kefir_preprocessor_filesystem_source_locator_init(struct kefir_preprocessor_filesystem_source_locator *,
                                                                 struct kefir_string_pool *);
kefir_result_t kefir_preprocessor_filesystem_source_locator_free(struct kefir_mem *,
                                                                 struct kefir_preprocessor_filesystem_source_locator *);
kefir_result_t kefir_preprocessor_filesystem_source_locator_append(
    struct kefir_mem *, struct kefir_preprocessor_filesystem_source_locator *, const char *, kefir_bool_t);
kefir_result_t kefir_preprocessor_filesystem_source_locator_append_embed(
    struct kefir_mem *, struct kefir_preprocessor_filesystem_source_locator *, const char *);

#endif
