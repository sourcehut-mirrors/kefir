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

#ifndef KEFIR_CODEGEN_TARGET_IR_TIE_H_
#define KEFIR_CODEGEN_TARGET_IR_TIE_H_

#include "kefir/codegen/target-ir/code.h"

#define KEFIR_CODEGEN_TARGET_IR_TIED_READ_INDEX_NONE ((kefir_size_t) ~0ull)

typedef struct kefir_codegen_target_ir_tied_operand {
    kefir_size_t read_index;
    kefir_codegen_target_ir_value_ref_t output;
} kefir_codegen_target_ir_tied_operand_t;

typedef struct kefir_codegen_target_ir_tie_classification {
    struct kefir_codegen_target_ir_instruction_destruction_classification classification;
    struct kefir_codegen_target_ir_tied_operand operands[KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS];
    kefir_size_t untied_parameter_index;
    kefir_size_t untied_output_index;
} kefir_codegen_target_ir_tie_classification_t;

kefir_result_t kefir_codegen_target_ir_tie_operands(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t, struct kefir_codegen_target_ir_tie_classification *);

#endif
