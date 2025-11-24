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

#ifndef KEFIR_CODEGEN_TARGET_AMD64_RT_DESTRUCTOR_H_
#define KEFIR_CODEGEN_TARGET_AMD64_RT_DESTRUCTOR_H_

#include "kefir/codegen/target-ir/rt_destructor.h"
#include "kefir/codegen/amd64/function.h"

typedef struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops {
    struct kefir_codegen_target_ir_round_trip_destructor_ops ops;
    const struct kefir_codegen_amd64_function *function;
    struct kefir_asmcmp_amd64 *code;
    struct kefir_hashtree constants;
} kefir_codegen_target_ir_round_trip_destructor_amd64_ops_t;

kefir_result_t kefir_codegen_target_ir_round_trip_destructor_amd64_ops_init(const struct kefir_codegen_amd64_function *, struct kefir_asmcmp_amd64 *, struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *);
kefir_result_t kefir_codegen_target_ir_round_trip_destructor_amd64_ops_free(struct kefir_mem *, struct kefir_codegen_target_ir_round_trip_destructor_amd64_ops *);

#endif
