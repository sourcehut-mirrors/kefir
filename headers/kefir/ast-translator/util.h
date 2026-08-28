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

#ifndef KEFIR_AST_TRANSLATOR_UTIL_H_
#define KEFIR_AST_TRANSLATOR_UTIL_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/memory_arena.h"
#include "kefir/ast/type.h"
#include "kefir/ast-translator/scope/scoped_identifier.h"

const struct kefir_ast_type *kefir_ast_translator_normalize_type(const struct kefir_ast_type *);
kefir_result_t kefir_ast_translator_scoped_identifier_allocate_object(struct kefir_memory_arena *, const struct kefir_ast_scoped_identifier *, struct kefir_ast_translator_scoped_identifier_object **);
kefir_result_t kefir_ast_translator_scoped_identifier_allocate_function(struct kefir_memory_arena *, const struct kefir_ast_scoped_identifier *, struct kefir_ast_translator_scoped_identifier_function **);

#endif
