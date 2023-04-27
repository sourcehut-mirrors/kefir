/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#ifndef KEFIR_OPTIMIZER_BUILDER_H_
#define KEFIR_OPTIMIZER_BUILDER_H_

#include "kefir/optimizer/code.h"

kefir_result_t kefir_opt_code_builder_add_instruction(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, const struct kefir_opt_operation *,
                                                      kefir_bool_t, kefir_opt_instruction_ref_t *);

kefir_result_t kefir_opt_code_builder_finalize_jump(struct kefir_mem *, struct kefir_opt_code_container *,
                                                    kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                    kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_indirect_jump(struct kefir_mem *, struct kefir_opt_code_container *,
                                                             kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                             kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_branch(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                      kefir_opt_block_id_t, kefir_opt_block_id_t,
                                                      kefir_opt_instruction_ref_t *);
kefir_result_t kefir_opt_code_builder_finalize_return(struct kefir_mem *, struct kefir_opt_code_container *,
                                                      kefir_opt_block_id_t, kefir_opt_instruction_ref_t,
                                                      kefir_opt_instruction_ref_t *);

#endif
