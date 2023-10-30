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

#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/register_allocator.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/target/asm/amd64/xasmgen.h"

static kefir_result_t opcode_mnemonic(kefir_asmcmp_instruction_opcode_t opcode, const char **mnemonic_ptr,
                                      void *payload) {
    UNUSED(payload);
    REQUIRE(mnemonic_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to opcode mnemonic"));

    switch (opcode) {
#define CASE(_opcode, _xasmgen, _argtp)      \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode): \
        *mnemonic_ptr = #_opcode;            \
        break;

        KEFIR_ASMCMP_AMD64_OPCODES(CASE, )
#undef CASE

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 opcode");
    }
    return KEFIR_OK;
}

static const struct kefir_asmcmp_context_class AMD64_KLASS = {.opcode_mnemonic = opcode_mnemonic};

struct register_preallocation {
    kefir_asmcmp_amd64_register_preallocation_type_t type;
    union {
        kefir_asm_amd64_xasmgen_register_t reg;
        kefir_asmcmp_virtual_register_index_t vreg;
    };
};

static kefir_result_t free_register_preallocation(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                  kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                                  void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct register_preallocation *, preallocation, value);
    REQUIRE(preallocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register preallocation"));

    KEFIR_FREE(mem, preallocation);
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_init(const char *function_name, kefir_abi_amd64_variant_t abi_variant,
                                       struct kefir_asmcmp_amd64 *target) {
    REQUIRE(function_name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid function name"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmgen amd64 target"));

    REQUIRE_OK(kefir_asmcmp_context_init(&AMD64_KLASS, target, &target->context));
    REQUIRE_OK(kefir_hashtree_init(&target->register_preallocation, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&target->register_preallocation, free_register_preallocation, NULL));
    target->function_name = function_name;
    target->abi_variant = abi_variant;
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_free(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));

    REQUIRE_OK(kefir_hashtree_free(mem, &target->register_preallocation));
    REQUIRE_OK(kefir_asmcmp_context_free(mem, &target->context));
    return KEFIR_OK;
}

#define PREALLOCATION_IMPL(_mem, _target, _vreg_idx, _init, _else)                                                    \
    do {                                                                                                              \
        REQUIRE((_mem) != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));         \
        REQUIRE((_target) != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));   \
        const struct kefir_asmcmp_virtual_register *vreg;                                                             \
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&(_target)->context, (_vreg_idx), &vreg));                       \
                                                                                                                      \
        struct register_preallocation *preallocation = NULL;                                                          \
        struct kefir_hashtree_node *node = NULL;                                                                      \
        kefir_result_t res =                                                                                          \
            kefir_hashtree_at(&(_target)->register_preallocation, (kefir_hashtree_key_t) (_vreg_idx), &node);         \
        if (res == KEFIR_NOT_FOUND) {                                                                                 \
            preallocation = KEFIR_MALLOC((_mem), sizeof(struct register_preallocation));                              \
            REQUIRE(preallocation != NULL,                                                                            \
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register preallocation"));      \
                                                                                                                      \
            _init res =                                                                                               \
                kefir_hashtree_insert((_mem), &(_target)->register_preallocation, (kefir_hashtree_key_t) (_vreg_idx), \
                                      (kefir_hashtree_value_t) preallocation);                                        \
            REQUIRE_ELSE(res == KEFIR_OK, {                                                                           \
                KEFIR_FREE(mem, preallocation);                                                                       \
                return res;                                                                                           \
            });                                                                                                       \
        } else {                                                                                                      \
            REQUIRE_OK(res);                                                                                          \
            _else                                                                                                     \
        }                                                                                                             \
    } while (false)

kefir_result_t kefir_asmcmp_amd64_register_allocation_same_as(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                              kefir_asmcmp_virtual_register_index_t vreg_idx,
                                                              kefir_asmcmp_virtual_register_index_t other_vreg_idx) {
    PREALLOCATION_IMPL(mem, target, vreg_idx,
                       {
                           preallocation->type = KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS;
                           preallocation->vreg = other_vreg_idx;
                       },
                       {
                           // Ignore hint
                       });

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_register_allocation_hint(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                           kefir_asmcmp_virtual_register_index_t vreg_idx,
                                                           kefir_asm_amd64_xasmgen_register_t reg) {
    PREALLOCATION_IMPL(
        mem, target, vreg_idx,
        {
            preallocation->type = KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT;
            preallocation->reg = reg;
        },
        {
            if (preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS) {
                preallocation->type = KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT;
                preallocation->reg = reg;
            }
        });
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_register_allocation_requirement(struct kefir_mem *mem,
                                                                  struct kefir_asmcmp_amd64 *target,
                                                                  kefir_asmcmp_virtual_register_index_t vreg_idx,
                                                                  kefir_asm_amd64_xasmgen_register_t reg) {
    PREALLOCATION_IMPL(
        mem, target, vreg_idx,
        {
            preallocation->type = KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT;
            preallocation->reg = reg;
        },
        {
            if (preallocation->type != KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT) {
                preallocation->type = KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT;
                preallocation->reg = reg;
            } else {
                REQUIRE(
                    preallocation->reg == reg,
                    KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Conflicting amd64 register preallocation requirements"));
            }
        });
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_get_register_preallocation(
    const struct kefir_asmcmp_amd64 *target, kefir_asmcmp_virtual_register_index_t vreg_idx,
    const struct kefir_asmcmp_amd64_register_preallocation **preallocation_ptr) {
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));
    REQUIRE(preallocation_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register preallocation"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&target->register_preallocation, (kefir_hashtree_key_t) vreg_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        *preallocation_ptr = NULL;
        return KEFIR_OK;
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct kefir_asmcmp_amd64_register_preallocation *, preallocation, node->value);
    *preallocation_ptr = preallocation;
    return KEFIR_OK;
}

#define DEF_OPCODE_virtual(_opcode)
#define DEF_OPCODE_arg0(_opcode)                                                                                 \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,        \
                                                kefir_asmcmp_instruction_index_t after_index,                    \
                                                kefir_asmcmp_instruction_index_t *idx_ptr) {                     \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));       \
        REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target")); \
        REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(                                                      \
            mem, &target->context, after_index,                                                                  \
            &(const struct kefir_asmcmp_instruction){.opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode),               \
                                                     .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,               \
                                                     .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,               \
                                                     .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE},              \
            idx_ptr));                                                                                           \
        return KEFIR_OK;                                                                                         \
    }
#define DEF_OPCODE_arg2(_opcode)                                                                                 \
    kefir_result_t kefir_asmcmp_amd64_##_opcode(                                                                 \
        struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target, kefir_asmcmp_instruction_index_t after_index,  \
        const struct kefir_asmcmp_value *arg1, const struct kefir_asmcmp_value *arg2,                            \
        kefir_asmcmp_instruction_index_t *idx_ptr) {                                                             \
        REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));       \
        REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target")); \
        REQUIRE(arg1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 value"));    \
        REQUIRE(arg2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 value"));    \
        REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(                                                      \
            mem, &target->context, after_index,                                                                  \
            &(const struct kefir_asmcmp_instruction){.opcode = KEFIR_ASMCMP_AMD64_OPCODE(_opcode),               \
                                                     .args[0] = *arg1,                                           \
                                                     .args[1] = *arg2,                                           \
                                                     .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE},              \
            idx_ptr));                                                                                           \
        return KEFIR_OK;                                                                                         \
    }
#define DEF_OPCODE(_opcode, _xasmgen, _argtp) DEF_OPCODE_##_argtp(_opcode)

KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, )

