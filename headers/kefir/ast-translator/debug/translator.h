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

#ifndef KEFIR_AST_TRANSLATOR_DEBUG_TYPE_H_
#define KEFIR_AST_TRANSLATOR_DEBUG_TYPE_H_

#include "kefir/ast/type_layout.h"
#include "kefir/ast-translator/environment.h"
#include "kefir/ir/debug.h"
#include "kefir/ir/module.h"

typedef struct kefir_ast_translator_debug_entries {
    struct kefir_hashtree type_index;
} kefir_ast_translator_debug_entries_t;

kefir_result_t kefir_ast_translator_debug_entries_init(struct kefir_ast_translator_debug_entries *);
kefir_result_t kefir_ast_translator_debug_entries_free(struct kefir_mem *, struct kefir_ast_translator_debug_entries *);

kefir_result_t kefir_ast_translate_debug_type(struct kefir_mem *, const struct kefir_ast_context *, const struct kefir_ast_translator_environment *, struct kefir_ir_module *, struct kefir_ast_translator_debug_entries *, const struct kefir_ast_type *, kefir_ir_debug_entry_id_t *);

#endif
