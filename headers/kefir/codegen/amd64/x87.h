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

#ifndef KEFIR_CODEGEN_AMD64_X87_H_
#define KEFIR_CODEGEN_AMD64_X87_H_

#include "kefir/core/list.h"
#include "kefir/optimizer/code.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/codegen/function.h"

typedef struct kefir_codegen_amd64_x87 {
    struct kefir_list x87_stack;
} kefir_codegen_amd64_x87_t;

typedef struct kefir_codegen_amd64_x87_iterator {
    const struct kefir_list_entry *iter;
} kefir_codegen_amd64_x87_iterator_t;

kefir_result_t kefir_codegen_amd64_x87_init(struct kefir_codegen_amd64_x87 *);
kefir_result_t kefir_codegen_amd64_x87_free(struct kefir_mem *, struct kefir_codegen_amd64_x87 *);

kefir_result_t kefir_codegen_amd64_x87_reset(struct kefir_mem *, struct kefir_codegen_amd64_x87 *);
kefir_size_t kefir_codegen_amd64_x87_length(const struct kefir_codegen_amd64_x87 *);

kefir_result_t kefir_codegen_amd64_x87_iter(const struct kefir_codegen_amd64_x87 *, struct kefir_codegen_amd64_x87_iterator *, kefir_opt_instruction_ref_t *);
kefir_result_t kefir_codegen_amd64_x87_next(struct kefir_codegen_amd64_x87_iterator *, kefir_opt_instruction_ref_t *);


kefir_result_t kefir_codegen_amd64_x87_ensure(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                       struct kefir_codegen_amd64_x87 *, struct kefir_codegen_amd64_stack_frame *, const struct kefir_codegen_function *,
                                                       kefir_size_t, kefir_bool_t);
kefir_result_t kefir_codegen_amd64_x87_push(struct kefir_mem *,
                                                     struct kefir_asmcmp_amd64 *, struct kefir_codegen_amd64_x87 *,
                                                     struct kefir_codegen_amd64_stack_frame *,
                                                     const struct kefir_codegen_function *,
                                                     kefir_opt_instruction_ref_t, kefir_bool_t);
kefir_result_t kefir_codegen_amd64_x87_pop(struct kefir_mem *, struct kefir_codegen_amd64_x87 *);

kefir_result_t kefir_codegen_amd64_x87_load(struct kefir_mem *,
                                                     struct kefir_asmcmp_amd64 *, struct kefir_codegen_amd64_x87 *,
                                                     struct kefir_codegen_amd64_stack_frame *,
                                                     const struct kefir_codegen_function *,
                                                     kefir_opt_instruction_ref_t, kefir_bool_t);
kefir_result_t kefir_codegen_amd64_x87_consume_by(struct kefir_mem *, const struct kefir_opt_code_container *,
                                                     struct kefir_asmcmp_amd64 *, struct kefir_codegen_amd64_x87 *,
                                                     struct kefir_codegen_amd64_stack_frame *,
                                                     const struct kefir_codegen_function *,
                                                     kefir_opt_instruction_ref_t,
                                                           kefir_opt_instruction_ref_t, kefir_abi_amd64_variant_t);
kefir_result_t kefir_codegen_amd64_x87_load_consume_by(struct kefir_mem *, const struct kefir_opt_code_container *,
                                                     struct kefir_asmcmp_amd64 *, struct kefir_codegen_amd64_x87 *,
                                                     struct kefir_codegen_amd64_stack_frame *,
                                                     const struct kefir_codegen_function *,
                                                     kefir_opt_instruction_ref_t,
                                                           kefir_opt_instruction_ref_t, kefir_bool_t);

kefir_result_t kefir_codegen_amd64_x87_flush(struct kefir_mem *,
                                                     struct kefir_asmcmp_amd64 *, struct kefir_codegen_amd64_x87 *,
                                                     struct kefir_codegen_amd64_stack_frame *,
                                                     const struct kefir_codegen_function *);
kefir_result_t kefir_codegen_amd64_x87_clear(struct kefir_mem *, struct kefir_asmcmp_amd64 *, struct kefir_codegen_amd64_x87 *, kefir_size_t);

#endif
