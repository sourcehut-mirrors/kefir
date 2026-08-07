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

#include "kefir/codegen/amd64/x87.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_x87_init(struct kefir_codegen_amd64_x87 *x87) {
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 codegen x87"));

    REQUIRE_OK(kefir_list_init(&x87->x87_stack));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_free(struct kefir_mem *mem, struct kefir_codegen_amd64_x87 *x87) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));

    REQUIRE_OK(kefir_list_free(mem, &x87->x87_stack));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_reset(struct kefir_mem *mem, struct kefir_codegen_amd64_x87 *x87) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));

    REQUIRE_OK(kefir_list_clear(mem, &x87->x87_stack));
    return KEFIR_OK;
}

kefir_size_t kefir_codegen_amd64_x87_length(const struct kefir_codegen_amd64_x87 *x87) {
    REQUIRE(x87 != NULL, 0);

    return kefir_list_length(&x87->x87_stack);
}

kefir_result_t kefir_codegen_amd64_x87_iter(const struct kefir_codegen_amd64_x87 *x87, struct kefir_codegen_amd64_x87_iterator *iter, kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 codegen x87 iterator"));

    iter->iter = kefir_list_head(&x87->x87_stack);
    REQUIRE(iter->iter != NULL, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of amd64 codegen x87 iterator"));

    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) (kefir_uptr_t) iter->iter->value);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_next(struct kefir_codegen_amd64_x87_iterator *iter, kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87 iterator"));

    kefir_list_next(&iter->iter);
    REQUIRE(iter->iter != NULL, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of amd64 codegen x87 iterator"));

    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) (kefir_uptr_t) iter->iter->value);
    return KEFIR_OK;
}

#define X87_STACK_CAPACITY 8

kefir_result_t kefir_codegen_amd64_x87_ensure(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                                       struct kefir_codegen_amd64_x87 *x87, struct kefir_codegen_amd64_stack_frame *stack_frame, const struct kefir_codegen_function *generic,
                                                       kefir_size_t capacity, kefir_bool_t valgrind_compatible_x87) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));
    REQUIRE(generic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen function"));
    REQUIRE(capacity < X87_STACK_CAPACITY,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Requested capacity exceeds x87 stack size"));
    REQUIRE(kefir_list_length(&x87->x87_stack) + capacity > X87_STACK_CAPACITY, KEFIR_OK);

    for (; kefir_list_length(&x87->x87_stack) + capacity > X87_STACK_CAPACITY;) {
        struct kefir_list_entry *iter = kefir_list_tail(&x87->x87_stack);
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);

        REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(stack_frame));

        if (valgrind_compatible_x87) {
            for (kefir_size_t i = 1; i < kefir_list_length(&x87->x87_stack); i++) {
                REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, code,
                                                   kefir_asmcmp_context_instr_tail(&code->context),
                                                   &KEFIR_ASMCMP_MAKE_X87(i), NULL));
            }
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_fdecstp(mem, code,
                                                  kefir_asmcmp_context_instr_tail(&code->context), NULL));
        }

        if (instr_ref == KEFIR_ID_NONE) {
            REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, code,
                                               kefir_asmcmp_context_instr_tail(&code->context),
                                               &KEFIR_ASMCMP_MAKE_X87(0), NULL));

        } else {
            kefir_asmcmp_virtual_register_index_t vreg;
            REQUIRE_OK(generic->resolve_virtual_register(instr_ref, &vreg, generic->payload));
            REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                mem, code, kefir_asmcmp_context_instr_tail(&code->context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
        }

        REQUIRE_OK(kefir_list_pop(mem, &x87->x87_stack, iter));
    }
    return KEFIR_OK;
}


