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

#include "kefir/core/platform.h"
#if defined(KEFIR_LINUX_HOST_PLATFORM) || defined(KEFIR_EMSCRIPTEN_HOST_PLATFORM)
#define _XOPEN_SOURCE 500
#define _POSIX_SOURCE
#endif

#include "kefir/platform/filesystem_source.h"
#include "kefir/platform/input.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/os_error.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct include_directory_descriptor {
    char *path;
    kefir_bool_t quote_only;
};

static kefir_result_t close_source(struct kefir_mem *mem, struct kefir_preprocessor_source_file *source_file) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source file"));

    struct kefir_cli_input *input = source_file->payload;
    REQUIRE_OK(kefir_cli_input_close(mem, input));
    KEFIR_FREE(mem, input);
    *source_file = (struct kefir_preprocessor_source_file) {0};
    return KEFIR_OK;
}

static kefir_result_t open_file(struct kefir_mem *mem, const char *root, const char *filepath, kefir_bool_t system,
                                struct kefir_preprocessor_source_file *source_file, struct kefir_string_pool *symbols) {
    filepath = kefir_string_pool_insert(mem, symbols, filepath, NULL);
    REQUIRE(filepath != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert file path into symbol table"));

    REQUIRE(access(filepath, R_OK) == 0, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested file"));

    struct kefir_cli_input *input = KEFIR_MALLOC(mem, sizeof(struct kefir_cli_input));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate file reader"));
    kefir_result_t res = kefir_cli_input_open(mem, input, filepath, NULL);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, input);
        return res;
    });

    const char *content = NULL;
    kefir_size_t length;
    res = kefir_cli_input_get(input, &content, &length);
    REQUIRE_CHAIN(&res, kefir_lexer_source_cursor_init(&source_file->cursor, content, length, filepath));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_cli_input_close(mem, input);
        KEFIR_FREE(mem, input);
        return res;
    });

    source_file->info.filepath = filepath;
    source_file->info.system = system;
    source_file->info.base_include_dir = root;
    source_file->close = close_source;
    source_file->payload = input;
    return KEFIR_OK;
}

static kefir_result_t try_open_file(struct kefir_mem *mem, const char *root, const char *filepath, kefir_bool_t system,
                                    struct kefir_preprocessor_source_file *source_file,
                                    struct kefir_preprocessor_filesystem_source_locator *locator) {
    kefir_size_t path_length = root != NULL ? strlen(root) + strlen(filepath) + 2 : strlen(filepath) + 1;

    char *path = KEFIR_MALLOC(mem, path_length);
    REQUIRE(path != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate filesystem path"));
    if (root != NULL) {
        strcpy(path, root);
        strcat(path, "/");
        strcat(path, filepath);
    } else {
        strcpy(path, filepath);
    }
    char *resolved_path = realpath(path, NULL);
    KEFIR_FREE(mem, path);
    if (resolved_path == NULL && (errno == ENOENT || errno == ENOTDIR)) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested file");
    } else if (resolved_path == NULL) {
        return KEFIR_SET_OS_ERROR("Failed to determine real path");
    }
    kefir_result_t res = open_file(mem, root, resolved_path, system, source_file, locator->symbols);
    KEFIR_FREE(mem, resolved_path);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        return KEFIR_OK;
    } else {
        return res;
    }
}

