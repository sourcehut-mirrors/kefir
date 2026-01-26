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

#ifndef KEFIR_CODEGEN_TARGET_IR_AMD64_CODE_H_
#define KEFIR_CODEGEN_TARGET_IR_AMD64_CODE_H_

#include "kefir/codegen/target-ir/code.h"
#include "kefir/codegen/amd64/asmcmp.h"

#define KEFIR_TARGET_IR_AMD64_OPCODE(_opcode) KEFIR_TARGET_IR_AMD64_##_opcode
#define KEFIR_CODEGEN_TARGET_IR_AMD64_VIRTUAL_OPCODES(_instr0, _separator)                                 \
    _instr0(preserve_active_virtual_registers, "preserve_active_virtual_registers") _separator _instr0(    \
        function_prologue, "function_prologue") _separator _instr0(function_epilogue, "function_epilogue") \
        _separator _instr0(tail_call, "tail_call") _separator _instr0(inline_assembly, "inline_assembly")  \
            _separator _instr0(data_word, "data_word")
typedef enum kefir_target_ir_amd64_opcode {
#define DEF_OPCODE(_opcode, ...) KEFIR_TARGET_IR_AMD64_OPCODE(_opcode)
    KEFIR_CODEGEN_TARGET_IR_AMD64_VIRTUAL_OPCODES(DEF_OPCODE, COMMA),
    KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE, DEF_OPCODE, DEF_OPCODE, DEF_OPCODE, COMMA),
    KEFIR_CODEGEN_TARGET_IR_SPECIAL_OPCODES(DEF_OPCODE, COMMA),
    KEFIR_TARGET_IR_AMD64_OPCODE(num_of_opcodes)
#undef DEF_OPCODE
} kefir_target_ir_amd64_opcode_t;

typedef enum kefir_target_ir_amd64_resource_id {
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_CF,
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_PF,
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_ZF,
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_SF,
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_DF,
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_FLAG_OF,
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_STACK,
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_X87_FPU_ENVIRONMENT,
    KEFIR_CODEGEN_TARGET_IR_AMD64_RESOURCE_MXCSR
} kefir_target_ir_amd64_resource_id_t;

extern const struct kefir_codegen_target_ir_code_class KEFIR_TARGET_AMD64_CODE_CLASS;

#endif
