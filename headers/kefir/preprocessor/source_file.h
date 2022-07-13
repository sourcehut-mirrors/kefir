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

#ifndef KEFIR_PREPROCESSOR_SOURCE_FILE_H_
#define KEFIR_PREPROCESSOR_SOURCE_FILE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/lexer/source_cursor.h"

typedef struct kefir_preprocessor_source_file_info {
    const char *filepath;
    kefir_bool_t system;
    const char *base_include_dir;
} kefir_preprocessor_source_file_info_t;

typedef struct kefir_preprocessor_source_file {
    struct kefir_preprocessor_source_file_info info;
    struct kefir_lexer_source_cursor cursor;
    kefir_result_t (*close)(struct kefir_mem *, struct kefir_preprocessor_source_file *);
    void *payload;
} kefir_preprocessor_source_file_t;

typedef enum kefir_preprocessor_source_locator_mode {
    KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NORMAL,
    KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NEXT
} kefir_preprocessor_source_locator_mode_t;

typedef struct kefir_preprocessor_source_locator {
    kefir_result_t (*open)(struct kefir_mem *, const struct kefir_preprocessor_source_locator *, const char *,
                           kefir_bool_t, const struct kefir_preprocessor_source_file_info *,
                           kefir_preprocessor_source_locator_mode_t, struct kefir_preprocessor_source_file *);
    void *payload;
} kefir_preprocessor_source_locator_t;

#endif
