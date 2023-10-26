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
    kefir_asm_amd64_xasmgen_register_t reg;
    kefir_asmcmp_amd64_register_preallocation_type_t type;
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

kefir_result_t kefir_asmcmp_amd64_preallocate_register(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                       kefir_asmcmp_virtual_register_index_t vreg_idx,
                                                       kefir_asmcmp_amd64_register_preallocation_type_t type,
                                                       kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));

    const struct kefir_asmcmp_virtual_register *vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&target->context, vreg_idx, &vreg));

    struct register_preallocation *preallocation = NULL;
    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&target->register_preallocation, (kefir_hashtree_key_t) vreg_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        preallocation = KEFIR_MALLOC(mem, sizeof(struct register_preallocation));
        REQUIRE(preallocation != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register preallocation"));

        preallocation->type = type;
        preallocation->reg = reg;
        res = kefir_hashtree_insert(mem, &target->register_preallocation, (kefir_hashtree_key_t) vreg_idx,
                                    (kefir_hashtree_value_t) preallocation);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, preallocation);
            return res;
        });
    } else {
        REQUIRE_OK(res);
        preallocation = (struct register_preallocation *) node->value;

        switch (preallocation->type) {
            case KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT:
                if (type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT) {
                    preallocation->type = KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT;
                    preallocation->reg = reg;
                }
                break;

            case KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT:
                REQUIRE(
                    type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT || reg == preallocation->reg,
                    KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Conflicting amd64 register preallocation requirements"));
                break;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_get_register_preallocation(
    const struct kefir_asmcmp_amd64 *target, kefir_asmcmp_virtual_register_index_t vreg_idx,
    kefir_asmcmp_amd64_register_preallocation_type_t *preallocaton_type,
    kefir_asm_amd64_xasmgen_register_t *preallocation_reg) {
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmgen amd64 target"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&target->register_preallocation, (kefir_hashtree_key_t) vreg_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested amd64 register preallocation");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(struct register_preallocation *, preallocation, node->value);

    ASSIGN_PTR(preallocaton_type, preallocation->type);
    ASSIGN_PTR(preallocation_reg, preallocation->reg);
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

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER: {
            const struct kefir_asmcmp_virtual_register *vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&target->context, value->vreg.index, &vreg));
            switch (register_allocation->allocations[value->vreg.index].type) {
                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unallocated amd64 virtual register");

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER: {
                    kefir_asm_amd64_xasmgen_register_t reg =
                        register_allocation->allocations[value->vreg.index].direct_reg;
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
                    return KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED,
                                           "Spill area register allocation is not implemented yet");
            }
        } break;
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

        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link):
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(xasmgen,
                                                   "Vreg link vreg%" KEFIR_UINT64_FMT " <-> vreg%" KEFIR_UINT64_FMT,
                                                   instr->args[0].vreg.index, instr->args[1].vreg.index));
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
