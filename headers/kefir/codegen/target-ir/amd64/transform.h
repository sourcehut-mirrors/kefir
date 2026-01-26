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

#ifndef KEFIR_CODEGEN_TARGET_IR_AMD64_TRANSFORM_H_
#define KEFIR_CODEGEN_TARGET_IR_AMD64_TRANSFORM_H_

#include "kefir/codegen/target-ir/code.h"
#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/codegen/target-ir/control_flow.h"

kefir_result_t kefir_codegen_target_ir_amd64_transform_peephole(struct kefir_mem *,
                                                                struct kefir_codegen_target_ir_code *);
kefir_result_t kefir_codegen_target_ir_amd64_transform_dead_code_elimination(struct kefir_mem *,
                                                                             struct kefir_codegen_target_ir_code *);
kefir_result_t kefir_codegen_target_ir_amd64_transform_rematerialize(
    struct kefir_mem *, struct kefir_codegen_target_ir_code *, const struct kefir_codegen_target_ir_control_flow *,
    const struct kefir_codegen_target_ir_liveness *, const struct kefir_codegen_target_ir_regalloc *);

kefir_result_t kefir_codegen_target_ir_amd64_is_rematerializable(const struct kefir_codegen_target_ir_code *,
                                                                 const struct kefir_codegen_target_ir_liveness *,
                                                                 kefir_codegen_target_ir_value_ref_t,
                                                                 kefir_codegen_target_ir_block_ref_t, kefir_bool_t *);

#ifdef KEFIR_CODEGEN_TARGET_IR_AMD64_PEEPHOLE_INTERNAL
kefir_result_t kefir_codegen_target_ir_amd64_peephole_const_operand(struct kefir_mem *,
                                                                    struct kefir_codegen_target_ir_code *,
                                                                    const struct kefir_codegen_target_ir_instruction *,
                                                                    kefir_bool_t, kefir_bool_t *);

kefir_result_t kefir_codegen_target_ir_amd64_peephole_add(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_adc(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_sbb(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_imul(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_imul3(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_div(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_idiv(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_xor(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_or(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                         const struct kefir_codegen_target_ir_instruction *,
                                                         kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_and(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_shl(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_shr(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_sar(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_shxd(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_rol(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_btc(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_test(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_cmp(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_movx(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_mov(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                          const struct kefir_codegen_target_ir_instruction *,
                                                          kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setcc(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_sete(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setne(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setnp(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setp(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setnc(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setc(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setno(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_seto(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setns(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_sets(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setnb(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setb(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setge(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setl(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setg(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setle(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_setbe(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                            const struct kefir_codegen_target_ir_instruction *,
                                                            kefir_bool_t *);
kefir_result_t kefir_codegen_target_ir_amd64_peephole_seta(struct kefir_mem *, struct kefir_codegen_target_ir_code *,
                                                           const struct kefir_codegen_target_ir_instruction *,
                                                           kefir_bool_t *);

#endif

#endif
