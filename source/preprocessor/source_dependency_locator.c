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

#include "kefir/preprocessor/source_dependency_locator.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t dependencies_source_locator_open(struct kefir_mem *mem,
                                                       const struct kefir_preprocessor_source_locator *source_locator,
                                                       const char *filepath, kefir_bool_t system,
                                                       const struct kefir_preprocessor_source_file_info *file_info,
                                                       kefir_preprocessor_source_locator_mode_t mode,
                                                       struct kefir_preprocessor_source_file *source_file) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(source_locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid filesystem source locator"));
    REQUIRE(filepath != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file path"));
    REQUIRE(source_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to source file"));
    ASSIGN_DECL_CAST(struct kefir_preprocessor_dependencies_source_locator *, locator, source_locator);

    REQUIRE_OK(locator->base_locator->open(mem, locator->base_locator, filepath, system, file_info, mode, source_file));
    if (!kefir_hashtree_has(&locator->includes, (kefir_hashtree_key_t) source_file->info.filepath)) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &locator->includes, (kefir_hashtree_key_t) source_file->info.filepath,
                                         (kefir_hashtree_value_t) 0));
    }

    kefir_bool_t is_system_include_dir;
    REQUIRE_OK(locator->is_system_include_dir(source_file->info.base_include_dir, &is_system_include_dir,
                                              locator->is_system_include_dir_payload));
    if (is_system_include_dir) {
        REQUIRE_OK(kefir_hashtreeset_add(mem, &locator->direct_system_includes,
                                         (kefir_hashtreeset_entry_t) source_file->info.filepath));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_dependencies_source_locator_init(
    struct kefir_preprocessor_source_locator *base_locator,
    kefir_result_t (*is_system_include_dir)(const char *, kefir_bool_t *, void *), void *is_system_include_dir_payload,
    struct kefir_preprocessor_dependencies_source_locator *locator) {
    REQUIRE(base_locator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid base source locator"));
    REQUIRE(is_system_include_dir != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid system include directory detection function"));
    REQUIRE(locator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to dependency source locator"));

    locator->base_locator = base_locator;
    locator->is_system_include_dir = is_system_include_dir;
    locator->is_system_include_dir_payload = is_system_include_dir_payload;
    locator->locator.open = dependencies_source_locator_open;
    locator->locator.payload = locator;
    REQUIRE_OK(kefir_hashtree_init(&locator->includes, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&locator->direct_system_includes, &kefir_hashtree_str_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_dependencies_source_locator_free(
    struct kefir_mem *mem, struct kefir_preprocessor_dependencies_source_locator *locator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(locator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid dependency source locator"));

    REQUIRE_OK(kefir_hashtree_free(mem, &locator->includes));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &locator->direct_system_includes));
    memset(locator, 0, sizeof(struct kefir_preprocessor_dependencies_source_locator));
    return KEFIR_OK;
}

static kefir_result_t is_system_dependency(const struct kefir_preprocessor_dependencies_source_locator *locator,
                                           const char *filepath, kefir_bool_t *system_dep) {
    while (filepath != NULL) {
        if (kefir_hashtreeset_has(&locator->direct_system_includes, (kefir_hashtreeset_entry_t) filepath)) {
            *system_dep = true;
            return KEFIR_OK;
        }

        struct kefir_hashtree_node *node = NULL;
        kefir_result_t res = kefir_hashtree_at(&locator->includes, (kefir_hashtree_key_t) filepath, &node);
        if (res == KEFIR_NOT_FOUND) {
            filepath = NULL;
        } else {
            REQUIRE_OK(res);
            filepath = (const char *) node->value;
        }
    }

    *system_dep = false;
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_dependencies_source_locator_iter(
    const struct kefir_preprocessor_dependencies_source_locator *locator,
    struct kefir_preprocessor_dependencies_source_locator_iterator *iter) {
    REQUIRE(locator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid dependency source locator"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to dependency source locator iterator"));

    iter->locator = locator;
    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&locator->includes, &iter->iter);
    if (node != NULL) {
        ASSIGN_DECL_CAST(const char *, filepath, node->key);

        iter->filepath = filepath;
        REQUIRE_OK(is_system_dependency(locator, filepath, &iter->system_dependency));
        return KEFIR_OK;
    } else {
        return KEFIR_ITERATOR_END;
    }
}

kefir_result_t kefir_preprocessor_dependencies_source_locator_next(
    struct kefir_preprocessor_dependencies_source_locator_iterator *iter) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid dependency source locator iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    if (node != NULL) {
        ASSIGN_DECL_CAST(const char *, filepath, node->key);

        iter->filepath = filepath;
        REQUIRE_OK(is_system_dependency(iter->locator, filepath, &iter->system_dependency));
        return KEFIR_OK;
    } else {
        return KEFIR_ITERATOR_END;
    }
}

kefir_result_t kefir_preprocessor_dependencies_source_locator_format_make_rule_prerequisites(
    const struct kefir_preprocessor_dependencies_source_locator *locator, kefir_bool_t include_system_paths,
    FILE *output) {
    REQUIRE(locator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid dependency source locator"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid FILE"));

    kefir_result_t res;
    struct kefir_preprocessor_dependencies_source_locator_iterator iter;
    for (res = kefir_preprocessor_dependencies_source_locator_iter(locator, &iter); res == KEFIR_OK;
         res = kefir_preprocessor_dependencies_source_locator_next(&iter)) {
        if (include_system_paths || !iter.system_dependency) {
            fprintf(output, " \\\n %s", iter.filepath);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