kefir_result_t kefir_codegen_amd64_x87_push(struct kefir_mem *mem,
                                                     struct kefir_asmcmp_amd64 *code, struct kefir_codegen_amd64_x87 *x87,
                                                     struct kefir_codegen_amd64_stack_frame *stack_frame,
                                                     const struct kefir_codegen_function *generic,
                                                     kefir_opt_instruction_ref_t instr_ref, kefir_bool_t valgrind_compatible_x87) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen stack frame"));
    REQUIRE(generic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen function"));

    REQUIRE_OK(kefir_codegen_amd64_x87_ensure(mem, code, x87, stack_frame, generic, 1, valgrind_compatible_x87));
    REQUIRE_OK(kefir_list_insert_after(mem, &x87->x87_stack, NULL, (void *) (kefir_uptr_t) instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_pop(struct kefir_mem *mem, struct kefir_codegen_amd64_x87 *x87) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(kefir_list_length(&x87->x87_stack) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected non-empty amd64 code generator x87 stack"));

    REQUIRE_OK(kefir_list_pop(mem, &x87->x87_stack, kefir_list_head(&x87->x87_stack)));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_load(struct kefir_mem *mem,
                                                     struct kefir_asmcmp_amd64 *code, struct kefir_codegen_amd64_x87 *x87,
                                                     struct kefir_codegen_amd64_stack_frame *stack_frame,
                                                     const struct kefir_codegen_function *generic,
                                                     kefir_opt_instruction_ref_t instr_ref, kefir_bool_t valgrind_compatible_x87) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen stack frame"));
    REQUIRE(generic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen function"));

    kefir_size_t stack_index = 0;
    for (struct kefir_list_entry *iter = kefir_list_head(&x87->x87_stack); iter != NULL;
         iter = iter->next, stack_index++) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, stack_instr_ref, (kefir_uptr_t) iter->value);

        if (stack_instr_ref == instr_ref) {
            // Move to the top
            for (kefir_size_t i = 1; i <= stack_index; i++) {
                REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(stack_frame));
                REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, code,
                                                   kefir_asmcmp_context_instr_tail(&code->context),
                                                   &KEFIR_ASMCMP_MAKE_X87(i), NULL));
            }

            REQUIRE_OK(kefir_list_pop(mem, &x87->x87_stack, iter));
            REQUIRE_OK(kefir_list_insert_after(mem, &x87->x87_stack, NULL, (void *) (kefir_uptr_t) instr_ref));
            return KEFIR_OK;
        }
    }

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_x87_ensure(mem, code, x87, stack_frame, generic, 1, valgrind_compatible_x87));
    kefir_asmcmp_virtual_register_index_t vreg;
    REQUIRE_OK(generic->resolve_virtual_register(instr_ref, &vreg, generic->payload));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, code, kefir_asmcmp_context_instr_tail(&code->context),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                                      NULL));
    REQUIRE_OK(kefir_list_insert_after(mem, &x87->x87_stack, NULL, (void *) (kefir_uptr_t) instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_consume_by(struct kefir_mem *mem, const struct kefir_opt_code_container *opt_code,
                                                     struct kefir_asmcmp_amd64 *code, struct kefir_codegen_amd64_x87 *x87,
                                                     struct kefir_codegen_amd64_stack_frame *stack_frame,
                                                     const struct kefir_codegen_function *generic,
                                                     kefir_opt_instruction_ref_t instr_ref,
                                                           kefir_opt_instruction_ref_t consumer_instr_ref, kefir_abi_amd64_variant_t abi_variant) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(opt_code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen stack frame"));
    REQUIRE(generic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen function"));

    kefir_size_t stack_index = 0;
    for (struct kefir_list_entry *iter = kefir_list_head(&x87->x87_stack); iter != NULL;
         iter = iter->next, stack_index++) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, stack_instr_ref, (kefir_uptr_t) iter->value);

        if (stack_instr_ref == instr_ref) {
            kefir_opt_instruction_ref_t sole_use_ref;
            REQUIRE_OK(kefir_opt_instruction_get_sole_use(opt_code, instr_ref, &sole_use_ref));
            if (consumer_instr_ref != sole_use_ref) {
                REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(stack_frame));
                kefir_asmcmp_virtual_register_index_t vreg;
                REQUIRE_OK(generic->resolve_virtual_register(instr_ref, &vreg, generic->payload));

                if (kefir_list_length(&x87->x87_stack) < X87_STACK_CAPACITY) {
                    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, code,
                                                      kefir_asmcmp_context_instr_tail(&code->context),
                                                      &KEFIR_ASMCMP_MAKE_X87(stack_index), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                        mem, code, kefir_asmcmp_context_instr_tail(&code->context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                } else {
                    kefir_asmcmp_virtual_register_index_t tmp_vreg;
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
                        mem, &code->context,
                        kefir_abi_amd64_long_double_qword_size(abi_variant),
                        kefir_abi_amd64_long_double_qword_alignment(abi_variant), &tmp_vreg));

                    REQUIRE_OK(kefir_asmcmp_amd64_produce_virtual_register(
                        mem, code, kefir_asmcmp_context_instr_tail(&code->context), tmp_vreg,
                        NULL));

                    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                        mem, code, kefir_asmcmp_context_instr_tail(&code->context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                    if (stack_index != 0) {
                        REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, code,
                                                          kefir_asmcmp_context_instr_tail(&code->context),
                                                          &KEFIR_ASMCMP_MAKE_X87(stack_index - 1), NULL));
                    } else {
                        REQUIRE_OK(kefir_asmcmp_amd64_fld(
                            mem, code, kefir_asmcmp_context_instr_tail(&code->context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                    }
                    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                        mem, code, kefir_asmcmp_context_instr_tail(&code->context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_fld(
                        mem, code, kefir_asmcmp_context_instr_tail(&code->context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                }
            }

            return KEFIR_OK;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_load_consume_by(struct kefir_mem *mem, const struct kefir_opt_code_container *opt_code,
                                                     struct kefir_asmcmp_amd64 *code, struct kefir_codegen_amd64_x87 *x87,
                                                     struct kefir_codegen_amd64_stack_frame *stack_frame,
                                                     const struct kefir_codegen_function *generic,
                                                     kefir_opt_instruction_ref_t instr_ref,
                                                           kefir_opt_instruction_ref_t consumer_instr_ref, kefir_bool_t valgrind_compatible_x87) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(opt_code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen stack frame"));
    REQUIRE(generic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen function"));

    kefir_opt_instruction_ref_t sole_use_ref;
    REQUIRE_OK(kefir_opt_instruction_get_sole_use(opt_code, instr_ref, &sole_use_ref));
    if (sole_use_ref == consumer_instr_ref) {
        REQUIRE_OK(kefir_codegen_amd64_x87_load(mem, code, x87, stack_frame, generic, instr_ref, valgrind_compatible_x87));
    } else if (kefir_list_length(&x87->x87_stack) < X87_STACK_CAPACITY) {
        kefir_size_t stack_index = 0;
        for (struct kefir_list_entry *iter = kefir_list_head(&x87->x87_stack); iter != NULL;
             iter = iter->next, stack_index++) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, stack_instr_ref, (kefir_uptr_t) iter->value);
            if (stack_instr_ref == instr_ref) {
                REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, code,
                                                  kefir_asmcmp_context_instr_tail(&code->context),
                                                  &KEFIR_ASMCMP_MAKE_X87(stack_index), NULL));
                REQUIRE_OK(
                    kefir_list_insert_after(mem, &x87->x87_stack, NULL, (void *) (kefir_uptr_t) KEFIR_ID_NONE));
                return KEFIR_OK;
            }
        }

        REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(stack_frame));
        kefir_asmcmp_virtual_register_index_t vreg;
        REQUIRE_OK(generic->resolve_virtual_register(instr_ref, &vreg, generic->payload));
        REQUIRE_OK(kefir_asmcmp_amd64_fld(
            mem, code, kefir_asmcmp_context_instr_tail(&code->context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
        REQUIRE_OK(kefir_list_insert_after(mem, &x87->x87_stack, NULL, (void *) (kefir_uptr_t) KEFIR_ID_NONE));
    } else {
        REQUIRE_OK(kefir_codegen_amd64_x87_load(mem, code, x87, stack_frame, generic, instr_ref, valgrind_compatible_x87));

        REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(stack_frame));
        kefir_asmcmp_virtual_register_index_t vreg;
        REQUIRE_OK(generic->resolve_virtual_register(instr_ref, &vreg, generic->payload));
        REQUIRE_OK(kefir_asmcmp_amd64_fstp(
            mem, code, kefir_asmcmp_context_instr_tail(&code->context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_fld(
            mem, code, kefir_asmcmp_context_instr_tail(&code->context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_flush(struct kefir_mem *mem,
                                                     struct kefir_asmcmp_amd64 *code, struct kefir_codegen_amd64_x87 *x87,
                                                     struct kefir_codegen_amd64_stack_frame *stack_frame,
                                                     const struct kefir_codegen_function *generic) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen stack frame"));
    REQUIRE(generic != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen function"));

    for (struct kefir_list_entry *iter = kefir_list_head(&x87->x87_stack); iter != NULL;
         iter = kefir_list_head(&x87->x87_stack)) {
        REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(stack_frame));
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, stack_instr_ref, (kefir_uptr_t) iter->value);

        if (stack_instr_ref != KEFIR_ID_NONE) {
            kefir_asmcmp_virtual_register_index_t vreg;
            REQUIRE_OK(generic->resolve_virtual_register(stack_instr_ref, &vreg, generic->payload));
            REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                mem, code, kefir_asmcmp_context_instr_tail(&code->context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, code,
                                               kefir_asmcmp_context_instr_tail(&code->context),
                                               &KEFIR_ASMCMP_MAKE_X87(0), NULL));
        }

        REQUIRE_OK(kefir_list_pop(mem, &x87->x87_stack, iter));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_x87_clear(struct kefir_mem *mem,
                                                     struct kefir_asmcmp_amd64 *code, struct kefir_codegen_amd64_x87 *x87,
                                                     kefir_size_t preserve_top) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp"));
    REQUIRE(x87 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen x87"));
    REQUIRE(preserve_top <= X87_STACK_CAPACITY,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Number of preserved x87 stack slots exceeds stack size"));
    REQUIRE(kefir_list_length(&x87->x87_stack) > preserve_top, KEFIR_OK);

    for (kefir_size_t i = 0; i < preserve_top; i++) {
        REQUIRE_OK(kefir_asmcmp_amd64_fxch(
            mem, code, kefir_asmcmp_context_instr_tail(&code->context),
            &KEFIR_ASMCMP_MAKE_X87(kefir_list_length(&x87->x87_stack) - preserve_top + i), NULL));
    }

    for (kefir_size_t i = 0; i < kefir_list_length(&x87->x87_stack) - preserve_top; i++) {
        REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, code,
                                           kefir_asmcmp_context_instr_tail(&code->context),
                                           &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    }
    REQUIRE_OK(kefir_list_clear(mem, &x87->x87_stack));

    return KEFIR_OK;
}
