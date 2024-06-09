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

#include "kefir/core/platform.h"
#if defined(KEFIR_LINUX_HOST_PLATFORM) || defined(KEFIR_EMSCRIPTEN_HOST_PLATFORM)
#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700
#endif

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

    mgr->file_index = 0;
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

static kefir_result_t init_tempfile_manager(struct kefir_mem *mem, struct kefir_tempfile_manager *mgr) {
    REQUIRE(mgr->basedir == NULL, KEFIR_OK);
    static const char TEMPLATE[] = "%s/kefir-%ld-XXXXXX";

    const char *tmp_directory = getenv("KEFIR_TMPDIR");
    if (tmp_directory == NULL) {
        tmp_directory = getenv("TMPDIR");
    }
    if (tmp_directory == NULL) {
#ifdef P_tmpdir
        tmp_directory = P_tmpdir;
#else
        tmp_directory = "/tmp";
#endif
    }

    const long pid = (long) getpid();
    const int basedir_length = snprintf(NULL, 0, TEMPLATE, tmp_directory, pid);
    REQUIRE(basedir_length > 0, KEFIR_SET_ERROR(KEFIR_UNKNOWN_ERROR, "Failed to initialize temporary file manager base directory name"));

    char *basedir = KEFIR_MALLOC(mem, basedir_length + 2);
    REQUIRE(basedir != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate temporary file manager base directory name"));
    REQUIRE_ELSE(snprintf(basedir, basedir_length + 1, TEMPLATE, tmp_directory, pid) == basedir_length, {
        KEFIR_FREE(mem, basedir);
        return KEFIR_SET_ERROR(KEFIR_UNKNOWN_ERROR, "Failed to initialize temporary file manager base directory name");
    });

    mgr->basedir = mkdtemp(basedir);
    REQUIRE_ELSE(mgr->basedir != NULL, {
        KEFIR_FREE(mem, basedir);
        return KEFIR_SET_OS_ERROR("Failed to create temporary file manager base directory");
    });

    kefir_result_t res = kefir_hashtree_insert(mem, &mgr->tracked_files, (kefir_hashtree_key_t) basedir, (kefir_hashtree_value_t) 0);
    REQUIRE_ELSE(res == KEFIR_OK, {
        remove(mgr->basedir);
        KEFIR_FREE(mem, basedir);
        mgr->basedir = NULL;
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_tempfile_manager_create_file(struct kefir_mem *mem, struct kefir_tempfile_manager *mgr,
                                                  const char *filename, const char **result) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(mgr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile manager"));
    REQUIRE(filename != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tempfile template"));
    REQUIRE(result != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to generated tempfile name"));

    REQUIRE_OK(init_tempfile_manager(mem, mgr));
    static const char FILEPATH_TEMPLATE[] = "%s/%s.%" KEFIR_SIZE_FMT ".XXXXXX";

    const kefir_size_t file_index = mgr->file_index++;
    const int filepath_length = snprintf(NULL, 0, FILEPATH_TEMPLATE, mgr->basedir, filename, file_index);
    REQUIRE(filepath_length > 0, KEFIR_SET_ERROR(KEFIR_UNKNOWN_ERROR, "Failed to prepare temporary file name template"));
    char * const filepath = KEFIR_MALLOC(mem, filepath_length + 2);
    REQUIRE(filepath != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate tempfile name"));

    REQUIRE_ELSE(snprintf(filepath, filepath_length + 1, FILEPATH_TEMPLATE, mgr->basedir, filename, file_index) == filepath_length, {
        KEFIR_FREE(mem, filepath);
        return KEFIR_SET_ERROR(KEFIR_UNKNOWN_ERROR, "Failed to prepare temporary file name template");
    });

#ifndef KEFIR_NETBSD_HOST_PLATFORM
    const int fd = mkstemp(filepath);
    REQUIRE_ELSE(fd >= 0, {
        const kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to generate temporary file name");
        KEFIR_FREE(mem, filepath);
        return res;
    });
    REQUIRE_ELSE(close(fd) == 0, {
        const kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to clone temporary file descriptor");
        remove(filepath);
        KEFIR_FREE(mem, filepath);
        return res;
    });
#else
    char *filepath_res = mktemp(filepath);
    REQUIRE_ELSE(filepath_res != NULL, {
        const kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to generate temporary file name");
        KEFIR_FREE(mem, filepath);
        return res;
    });
#endif

    const kefir_result_t res =
        kefir_hashtree_insert(mem, &mgr->tracked_files, (kefir_hashtree_key_t) filepath, (kefir_hashtree_value_t) 0);
    REQUIRE_ELSE(res == KEFIR_OK, {
        remove(filepath);
        free(filepath);
        return res;
    });

    *result = filepath;
    return KEFIR_OK;
}
