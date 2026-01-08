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

#ifndef KEFIR_CODEGEN_TARGET_IR_DESTRUCTOR_OPS_H_
#define KEFIR_CODEGEN_TARGET_IR_DESTRUCTOR_OPS_H_

#include "kefir/codegen/target-ir/constructor.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/schedule.h"

typedef struct kefir_codegen_target_ir_destructor_ops {
    kefir_asmcmp_instruction_opcode_t unreachable_opcode;
    kefir_asmcmp_instruction_opcode_t noop_opcode;
    kefir_result_t (*bind_native_id)(struct kefir_mem *, kefir_asmcmp_label_index_t, kefir_codegen_target_ir_native_id_t, void *);
    kefir_result_t (*new_inline_asm)(struct kefir_mem *, kefir_asmcmp_instruction_index_t, kefir_asmcmp_inline_assembly_index_t, kefir_asmcmp_instruction_index_t *, void *);
    kefir_result_t (*materialize_attribute)(struct kefir_mem *, kefir_asmcmp_instruction_index_t, kefir_codegen_target_ir_native_id_t, kefir_asmcmp_instruction_index_t *, void *);
    kefir_result_t (*new_code_fragment)(struct kefir_mem *, kefir_codegen_target_ir_metadata_code_ref_t, kefir_asmcmp_label_index_t, kefir_asmcmp_label_index_t, void *);
    kefir_result_t (*new_value_fragment)(struct kefir_mem *, kefir_codegen_target_ir_metadata_value_ref_t, kefir_asmcmp_debug_info_value_location_reference_t, kefir_asmcmp_label_index_t, kefir_asmcmp_label_index_t, void *);
    kefir_result_t (*schedule_code)(struct kefir_mem *, const struct kefir_codegen_target_ir_control_flow *, struct kefir_codegen_target_ir_code_schedule *, void *);
    void *payload;
} kefir_codegen_target_ir_destructor_parameter_t;


#endif
