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

#ifndef KEFIR_CODEGEN_TARGET_IR_RT_DESTRUCTOR_H_
#define KEFIR_CODEGEN_TARGET_IR_RT_DESTRUCTOR_H_

#include "kefir/codegen/target-ir/constructor.h"
#include "kefir/codegen/asmcmp/context.h"

typedef struct kefir_codegen_target_ir_target_ir_operand_classification {
    kefir_codegen_target_ir_asmcmp_operand_class_t class;
    kefir_size_t index;
    kefir_bool_t implicit;
    kefir_bool_t immediate;

    struct {
        kefir_asmcmp_virtual_register_type_t vreg_type;
        kefir_codegen_target_ir_physical_register_t phreg;
    } implicit_params;
} kefir_codegen_target_ir_target_ir_operand_classification_t;

typedef struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification {
    kefir_asmcmp_instruction_opcode_t opcode;
    struct kefir_codegen_target_ir_target_ir_operand_classification operands[KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS];
} kefir_codegen_target_ir_target_ir_instruction_destructor_classification_t;

typedef struct kefir_codegen_target_ir_round_trip_destructor_ops {
    kefir_asmcmp_instruction_opcode_t link_virtual_registers_opcode;
    kefir_asmcmp_instruction_opcode_t touch_virtual_register_opcode;
    kefir_asmcmp_instruction_opcode_t virtual_block_begin_opcode;
    kefir_asmcmp_instruction_opcode_t virtual_block_end_opcode;
    kefir_asmcmp_instruction_opcode_t unreachable_opcode;
    kefir_asmcmp_instruction_opcode_t noop_opcode;
    kefir_asmcmp_instruction_opcode_t jump_opcode;
    kefir_result_t (*classify_instruction)(const struct kefir_codegen_target_ir_code *, kefir_codegen_target_ir_instruction_ref_t, struct kefir_codegen_target_ir_target_ir_instruction_destructor_classification *, void *);
    kefir_result_t (*bind_native_id)(struct kefir_mem *, kefir_asmcmp_label_index_t, kefir_codegen_target_ir_native_id_t, void *);
    kefir_result_t (*preallocation_requirement)(struct kefir_mem *, kefir_asmcmp_virtual_register_index_t, kefir_codegen_target_ir_physical_register_t, void *);
    kefir_result_t (*preallocation_hint)(struct kefir_mem *, kefir_asmcmp_virtual_register_index_t, kefir_codegen_target_ir_physical_register_t, void *);
    kefir_result_t (*split_branch_instruction)(struct kefir_mem *, const struct kefir_codegen_target_ir_instruction *, struct kefir_asmcmp_instruction[2], void *);
    kefir_result_t (*new_inline_asm)(struct kefir_mem *, kefir_asmcmp_instruction_index_t, kefir_asmcmp_inline_assembly_index_t, kefir_asmcmp_instruction_index_t *, void *);
    kefir_result_t (*materialize_attribute)(struct kefir_mem *, kefir_asmcmp_instruction_index_t, kefir_codegen_target_ir_native_id_t, kefir_asmcmp_instruction_index_t *, void *);
    kefir_result_t (*new_code_fragment)(struct kefir_mem *, kefir_codegen_target_ir_metadata_code_ref_t, kefir_asmcmp_label_index_t, kefir_asmcmp_label_index_t, void *);
    kefir_result_t (*new_value_fragment)(struct kefir_mem *, kefir_codegen_target_ir_metadata_value_ref_t, kefir_asmcmp_virtual_register_index_t, kefir_asmcmp_label_index_t, kefir_asmcmp_label_index_t, void *);
    void *payload;
} kefir_codegen_target_ir_round_trip_destructor_parameter_t;

kefir_result_t kefir_codegen_target_ir_round_trip_destruct(struct kefir_mem *, const struct kefir_codegen_target_ir_code *, struct kefir_asmcmp_context *, const struct kefir_codegen_target_ir_code_constructor_metadata *, const struct kefir_codegen_target_ir_round_trip_destructor_ops *);

#endif
