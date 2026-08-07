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

#ifndef KEFIR_CODEGEN_AMD64_UTIL_H_
#define KEFIR_CODEGEN_AMD64_UTIL_H_

#include "kefir/optimizer/code.h"
#include "kefir/codegen/asmcmp/type_defs.h"

typedef struct kefir_codegen_amd64_module kefir_codegen_amd64_module_t;
typedef struct kefir_codegen_amd64_function kefir_codegen_amd64_function_t;

kefir_result_t kefir_codegen_amd64_function_map_phi_outputs(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                            kefir_opt_block_id_t, kefir_opt_block_id_t);

kefir_result_t kefir_codegen_amd64_return_from_function(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                        kefir_opt_instruction_ref_t,
                                                        kefir_asmcmp_virtual_register_index_t);

kefir_result_t kefir_codegen_amd64_copy_memory(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                               kefir_asmcmp_virtual_register_index_t,
                                               kefir_asmcmp_virtual_register_index_t, kefir_size_t);
kefir_result_t kefir_codegen_amd64_zero_memory(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                               kefir_asmcmp_virtual_register_index_t, kefir_size_t);

kefir_result_t kefir_codegen_amd64_translate_builtin(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                     const struct kefir_opt_instruction *, kefir_bool_t *,
                                                     kefir_asmcmp_virtual_register_index_t *);

kefir_result_t kefir_codegen_amd64_load_general_purpose_register(struct kefir_mem *,
                                                                 struct kefir_codegen_amd64_function *,
                                                                 kefir_asmcmp_instruction_index_t,
                                                                 kefir_asmcmp_instruction_index_t, kefir_size_t,
                                                                 kefir_int64_t);
kefir_result_t kefir_codegen_amd64_load_floating_point_register(struct kefir_mem *,
                                                                struct kefir_codegen_amd64_function *,
                                                                kefir_asmcmp_instruction_index_t,
                                                                kefir_asmcmp_instruction_index_t, kefir_size_t,
                                                                kefir_int64_t);
kefir_result_t kefir_codegen_amd64_store_general_purpose_register(struct kefir_mem *,
                                                                  struct kefir_codegen_amd64_function *,
                                                                  kefir_asmcmp_instruction_index_t,
                                                                  kefir_asmcmp_instruction_index_t, kefir_size_t,
                                                                  kefir_int64_t);
kefir_result_t kefir_codegen_amd64_store_floating_point_register(struct kefir_mem *,
                                                                 struct kefir_codegen_amd64_function *,
                                                                 kefir_asmcmp_instruction_index_t,
                                                                 kefir_asmcmp_instruction_index_t, kefir_size_t,
                                                                 kefir_int64_t);

kefir_result_t kefir_codegen_amd64_tail_call_possible(struct kefir_mem *, const struct kefir_codegen_amd64_function *,
                                                      kefir_opt_call_id_t, kefir_bool_t *);
kefir_result_t kefir_codegen_amd64_tail_call_return_aggregate_passthrough(const struct kefir_codegen_amd64_function *,
                                                                          kefir_opt_call_id_t, kefir_bool_t *);


