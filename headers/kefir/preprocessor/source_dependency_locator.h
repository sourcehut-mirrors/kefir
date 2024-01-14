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

#ifndef KEFIR_PREPROCESSOR_DEPENDENCY_LOCATOR_H_
#define KEFIR_PREPROCESSOR_DEPENDENCY_LOCATOR_H_

#include "kefir/preprocessor/source_file.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/hashtreeset.h"
#include <stdio.h>

typedef struct kefir_preprocessor_dependencies_source_locator {
    struct kefir_preprocessor_source_locator locator;
    struct kefir_preprocessor_source_locator *base_locator;
    kefir_result_t (*is_system_include_dir)(const char *, kefir_bool_t *, void *);
    void *is_system_include_dir_payload;
    struct kefir_hashtree includes;
    struct kefir_hashtreeset direct_system_includes;
} kefir_preprocessor_dependencies_source_locator_t;

kefir_result_t kefir_preprocessor_dependencies_source_locator_init(
    struct kefir_preprocessor_source_locator *, kefir_result_t (*)(const char *, kefir_bool_t *, void *), void *,
    struct kefir_preprocessor_dependencies_source_locator *);

kefir_result_t kefir_preprocessor_dependencies_source_locator_free(
    struct kefir_mem *, struct kefir_preprocessor_dependencies_source_locator *);

typedef struct kefir_preprocessor_dependencies_source_locator_iterator {
    const struct kefir_preprocessor_dependencies_source_locator *locator;
    struct kefir_hashtree_node_iterator iter;

    const char *filepath;
    kefir_bool_t system_dependency;
} kefir_preprocessor_dependencies_source_locator_iterator_t;

kefir_result_t kefir_preprocessor_dependencies_source_locator_iter(
    const struct kefir_preprocessor_dependencies_source_locator *,
    struct kefir_preprocessor_dependencies_source_locator_iterator *);
kefir_result_t kefir_preprocessor_dependencies_source_locator_next(
    struct kefir_preprocessor_dependencies_source_locator_iterator *);

kefir_result_t kefir_preprocessor_dependencies_source_locator_format_make_rule_prerequisites(
    const struct kefir_preprocessor_dependencies_source_locator *, kefir_bool_t, FILE *);

#endif
