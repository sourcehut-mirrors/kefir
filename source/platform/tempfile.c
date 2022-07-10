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

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700
#include "kefir/platform/tempfile.h"
#include "kefir/core/error.h"
#include "kefir/core/os_error.h"
#include "kefir/core/util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ftw.h>

int dir_traverse_remove(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    UNUSED(sb);
    UNUSED(typeflag);
    UNUSED(ftwbuf);

    int rv = remove(fpath);
    if (rv != 0) {
        perror(fpath);
    }
    return rv;
}

static kefir_result_t remove_entry(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                   kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(value);
    UNUSED(payload);

    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(char *, name, key);
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hashtree key"));

    if (access(name, F_OK) == 0) {
        struct stat statbuf;
        REQUIRE(stat(name, &statbuf) == 0, KEFIR_SET_OS_ERROR("Failed to obtain tempfile status"));

        if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
            // Remove directory
            REQUIRE(nftw(name, dir_traverse_remove, 64, FTW_DEPTH | FTW_PHYS) == 0,
                    KEFIR_SET_OS_ERROR("Failed to remove tempfile directory"));
        } else {
            REQUIRE(remove(name) == 0, KEFIR_SET_OS_ERROR("Failed to remove tempfile"));
        }
    }

    KEFIR_FREE(mem, name);
    return KEFIR_OK;
}

kefir_result_t kefir_tempfile_manager_init(struct kefir_tempfile_manager *mgr) {
    REQUIRE(mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to tempfile manager"));

    REQUIRE_OK(kefir_hashtree_init(&mgr->tracked_files, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&mgr->tracked_files, remove_entry, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_tempfile_manager_free(struct kefir_mem *mem, struct kefir_tempfile_manager *mgr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile manager"));

    REQUIRE_OK(kefir_hashtree_free(mem, &mgr->tracked_files));
    return KEFIR_OK;
}

kefir_result_t kefir_tempfile_manager_create_file(struct kefir_mem *mem, struct kefir_tempfile_manager *mgr,
                                                  const char *template, const char **result) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile manager"));
    REQUIRE(template != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile template"));
    REQUIRE(result != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to generated tempfile name"));

    char *name = KEFIR_MALLOC(mem, strlen(template) + 1);
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate tempfile name"));
    strcpy(name, template);

    REQUIRE_ELSE(mktemp(name) == name, {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to generate temporary file name");
        KEFIR_FREE(mem, name);
        return res;
    });

    kefir_result_t res =
        kefir_hashtree_insert(mem, &mgr->tracked_files, (kefir_hashtree_key_t) name, (kefir_hashtree_value_t) 0);
    REQUIRE_ELSE(res == KEFIR_OK, {
        remove(name);
        free(name);
        return res;
    });

    *result = name;
    return KEFIR_OK;
}

kefir_result_t kefir_tempfile_manager_create_directory(struct kefir_mem *mem, struct kefir_tempfile_manager *mgr,
                                                       const char *template, const char **result) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile manager"));
    REQUIRE(template != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile template"));
    REQUIRE(result != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to generated tempfile name"));

    char *name = KEFIR_MALLOC(mem, strlen(template) + 1);
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate tempfile name"));
    strcpy(name, template);

    REQUIRE_ELSE(mkdtemp(name) == name, {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to create temporary directory");
        KEFIR_FREE(mem, name);
        return res;
    });

    kefir_result_t res =
        kefir_hashtree_insert(mem, &mgr->tracked_files, (kefir_hashtree_key_t) name, (kefir_hashtree_value_t) 0);
    REQUIRE_ELSE(res == KEFIR_OK, {
        remove(name);
        free(name);
        return res;
    });

    *result = name;
    return KEFIR_OK;
}

kefir_result_t kefir_tempfile_manager_tmpdir(struct kefir_mem *mem, struct kefir_tempfile_manager *mgr,
                                             const char *prefix, const char **result) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile manager"));
    REQUIRE(prefix != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile prefix"));
    REQUIRE(result != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to generated tempfile name"));

    const char *tmp_directory = getenv("TMPDIR");
    if (tmp_directory == NULL) {
#ifdef P_tmpdir
        tmp_directory = P_tmpdir;
#else
        tmp_directory = "/tmp";
#endif
    }

    char template[PATH_MAX + 1];
    strcpy(template, tmp_directory);
    strcat(template, "/");
    strcat(template, prefix);

    REQUIRE_OK(kefir_tempfile_manager_create_directory(mem, mgr, template, result));
    return KEFIR_OK;
}
