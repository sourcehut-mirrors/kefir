/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#ifndef KEFIR_CORE_SOURCE_LOCATION_INDEX_H_
#define KEFIR_CORE_SOURCE_LOCATION_INDEX_H_

#include "kefir/core/memory_arena.h"
#include "kefir/core/source_location.h"
#include "kefir/core/hashtable.h"

typedef struct kefir_parser_source_location_index {
    struct kefir_hashtable locations;
} kefir_parser_source_location_index_t;

kefir_result_t kefir_parser_source_location_index_init(struct kefir_parser_source_location_index *);
kefir_result_t kefir_parser_source_location_index_free(struct kefir_mem *, struct kefir_parser_source_location_index *);
kefir_result_t kefir_parser_source_location_index_reset(struct kefir_mem *, struct kefir_parser_source_location_index *);

kefir_result_t kefir_parser_source_location_index_find(struct kefir_mem *, struct kefir_parser_source_location_index *, const struct kefir_source_location *, const struct kefir_source_location **);

#endif
