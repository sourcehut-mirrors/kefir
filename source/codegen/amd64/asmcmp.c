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
                        .vreg = {.index = vreg1, .variant = KEFIR_ASMCMP_REGISTER_VARIANT_NONE}},
            .args[1] = {.type = KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER,
                        .vreg = {.index = vreg2, .variant = KEFIR_ASMCMP_REGISTER_VARIANT_NONE}},
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
                        .vreg = {.index = vreg, .variant = KEFIR_ASMCMP_REGISTER_VARIANT_NONE}},
            .args[1].type = KEFIR_ASMCMP_VALUE_TYPE_NONE,
            .args[2].type = KEFIR_ASMCMP_VALUE_TYPE_NONE},
        idx_ptr));
    return KEFIR_OK;
}

#define LABEL_FMT "_kefir_func_%s_label%" KEFIR_ID_FMT
#define FMT_BUF_LEN 255

static kefir_result_t build_xasmgen_operand(struct kefir_asm_amd64_xasmgen_operand *operand,
                                            const struct kefir_asm_amd64_xasmgen_operand **result_op,
                                            char buf[static FMT_BUF_LEN + 1], const struct kefir_asmcmp_amd64 *target,
                                            const struct kefir_codegen_amd64_register_allocator *register_allocation,
                                            const struct kefir_asmcmp_value *value) {
    UNUSED(buf);
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 asmgen none value");

        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
            *result_op = kefir_asm_amd64_xasmgen_operand_imm(operand, value->int_immediate);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
            *result_op = kefir_asm_amd64_xasmgen_operand_immu(operand, value->uint_immediate);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.variant) {
                case KEFIR_ASMCMP_REGISTER_VARIANT_8BIT:
                    snprintf(buf, FMT_BUF_LEN, "indirect%" KEFIR_INT64_FMT "_8bit", value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_REGISTER_VARIANT_16BIT:
                    snprintf(buf, FMT_BUF_LEN, "indirect%" KEFIR_INT64_FMT "_16bit", value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_REGISTER_VARIANT_32BIT:
                    snprintf(buf, FMT_BUF_LEN, "indirect%" KEFIR_INT64_FMT "_32bit", value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_REGISTER_VARIANT_64BIT:
                    snprintf(buf, FMT_BUF_LEN, "indirect%" KEFIR_INT64_FMT "_64bit", value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_REGISTER_VARIANT_NONE:
                    snprintf(buf, FMT_BUF_LEN, "indirect%" KEFIR_INT64_FMT, value->indirect.offset);
                    break;
            }
            *result_op = kefir_asm_amd64_xasmgen_operand_label(operand, buf);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER: {
            const struct kefir_asmcmp_virtual_register *vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&target->context, value->vreg.index, &vreg));
            const struct kefir_codegen_amd64_register_allocation *reg_alloc;
            REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(register_allocation, value->vreg.index, &reg_alloc));
            switch (reg_alloc->type) {
                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register");

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER: {
                    kefir_asm_amd64_xasmgen_register_t reg = reg_alloc->direct_reg;
                    switch (value->vreg.variant) {
                        case KEFIR_ASMCMP_REGISTER_VARIANT_8BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(reg, &reg));
                            break;

                        case KEFIR_ASMCMP_REGISTER_VARIANT_16BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(reg, &reg));
                            break;

                        case KEFIR_ASMCMP_REGISTER_VARIANT_32BIT:
                            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(reg, &reg));
                            break;

                        case KEFIR_ASMCMP_REGISTER_VARIANT_NONE:
                        case KEFIR_ASMCMP_REGISTER_VARIANT_64BIT:
                            // Intentionally left blank
                            break;
                    }

                    *result_op = kefir_asm_amd64_xasmgen_operand_reg(reg);
                } break;

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                    snprintf(buf, FMT_BUF_LEN, "spill_area%" KEFIR_SIZE_FMT, reg_alloc->spill_area_index);
                    *result_op = kefir_asm_amd64_xasmgen_operand_label(operand, buf);
                    break;
            }
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_LOCAL_VAR_ADDRESS:
            snprintf(buf, FMT_BUF_LEN, "localvar%" KEFIR_SIZE_FMT, value->local_var_offset);
            *result_op = kefir_asm_amd64_xasmgen_operand_label(operand, buf);
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t generate_instr(struct kefir_amd64_xasmgen *xasmgen, const struct kefir_asmcmp_amd64 *target,
                                     const struct kefir_codegen_amd64_register_allocator *register_allocation,
                                     kefir_asmcmp_instruction_index_t index) {
    const struct kefir_asmcmp_instruction *instr;
    REQUIRE_OK(kefir_asmcmp_context_instr_at(&target->context, index, &instr));
    for (kefir_asmcmp_label_index_t label = kefir_asmcmp_context_instr_label_head(&target->context, index);
         label != KEFIR_ASMCMP_INDEX_NONE; label = kefir_asmcmp_context_instr_label_next(&target->context, label)) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, LABEL_FMT, target->function_name, label));
    }

    struct kefir_asm_amd64_xasmgen_operand operands[3];
    const struct kefir_asm_amd64_xasmgen_operand *result_operands[3];
    char operand_bufs[3][FMT_BUF_LEN + 1];

    switch (instr->opcode) {
#define DEF_OPCODE_virtual(_opcode, _xasmgen)
#define DEF_OPCODE_arg0(_opcode, _xasmgen)                         \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                       \
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen)); \
        break;
