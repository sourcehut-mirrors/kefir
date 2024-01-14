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

#ifndef KEFIR_AST_TEMPORARIES_H_
#define KEFIR_AST_TEMPORARIES_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"
#include "kefir/ast/base.h"

typedef struct kefir_ast_scoped_identifier kefir_ast_scoped_identifier_t;  // Forward declaration

typedef struct kefir_ast_temporary_identifier {
    const char *identifier;
    const struct kefir_ast_scoped_identifier *scoped_id;
} kefir_ast_temporary_identifier_t;

#endif