kefir_result_t kefir_codegen_amd64_function_int_to_float(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                         kefir_asmcmp_instruction_index_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_uint_to_float(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                          kefir_asmcmp_instruction_index_t,
                                                          kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_int_to_double(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                          kefir_asmcmp_instruction_index_t,
                                                          kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_uint_to_double(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                           kefir_asmcmp_instruction_index_t,
                                                           kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_int_to_long_double(struct kefir_mem *,
                                                               struct kefir_codegen_amd64_function *,
                                                               kefir_asmcmp_instruction_index_t,
                                                               kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_uint_to_long_double(struct kefir_mem *,
                                                                struct kefir_codegen_amd64_function *,
                                                                kefir_asmcmp_instruction_index_t,
                                                                kefir_opt_instruction_ref_t);

kefir_result_t kefir_codegen_amd64_function_float_to_int(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                         kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_float_to_uint(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                          kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_double_to_int(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                          kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_double_to_uint(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                           kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_long_double_to_int(struct kefir_mem *,
                                                               struct kefir_codegen_amd64_function *,
                                                               kefir_opt_instruction_ref_t,
                                                               kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_long_double_to_uint(struct kefir_mem *,
                                                                struct kefir_codegen_amd64_function *,
                                                                kefir_opt_instruction_ref_t,
                                                                kefir_opt_instruction_ref_t);

kefir_result_t kefir_codegen_amd64_get_atomic_memorder(kefir_opt_memory_order_t, kefir_int64_t *);

kefir_result_t kefir_codegen_amd64_function_call_preserve_regs(struct kefir_mem *,
                                                               struct kefir_codegen_amd64_function *,
                                                               const struct kefir_opt_call_node *,
                                                               kefir_asmcmp_virtual_register_index_t *);

kefir_result_t kefir_codegen_amd64_do_call_direct(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                  const char *, kefir_asmcmp_instruction_index_t *);

#define KEFIR_AMD64_CODEGEN_INSTR_CONSUMES_8BIT_BOOL(_instr, _consumed_ref)                                    \
    ((_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_AND ||                                           \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_OR ||                                            \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_NOT ||                                           \
     ((_instr)->operation.opcode == KEFIR_OPT_OPCODE_BRANCH &&                                                 \
      ((_instr)->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_8BIT ||           \
       (_instr)->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT)) || \
     ((_instr)->operation.opcode == KEFIR_OPT_OPCODE_SELECT &&                                                 \
      (_instr)->operation.parameters.refs[0] == (_consumed_ref) &&                                             \
      (_instr)->operation.parameters.refs[1] != (_consumed_ref) &&                                             \
      (_instr)->operation.parameters.refs[2] != (_consumed_ref) &&                                             \
      ((_instr)->operation.parameters.condition_variant == KEFIR_OPT_BRANCH_CONDITION_8BIT ||                  \
       (_instr)->operation.parameters.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT)))

#define KEFIR_CODEGEN_AMD64_FUNCTION_X87_ENSURE(_mem, _func, _capacity) (kefir_codegen_amd64_x87_ensure((_mem), &(_func)->code, &(_func)->translator.x87, &(_func)->stack_frame, &(_func)->generic, (_capacity), (_func)->codegen->config->valgrind_compatible_x87))
#define KEFIR_CODEGEN_AMD64_FUNCTION_X87_PUSH(_mem, _func, _instr_ref) (kefir_codegen_amd64_x87_push((_mem), &(_func)->code, &(_func)->translator.x87, &(_func)->stack_frame, &(_func)->generic, (_instr_ref), (_func)->codegen->config->valgrind_compatible_x87))
#define KEFIR_CODEGEN_AMD64_FUNCTION_X87_POP(_mem, _func) (kefir_codegen_amd64_x87_pop((_mem), &(_func)->translator.x87))
#define KEFIR_CODEGEN_AMD64_FUNCTION_X87_LOAD(_mem, _func, _instr_ref) (kefir_codegen_amd64_x87_load((_mem), &(_func)->code, &(_func)->translator.x87, &(_func)->stack_frame, &(_func)->generic, (_instr_ref), (_func)->codegen->config->valgrind_compatible_x87))
#define KEFIR_CODEGEN_AMD64_FUNCTION_X87_LOAD_CONSUME_BY(_mem, _func, _instr_ref, _consumer_instr_ref) (kefir_codegen_amd64_x87_load_consume_by((_mem), &(_func)->generic.function->code, &(_func)->code, &(_func)->translator.x87, &(_func)->stack_frame, &(_func)->generic, (_instr_ref), (_consumer_instr_ref), (_func)->codegen->config->valgrind_compatible_x87))
#define KEFIR_CODEGEN_AMD64_FUNCTION_X87_CONSUME_BY(_mem, _func, _instr_ref, _consumer_instr_ref) (kefir_codegen_amd64_x87_consume_by((_mem), &(_func)->generic.function->code, &(_func)->code, &(_func)->translator.x87, &(_func)->stack_frame, &(_func)->generic, (_instr_ref), (_consumer_instr_ref), (_func)->codegen->abi_variant))
#define KEFIR_CODEGEN_AMD64_FUNCTION_X87_FLUSH(_mem, _func) (kefir_codegen_amd64_x87_flush((_mem), &(_func)->code, &(_func)->translator.x87, &(_func)->stack_frame, &(_func)->generic))
#define KEFIR_CODEGEN_AMD64_FUNCTION_X87_CLEAR(_mem, _func, _preserve_top) (kefir_codegen_amd64_x87_clear((_mem), &(_func)->code, &(_func)->translator.x87, (_preserve_top)))


#endif
