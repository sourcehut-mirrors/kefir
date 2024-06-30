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

#ifndef KEFIR_AST_RUNTIME_H_
#define KEFIR_AST_RUNTIME_H_

#include "kefir/core/basic-types.h"

#define KEFIR_AST_TRANSLATOR_FUNCTION_STATIC_VARIABLE_IDENTIFIER "__kefir_function_%s_static_%s_%" KEFIR_ID_FMT
#define KEFIR_AST_TRANSLATOR_TEMPORARY_GLOBAL_IDENTIFIER "__kefirrt_temporary_%" KEFIR_ID_FMT
#define KEFIR_AST_TRANSLATOR_TEMPORARY_LOCAL_IDENTIFIER "__kefirrt_local_temporary_%" KEFIR_ID_FMT
#define KEFIR_AST_TRANSLATOR_VLA_ELEMENTS_IDENTIFIER "__kefirrt_vla_elements"
#define KEFIR_AST_TRANSLATOR_GNU_INLINE_FUNCTION_IDENTIFIER "__kefirrt_gnu_inline_%s"

#endif
