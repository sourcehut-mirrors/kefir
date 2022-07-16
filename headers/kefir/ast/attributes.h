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

#ifndef KEFIR_AST_ATTRIBUTES_H_
#define KEFIR_AST_ATTRIBUTES_H_

#include "kefir/core/list.h"

typedef struct kefir_ast_attribute_list kefir_ast_attribute_list_t;  // Forward declaration

typedef struct kefir_ast_node_attributes {
    struct kefir_list attributes;
} kefir_ast_node_attributes_t;

kefir_result_t kefir_ast_node_attributes_init(struct kefir_ast_node_attributes *);
kefir_result_t kefir_ast_node_attributes_free(struct kefir_mem *, struct kefir_ast_node_attributes *);
kefir_result_t kefir_ast_node_attributes_append(struct kefir_mem *, struct kefir_ast_node_attributes *,
                                                struct kefir_ast_attribute_list *);
kefir_result_t kefir_ast_node_attributes_move(struct kefir_ast_node_attributes *, struct kefir_ast_node_attributes *);
kefir_result_t kefir_ast_node_attributes_clone(struct kefir_mem *, struct kefir_ast_node_attributes *,
                                               const struct kefir_ast_node_attributes *);

#endif
