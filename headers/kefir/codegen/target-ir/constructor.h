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

#define KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_EXTRA_NONE (0ull)
#define KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_EXTRA_CONSUMES_FLAGS (1ull)
#define KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_EXTRA_PRODUCES_FLAGS (1ull << 1)

typedef enum kefir_codegen_target_ir_asmcmp_operand_class {
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_NONE,
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ,
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_WRITE,
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_OPERAND_READ_WRITE
} kefir_codegen_target_ir_asmcmp_operand_class_t;

typedef struct kefir_codegen_target_ir_asmcmp_operand_classification {
    kefir_codegen_target_ir_asmcmp_operand_class_t class;
    kefir_size_t index;
    kefir_bool_t implicit;
    
    union {
        struct {
            kefir_codegen_target_ir_physical_register_t phreg;
            kefir_codegen_target_ir_operand_variant_t variant;
        } implicit_parameter;
    };
} kefir_codegen_target_ir_asmcmp_operand_classification_t;

typedef enum kefir_codegen_target_ir_asmcmp_special_instruction {
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_SPECIAL_NONE,
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_VIRTUAL_REGISTER_LINK,
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_VIRTUAL_REGISTER_PRODUCE,
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_VIRTUAL_REGISTER_TOUCH,
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_ATTRIBUTE,
    KEFIR_CODEGEN_TARGET_IR_ASMCMP_INSTRUCTION_SKIP
} kefir_codegen_target_ir_asmcmp_special_instruction_t;

typedef struct kefir_codegen_target_ir_asmcmp_instruction_classification {
    kefir_codegen_target_ir_opcode_t opcode;
    kefir_codegen_target_ir_asmcmp_special_instruction_t special;
    union {
        struct {
            struct kefir_codegen_target_ir_asmcmp_operand_classification operands[KEFIR_ASMCMP_INSTRUCTION_NUM_OF_OPERANDS];
        };
        kefir_codegen_target_ir_native_id_t attribute;
    };
    kefir_uint64_t extra_flags;
} kefir_codegen_target_ir_asmcmp_instruction_classification_t;

typedef struct kefir_codegen_target_ir_code_constructor_class {
    kefir_result_t (*is_jump)(const struct kefir_asmcmp_context *, kefir_asmcmp_instruction_opcode_t, kefir_bool_t *, void *);
    kefir_result_t (*classify_instruction)(const struct kefir_asmcmp_instruction *, struct kefir_codegen_target_ir_asmcmp_instruction_classification *, void *);
    kefir_result_t (*compute_vreg_pair_part_spill_offset)(kefir_asmcmp_virtual_register_pair_type_t, kefir_size_t, kefir_size_t *, void *);
    void *payload;
} kefir_codegen_target_ir_code_constructor_class_t;

typedef struct kefir_codegen_target_ir_code_constructor_ops {
    const struct kefir_codegen_target_ir_code_constructor_class *klass;
    kefir_result_t (*get_allocation_constraint)(kefir_asmcmp_virtual_register_index_t, struct kefir_codegen_target_ir_allocation_constraint *, void *);
    kefir_result_t (*preallocation_match)(kefir_asmcmp_virtual_register_index_t, kefir_codegen_target_ir_physical_register_t, void *);
    kefir_result_t (*get_native_id_by_label)(kefir_asmcmp_label_index_t, kefir_codegen_target_ir_native_id_t *, void *);
    void *payload;
} kefir_codegen_target_ir_code_constructor_parameters_t;

kefir_result_t kefir_codegen_target_ir_code_construct(struct kefir_mem *, struct kefir_codegen_target_ir_code *, const struct kefir_asmcmp_context *, const struct kefir_codegen_target_ir_code_constructor_ops *);

#endif