#undef DEF_OPCODE_virtual
#undef DEF_OPCODE_arg0
#undef DEF_OPCODE_arg2
#undef DEF_OPCODE

kefir_result_t kefir_asmcmp_amd64_link_virtual_registers(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                         kefir_asmcmp_instruction_index_t after_index,
                                                         kefir_asmcmp_virtual_register_index_t vreg1,
                                                         kefir_asmcmp_virtual_register_index_t vreg2,
                                                         kefir_asmcmp_instruction_index_t *idx_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(
        mem, &target->context, after_index,
        &(const struct kefir_asmcmp_instruction){
            .opcode = KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link),
            .args[0] = {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
                        .vreg = {.index = vreg1, .variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT}},
            .args[1] = {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
                        .vreg = {.index = vreg2, .variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT}},
            .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE},
        idx_ptr));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_same_as(mem, target, vreg1, vreg2));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_touch_virtual_register(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                         kefir_asmcmp_instruction_index_t after_index,
                                                         kefir_asmcmp_virtual_register_index_t vreg,
                                                         kefir_asmcmp_instruction_index_t *idx_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(
        mem, &target->context, after_index,
        &(const struct kefir_asmcmp_instruction){
            .opcode = KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register),
            .args[0] = {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
                        .vreg = {.index = vreg, .variant = KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT}},
            .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
            .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE},
        idx_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_function_prologue(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                    kefir_asmcmp_instruction_index_t after_index,
                                                    kefir_asmcmp_instruction_index_t *idx_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(
        mem, &target->context, after_index,
        &(const struct kefir_asmcmp_instruction){.opcode = KEFIR_ASMCMP_AMD64_OPCODE(function_prologue),
                                                 .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                                                 .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                                                 .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE},
        idx_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_function_epilogue(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                    kefir_asmcmp_instruction_index_t after_index,
                                                    kefir_asmcmp_instruction_index_t *idx_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));
    REQUIRE_OK(kefir_asmcmp_context_instr_insert_after(
        mem, &target->context, after_index,
        &(const struct kefir_asmcmp_instruction){.opcode = KEFIR_ASMCMP_AMD64_OPCODE(function_epilogue),
                                                 .args[0].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                                                 .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
                                                 .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE},
        idx_ptr));
    return KEFIR_OK;
}
