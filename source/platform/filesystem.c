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

#define _POSIX_SOURCE
#include "kefir/platform/filesystem.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/os_error.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

const char *kefir_filesystem_path_list_iter_init(struct kefir_filesystem_path_list_iter *iter, const char *list,
                                                 char separator, kefir_size_t *length) {
    REQUIRE(iter != NULL, NULL);

    iter->current_list = list;
    iter->separator = separator;
    return kefir_filesystem_path_list_iter_next(iter, length);
}

const char *kefir_filesystem_path_list_iter_next(struct kefir_filesystem_path_list_iter *iter, kefir_size_t *length) {
    REQUIRE(iter != NULL, NULL);
    REQUIRE(iter->current_list != NULL, NULL);
    REQUIRE(strlen(iter->current_list) > 0, NULL);

    char *delim = strchr(iter->current_list, iter->separator);
    const char *result = iter->current_list;
    if (delim != NULL) {
        ASSIGN_PTR(length, delim - iter->current_list);
        iter->current_list = delim + 1;
    } else {
        ASSIGN_PTR(length, strlen(result));
        iter->current_list = NULL;
    }
    return result;
}

kefir_result_t kefir_filesystem_find_in_path_list(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                                  const char *paths, const char *name, const char **result) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(symbols != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbol table"));
    REQUIRE(paths != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid filesystem path list"));
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file name"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to result"));

    struct kefir_filesystem_path_list_iter iter;
    kefir_size_t length;
    char buffer[PATH_MAX + 1];
    for (const char *path = kefir_filesystem_path_list_iter_init(&iter, paths, ';', &length); path != NULL;
         path = kefir_filesystem_path_list_iter_next(&iter, &length)) {

        if (length == 0) {
            continue;
        }
        snprintf(buffer, sizeof(buffer) - 1, "%.*s/%s", (int) length, path, name);

        if (access(buffer, F_OK) == 0) {
            *result = kefir_string_pool_insert(mem, symbols, buffer, NULL);
            REQUIRE(*result != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert file path into symbol table"));
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested file in path list");
}