static kefir_result_t open_source(struct kefir_mem *mem, const struct kefir_preprocessor_source_locator *source_locator,
                                  const char *filepath, kefir_bool_t system,
                                  const struct kefir_preprocessor_source_file_info *current_file,
                                  kefir_preprocessor_source_locator_mode_t mode,
                                  struct kefir_preprocessor_source_file *source_file) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid filesystem source locator"));
    REQUIRE(filepath != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file path"));
    REQUIRE(source_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to source file"));
    ASSIGN_DECL_CAST(struct kefir_preprocessor_filesystem_source_locator *, locator, source_locator);

    if (current_file != NULL && current_file->filepath && !system &&
        mode == KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NORMAL) {
        char *current_clone = KEFIR_MALLOC(mem, strlen(current_file->filepath) + 1);
        REQUIRE(current_clone != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate directory name"));
        strcpy(current_clone, current_file->filepath);
        char *directory = dirname(current_clone);
        REQUIRE_ELSE(directory != NULL, {
            KEFIR_FREE(mem, current_clone);
            return KEFIR_SET_OS_ERROR("Failed to obtain dirname");
        });

        const char *directory_copy = kefir_string_pool_insert(mem, locator->symbols, directory, NULL);
        REQUIRE_ELSE(directory_copy != NULL, {
            KEFIR_FREE(mem, current_clone);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert directory path into symbol table");
        });
        KEFIR_FREE(mem, current_clone);

        kefir_result_t res = try_open_file(mem, directory_copy, filepath, system, source_file, locator);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            return KEFIR_OK;
        }
    }

    const struct kefir_list_entry *iter = kefir_list_head(&locator->include_roots);
    if (mode == KEFIR_PREPROCESSOR_SOURCE_LOCATOR_MODE_NEXT && current_file != NULL &&
        current_file->base_include_dir != NULL) {
        for (; iter != NULL; kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct include_directory_descriptor *, root, iter->value);
            if (strcmp(current_file->base_include_dir, root->path) == 0) {
                break;
            }
        }

        if (iter == NULL) {
            iter = kefir_list_head(&locator->include_roots);
        } else {
            kefir_list_next(&iter);
        }
    }
    for (; iter != NULL; kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct include_directory_descriptor *, root, iter->value);
        if (system && root->quote_only) {
            continue;
        }
        kefir_result_t res = try_open_file(mem, root->path, filepath, system, source_file, locator);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            return KEFIR_OK;
        }
    }

    kefir_result_t res = try_open_file(mem, NULL, filepath, system, source_file, locator);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        return KEFIR_OK;
    }

    return KEFIR_SET_ERRORF(KEFIR_NOT_FOUND, "Unable to find requested include file %s", filepath);
}

struct embed_file_data {
    char *data;
    kefir_size_t length;
    kefir_size_t index;
};