#define DEF_OPCODE_arg2(_opcode, _xasmgen)                                                                 \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                               \
        REQUIRE_OK(build_xasmgen_operand(&operands[0], &result_operands[0], operand_bufs[0], target,       \
                                         register_allocation, &instr->args[0]));                           \
        REQUIRE_OK(build_xasmgen_operand(&operands[1], &result_operands[1], operand_bufs[1], target,       \
                                         register_allocation, &instr->args[1]));                           \
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_##_xasmgen(xasmgen, result_operands[0], result_operands[1])); \
        break;
#define DEF_OPCODE(_opcode, _xasmgen, _argtp) DEF_OPCODE_##_argtp(_opcode, _xasmgen)

        KEFIR_ASMCMP_AMD64_OPCODES(DEF_OPCODE, ;);
#undef DEF_OPCODE
#undef DEF_OPCODE_virtual
#undef DEF_OPCODE_arg0
#undef DEF_OPCODE_arg2

        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link): {
            const struct kefir_codegen_amd64_register_allocation *target_allocation, *source_allocation;
            REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(register_allocation, instr->args[0].vreg.index,
                                                                  &target_allocation));
            REQUIRE_OK(kefir_codegen_amd64_register_allocation_of(register_allocation, instr->args[1].vreg.index,
                                                                  &source_allocation));

            if (!((target_allocation->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER &&
                   source_allocation->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER &&
                   target_allocation->direct_reg == source_allocation->direct_reg)) ||
                (target_allocation->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA &&
                 source_allocation->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA &&
                 target_allocation->spill_area_index == source_allocation->spill_area_index)) {
                // TODO Spill as necessary
                REQUIRE_OK(build_xasmgen_operand(&operands[0], &result_operands[0], operand_bufs[0], target,
                                                 register_allocation, &instr->args[0]));
                REQUIRE_OK(build_xasmgen_operand(&operands[1], &result_operands[1], operand_bufs[1], target,
                                                 register_allocation, &instr->args[1]));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(xasmgen, result_operands[0], result_operands[1]));
            }
        } break;

        case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_generate_code(
    struct kefir_amd64_xasmgen *xasmgen, const struct kefir_asmcmp_amd64 *target,
    const struct kefir_codegen_amd64_register_allocator *register_allocation) {
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 assembly generator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp amd64 target"));
    REQUIRE(register_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocation"));

    for (kefir_asmcmp_instruction_index_t idx = kefir_asmcmp_context_instr_head(&target->context);
         idx != KEFIR_ASMCMP_INDEX_NONE; idx = kefir_asmcmp_context_instr_next(&target->context, idx)) {
        REQUIRE_OK(generate_instr(xasmgen, target, register_allocation, idx));
    }
    return KEFIR_OK;
}
