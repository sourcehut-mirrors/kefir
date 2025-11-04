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

#ifndef KEFIR_CODEGEN_TARGET_IR_CONSTRUCTOR_H_
#define KEFIR_CODEGEN_TARGET_IR_CONSTRUCTOR_H_

#include "kefir/codegen/target-ir/code.h"
#include "kefir/codegen/asmcmp/context.h"

typedef struct kefir_codegen_target_ir_code_constructor_class {
    kefir_result_t (*map_opcode)(kefir_asmcmp_instruction_opcode_t, kefir_codegen_target_ir_opcode_t *, void *);
    kefir_result_t (*is_jump)(kefir_asmcmp_instruction_opcode_t, kefir_bool_t *, void *);
    void *payload;
} kefir_codegen_target_ir_code_constructor_class_t;

kefir_result_t kefir_codegen_target_ir_code_construct(struct kefir_mem *, struct kefir_codegen_target_ir_code *, const struct kefir_asmcmp_context *, const struct kefir_codegen_target_ir_code_constructor_class *);

#endif
