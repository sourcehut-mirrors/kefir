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

#ifndef KEFIR_PREPROCESSOR_UTIL_H_
#define KEFIR_PREPROCESSOR_UTIL_H_

#include "kefir/lexer/lexem.h"
#include "kefir/core/mem.h"

kefir_result_t kefir_token_new_string_literal_raw_from_escaped_multibyte(struct kefir_mem *,
                                                                         kefir_string_literal_token_type_t,
                                                                         const char *, kefir_size_t,
                                                                         struct kefir_token *);

#endif
