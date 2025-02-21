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

#ifndef KEFIR_OPTIMIZER_INSTR_UTIL_H_
#define KEFIR_OPTIMIZER_INSTR_UTIL_H_

#include "kefir/optimizer/code.h"

kefir_result_t kefir_opt_instruction_extract_inputs(const struct kefir_opt_code_container *,
                                                    const struct kefir_opt_instruction *, kefir_bool_t,
                                                    kefir_result_t (*)(kefir_opt_instruction_ref_t, void *), void *);

kefir_result_t kefir_opt_code_instruction_is_control_flow(const struct kefir_opt_code_container *,
                                                          kefir_opt_instruction_ref_t, kefir_bool_t *);

#define KEFIR_OPT_INSTRUCTION_IS_NONVOLATILE_LOAD(_instr)                   \
    (((_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD_SIGNED ||    \
      (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD_UNSIGNED ||  \
      (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD_SIGNED ||   \
      (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD_UNSIGNED || \
      (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD_SIGNED ||   \
      (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD_UNSIGNED || \
      (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT64_LOAD ||          \
      (_instr)->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD) &&   \
     !(_instr)->operation.parameters.memory_access.flags.volatile_access)

kefir_result_t kefir_opt_code_block_merge_into(struct kefir_mem *, struct kefir_opt_code_container *,
                                               kefir_opt_block_id_t, kefir_opt_block_id_t, kefir_bool_t);
kefir_result_t kefir_opt_code_block_redirect_phi_links(struct kefir_mem *, struct kefir_opt_code_container *,
                                                       kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                       kefir_opt_block_id_t);

#endif