static kefir_result_t embed_read_next(struct kefir_mem *mem, struct kefir_preprocessor_embed_file *embed_file,
                                      kefir_uint8_t *value_ptr, kefir_bool_t *end_of_file_ptr) {
    UNUSED(mem);
    REQUIRE(embed_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid embed file"));

    ASSIGN_DECL_CAST(struct embed_file_data *, file_data, embed_file->payload);
    REQUIRE(file_data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Embed file reader has already been closed"));

    if (file_data->index == file_data->length) {
        ASSIGN_PTR(end_of_file_ptr, true);
    } else {
        ASSIGN_PTR(value_ptr, file_data->data[file_data->index++]);
        ASSIGN_PTR(end_of_file_ptr, false);
    }
    return KEFIR_OK;
}

static kefir_result_t embed_close(struct kefir_mem *mem, struct kefir_preprocessor_embed_file *embed_file) {
    UNUSED(mem);
    REQUIRE(embed_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid embed file"));

    ASSIGN_DECL_CAST(struct embed_file_data *, file_data, embed_file->payload);
    REQUIRE(file_data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Embed file reader has already been closed"));

    if (file_data->length > 0) {
        int err = munmap(file_data->data, file_data->length);
        REQUIRE(err == 0, KEFIR_SET_OS_ERROR("Failed to unmap file"));
    }

    KEFIR_FREE(mem, file_data);
    embed_file->payload = NULL;
    return KEFIR_OK;
}

static kefir_result_t open_embed_file(struct kefir_mem *mem, const char *root, const char *filepath,
                                      kefir_bool_t system, struct kefir_preprocessor_embed_file *embed_file,
                                      struct kefir_string_pool *symbols) {
    filepath = kefir_string_pool_insert(mem, symbols, filepath, NULL);
    REQUIRE(filepath != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert file path into symbol table"));

    REQUIRE(access(filepath, R_OK) == 0, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested file"));
    int fd = open(filepath, O_RDONLY);
    REQUIRE(fd != -1, KEFIR_SET_OS_ERRORF("Failed to open file \"%s\"", filepath));

    struct stat statbuf;
    REQUIRE(fstat(fd, &statbuf) >= 0, KEFIR_SET_OS_ERROR("Failed to fstat file"));

    char *data = NULL;
    if (statbuf.st_size > 0) {
        data = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
        REQUIRE(data != MAP_FAILED, KEFIR_SET_OS_ERROR("Failed to mmap file"));
        close(fd);
    }

    struct embed_file_data *file_data = KEFIR_MALLOC(mem, sizeof(struct embed_file_data));
    REQUIRE_ELSE(file_data != NULL, {
        munmap(data, statbuf.st_size);
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate embed file data");
    });

    file_data->data = data;
    file_data->length = statbuf.st_size;
    file_data->index = 0;

    embed_file->info.filepath = filepath;
    embed_file->info.system = system;
    embed_file->info.base_include_dir = root;
    embed_file->read_next = embed_read_next;
    embed_file->close = embed_close;
    embed_file->payload = file_data;
    return KEFIR_OK;
}

static kefir_result_t try_open_embed(struct kefir_mem *mem, const char *root, const char *filepath, kefir_bool_t system,
                                     struct kefir_preprocessor_embed_file *embed_file,
                                     struct kefir_preprocessor_filesystem_source_locator *locator) {
    kefir_size_t path_length = root != NULL ? strlen(root) + strlen(filepath) + 2 : strlen(filepath) + 1;

    char *path = KEFIR_MALLOC(mem, path_length);
    REQUIRE(path != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate filesystem path"));
    if (root != NULL) {
        strcpy(path, root);
        strcat(path, "/");
        strcat(path, filepath);
    } else {
        strcpy(path, filepath);
    }
    char *resolved_path = realpath(path, NULL);
    KEFIR_FREE(mem, path);
    if (resolved_path == NULL && (errno == ENOENT || errno == ENOTDIR)) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested file");
    } else if (resolved_path == NULL) {
        return KEFIR_SET_OS_ERROR("Failed to determine real path");
    }
    kefir_result_t res = open_embed_file(mem, root, resolved_path, system, embed_file, locator->symbols);
    KEFIR_FREE(mem, resolved_path);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        return KEFIR_OK;
    } else {
        return res;
    }
}

static kefir_result_t open_embed(struct kefir_mem *mem, const struct kefir_preprocessor_source_locator *source_locator,
                                 const char *filepath, kefir_bool_t system,
                                 const struct kefir_preprocessor_source_file_info *current_file,
                                 struct kefir_preprocessor_embed_file *embed_file) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid filesystem source locator"));
    REQUIRE(filepath != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file path"));
    REQUIRE(embed_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to embed file"));
    ASSIGN_DECL_CAST(struct kefir_preprocessor_filesystem_source_locator *, locator, source_locator);

    if (current_file != NULL && current_file->filepath && !system) {
        char *current_clone = KEFIR_MALLOC(mem, strlen(current_file->filepath) + 1);
        REQUIRE(current_clone != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate directory name"));
        strcpy(current_clone, current_file->filepath);
        char *directory = dirname(current_clone);
        REQUIRE_ELSE(directory != NULL, {
            KEFIR_FREE(mem, current_clone);
            return KEFIR_SET_OS_ERROR("Failed to obtain dirname");
        });

        const char *directory_copy = kefir_string_pool_insert(mem, locator->symbols, directory, NULL);
        REQUIRE_ELSE(directory_copy != NULL, {
            KEFIR_FREE(mem, current_clone);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert directory path into symbol table");
        });
        KEFIR_FREE(mem, current_clone);

        kefir_result_t res = try_open_embed(mem, directory_copy, filepath, system, embed_file, locator);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            return KEFIR_OK;
        }
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&locator->embed_roots); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct include_directory_descriptor *, root, iter->value);
        kefir_result_t res = try_open_embed(mem, root->path, filepath, system, embed_file, locator);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            return KEFIR_OK;
        }
    }

    kefir_result_t res = try_open_embed(mem, NULL, filepath, system, embed_file, locator);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        return KEFIR_OK;
    }

    return KEFIR_SET_ERRORF(KEFIR_NOT_FOUND, "Unable to find requested embed file %s", filepath);
}

static kefir_result_t free_include_directory_descr(struct kefir_mem *mem, struct kefir_list *list,
                                                   struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct include_directory_descriptor *, descr, entry->value);
    KEFIR_FREE(mem, descr->path);
    KEFIR_FREE(mem, descr);
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_filesystem_source_locator_init(
    struct kefir_preprocessor_filesystem_source_locator *locator, struct kefir_string_pool *symbols) {
    REQUIRE(locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to filesystem source locator"));
    REQUIRE(symbols != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbol table"));

    REQUIRE_OK(kefir_list_init(&locator->include_roots));
    REQUIRE_OK(kefir_list_init(&locator->embed_roots));
    REQUIRE_OK(kefir_list_on_remove(&locator->include_roots, free_include_directory_descr, NULL));
    REQUIRE_OK(kefir_list_on_remove(&locator->embed_roots, free_include_directory_descr, NULL));
    REQUIRE_OK(kefir_hashtreeset_init(&locator->include_root_set, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&locator->embed_root_set, &kefir_hashtree_str_ops));
    locator->symbols = symbols;
    locator->locator.payload = &locator;
    locator->locator.open = open_source;
    locator->locator.open_embed = open_embed;
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_filesystem_source_locator_free(
    struct kefir_mem *mem, struct kefir_preprocessor_filesystem_source_locator *locator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to filesystem source locator"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &locator->include_root_set));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &locator->embed_root_set));
    REQUIRE_OK(kefir_list_free(mem, &locator->include_roots));
    REQUIRE_OK(kefir_list_free(mem, &locator->embed_roots));
    locator->locator.payload = NULL;
    locator->locator.open = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_filesystem_source_locator_append(
    struct kefir_mem *mem, struct kefir_preprocessor_filesystem_source_locator *locator, const char *path,
    kefir_bool_t quote_only) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to filesystem source locator"));
    REQUIRE(path != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid filesystem path"));
    REQUIRE(!kefir_hashtreeset_has(&locator->include_root_set, (kefir_hashtreeset_entry_t) path), KEFIR_OK);

    char *copy = KEFIR_MALLOC(mem, strlen(path) + 1);
    REQUIRE(copy != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate filesystem path"));
    struct include_directory_descriptor *descr = KEFIR_MALLOC(mem, sizeof(struct include_directory_descriptor));
    REQUIRE_ELSE(descr != NULL, {
        KEFIR_FREE(mem, copy);
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate include directory descriptor");
    });
    descr->path = copy;
    descr->quote_only = quote_only;
    strcpy(copy, path);
    kefir_result_t res =
        kefir_list_insert_after(mem, &locator->include_roots, kefir_list_tail(&locator->include_roots), (void *) descr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, copy);
        return res;
    });

    REQUIRE_OK(kefir_hashtreeset_add(mem, &locator->include_root_set, (kefir_hashtreeset_entry_t) copy));
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_filesystem_source_locator_append_embed(
    struct kefir_mem *mem, struct kefir_preprocessor_filesystem_source_locator *locator, const char *path) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to filesystem source locator"));
    REQUIRE(path != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid filesystem path"));
    REQUIRE(!kefir_hashtreeset_has(&locator->embed_root_set, (kefir_hashtreeset_entry_t) path), KEFIR_OK);

    char *copy = KEFIR_MALLOC(mem, strlen(path) + 1);
    REQUIRE(copy != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate filesystem path"));
    struct include_directory_descriptor *descr = KEFIR_MALLOC(mem, sizeof(struct include_directory_descriptor));
    REQUIRE_ELSE(descr != NULL, {
        KEFIR_FREE(mem, copy);
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate include directory descriptor");
    });
    descr->path = copy;
    descr->quote_only = false;
    strcpy(copy, path);
    kefir_result_t res =
        kefir_list_insert_after(mem, &locator->embed_roots, kefir_list_tail(&locator->embed_roots), (void *) descr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, copy);
        return res;
    });

    REQUIRE_OK(kefir_hashtreeset_add(mem, &locator->embed_root_set, (kefir_hashtreeset_entry_t) copy));
    return KEFIR_OK;
}
