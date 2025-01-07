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

#ifndef KEFIR_PREPROCESSOR_TOKENIZER_H_
#define KEFIR_PREPROCESSOR_TOKENIZER_H_

#include "kefir/lexer/lexer.h"

typedef enum kefir_preprocessor_tokenizer_state {
    KEFIR_PREPROCESSOR_TOKENIZER_NORMAL,
    KEFIR_PREPROCESSOR_TOKENIZER_HAS_INCLUDE1,
    KEFIR_PREPROCESSOR_TOKENIZER_HAS_INCLUDE2
} kefir_preprocessor_tokenizer_state_t;

typedef struct kefir_preprocessor_tokenizer_context {
    kefir_preprocessor_tokenizer_state_t state;
} kefir_preprocessor_tokenizer_context_t;

kefir_result_t kefir_preprocessor_tokenizer_context_init(struct kefir_preprocessor_tokenizer_context *);
kefir_result_t kefir_preprocessor_tokenize_next(struct kefir_mem *, struct kefir_lexer *, struct kefir_preprocessor_tokenizer_context *, struct kefir_token *);

#endif
