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

#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/xregalloc.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/target/asm/amd64/xasmgen.h"
#include "kefir/core/string_builder.h"
#include <string.h>

#define BUFLEN 256
struct instruction_argument_state {
    struct kefir_asm_amd64_xasmgen_operand base_operands[4];
    const struct kefir_asm_amd64_xasmgen_operand *operand;
    char buf[BUFLEN];
};

static kefir_asm_amd64_xasmgen_symbol_relocation_t symbol_type_for_label(
    kefir_asmcmp_external_label_relocation_t type) {
    switch (type) {
        case KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE:
            return KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_PLT:
            return KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_PLT;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_GOTPCREL:
            return KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_GOTPCREL;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_TPOFF:
            return KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_TPOFF;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_GOTTPOFF:
            return KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_GOTTPOFF;

        case KEFIR_ASMCMP_EXTERNAL_LABEL_TLSGD:
            return KEFIR_AMD64_XASMGEN_SYMBOL_RELATIVE_TLSGD;
    }
    return KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE;
}

static kefir_result_t build_operand(const struct kefir_asmcmp_amd64 *target,
                                    const struct kefir_codegen_amd64_stack_frame *stack_frame,
                                    const struct kefir_asmcmp_value *value,
                                    struct instruction_argument_state *arg_state,
                                    kefir_asmcmp_operand_variant_t default_variant_override) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 asmcmp value type");

        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
            arg_state->operand =
                kefir_asm_amd64_xasmgen_operand_imm(&arg_state->base_operands[0], value->int_immediate);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
            arg_state->operand =
                kefir_asm_amd64_xasmgen_operand_immu(&arg_state->base_operands[1], value->uint_immediate);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            arg_state->operand = kefir_asm_amd64_xasmgen_operand_reg((kefir_asm_amd64_xasmgen_register_t) value->phreg);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register");

        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
            snprintf(arg_state->buf, BUFLEN, KEFIR_AMD64_LABEL, target->function_name, value->internal_label);
            arg_state->operand = kefir_asm_amd64_xasmgen_operand_label(
                &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE, arg_state->buf);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
            arg_state->operand = kefir_asm_amd64_xasmgen_operand_label(
                &arg_state->base_operands[0], symbol_type_for_label(value->external_label.position),
                value->external_label.symbolic);
            if (value->external_label.offset != 0) {
                arg_state->operand = kefir_asm_amd64_xasmgen_operand_offset(
                    &arg_state->base_operands[1], arg_state->operand, value->external_label.offset);
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_X87:
            arg_state->operand = kefir_asm_amd64_xasmgen_operand_fpu_register(&arg_state->base_operands[0], value->x87);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT: {
            const struct kefir_asm_amd64_xasmgen_operand *base_ptr = NULL;

            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register");

                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0], kefir_asm_amd64_xasmgen_operand_reg(value->indirect.base.phreg),
                        value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS:
                    snprintf(arg_state->buf, BUFLEN, KEFIR_AMD64_LABEL, target->function_name,
                             value->indirect.base.internal_label);
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(&arg_state->base_operands[1],
                                                              KEFIR_AMD64_XASMGEN_SYMBOL_ABSOLUTE, arg_state->buf),
                        value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(&arg_state->base_operands[1],
                                                              symbol_type_for_label(value->indirect.base.external_type),
                                                              value->indirect.base.external_label),
                        value->indirect.offset);
                    break;

                case KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS: {
                    kefir_int64_t offset;
                    REQUIRE_OK(kefir_codegen_amd64_stack_frame_local_variable_offset(stack_frame, value->indirect.base.local_variable_id, &offset));
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        stack_frame->offsets.local_area + offset + value->indirect.offset);
                } break;

                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    base_ptr = kefir_asm_amd64_xasmgen_operand_indirect(
                        &arg_state->base_operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        stack_frame->offsets.spill_area + value->indirect.base.spill_index * KEFIR_AMD64_ABI_QWORD +
                            value->indirect.offset);
                    break;
            }

            kefir_asmcmp_operand_variant_t variant = value->indirect.variant;
            if (variant == KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT) {
                variant = default_variant_override;
            }
            switch (variant) {
                case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[2], KEFIR_AMD64_XASMGEN_POINTER_BYTE, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[2], KEFIR_AMD64_XASMGEN_POINTER_WORD, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[2], KEFIR_AMD64_XASMGEN_POINTER_DWORD, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[2], KEFIR_AMD64_XASMGEN_POINTER_QWORD, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_80BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[2], KEFIR_AMD64_XASMGEN_POINTER_TBYTE, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_128BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[2], KEFIR_AMD64_XASMGEN_POINTER_XMMWORD, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[2], KEFIR_AMD64_XASMGEN_POINTER_FP_SINGLE, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[2], KEFIR_AMD64_XASMGEN_POINTER_FP_DOUBLE, base_ptr);
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                    arg_state->operand = base_ptr;
                    break;
            }
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL: {
            const char *base_label = NULL;
            if (value->type == KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL) {
                snprintf(arg_state->buf, BUFLEN, KEFIR_AMD64_LABEL, target->function_name,
                         value->rip_indirection.internal);
                base_label = arg_state->buf;
            } else {
                base_label = value->rip_indirection.external;
            }

            kefir_asm_amd64_xasmgen_symbol_relocation_t symbol_type =
                symbol_type_for_label(value->rip_indirection.position);

            kefir_asmcmp_operand_variant_t variant = value->rip_indirection.variant;
            if (variant == KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT) {
                variant = default_variant_override;
            }
            switch (variant) {
                case KEFIR_ASMCMP_OPERAND_VARIANT_8BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_BYTE,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1], symbol_type,
                                                                        base_label));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_16BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_WORD,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1], symbol_type,
                                                                        base_label));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_32BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1], symbol_type,
                                                                        base_label));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_64BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1], symbol_type,
                                                                        base_label));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_80BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1], symbol_type,
                                                                        base_label));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_128BIT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_XMMWORD,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1], symbol_type,
                                                                        base_label));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_SINGLE,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1], symbol_type,
                                                                        base_label));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_pointer(
                        &arg_state->base_operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_DOUBLE,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[1], symbol_type,
                                                                        base_label));
                    break;

                case KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT:
                    arg_state->operand = kefir_asm_amd64_xasmgen_operand_rip_indirection(&arg_state->base_operands[0],
                                                                                         symbol_type, base_label);
                    break;
            }
        } break;
    }

    if (value->segment.present) {
        arg_state->operand = kefir_asm_amd64_xasmgen_operand_segment(
            &arg_state->base_operands[3], (kefir_asm_amd64_xasmgen_segment_register_t) value->segment.reg,
            arg_state->operand);
    }

    return KEFIR_OK;
}

static kefir_result_t format_inline_assembly(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen,
                                             const struct kefir_asmcmp_amd64 *target,
                                             const struct kefir_codegen_amd64_stack_frame *stack_frame,
                                             kefir_asmcmp_inline_assembly_index_t inline_asm_idx) {
    struct kefir_string_builder builder;
    REQUIRE_OK(kefir_string_builder_init(&builder));
    kefir_result_t res = KEFIR_OK;

    struct instruction_argument_state arg_state;
    struct kefir_asmcmp_inline_assembly_fragment_iterator iter;
    char buffer[256];
    for (res = kefir_asmcmp_inline_assembly_fragment_iter(&target->context, inline_asm_idx, &iter);
         res == KEFIR_OK && iter.fragment != NULL; res = kefir_asmcmp_inline_assembly_fragment_next(&iter)) {

        switch (iter.fragment->type) {
            case KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_TEXT:
                REQUIRE_CHAIN(&res, kefir_string_builder_printf(mem, &builder, "%s", iter.fragment->text));
                break;

            case KEFIR_ASMCMP_INLINE_ASSEMBLY_FRAGMENT_VALUE:
                REQUIRE_CHAIN(&res, build_operand(target, stack_frame, &iter.fragment->value, &arg_state,
                                                  KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT));
                REQUIRE_CHAIN(
                    &res, KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(xasmgen, arg_state.operand, buffer, sizeof(buffer) - 1));
                REQUIRE_CHAIN(&res, kefir_string_builder_printf(mem, &builder, "%s", buffer));
                break;
        }

        if (res != KEFIR_OK) {
            break;
        }
    }

    REQUIRE_CHAIN(&res, KEFIR_AMD64_XASMGEN_INLINE_ASSEMBLY(xasmgen, builder.string));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_builder_free(mem, &builder);
        return res;
    });
    REQUIRE_OK(kefir_string_builder_free(mem, &builder));
    return KEFIR_OK;
}

static kefir_bool_t same_operands(const struct kefir_asmcmp_value *arg1, const struct kefir_asmcmp_value *arg2) {
    REQUIRE(arg1->type == arg2->type, false);
    REQUIRE((!arg1->segment.present && !arg2->segment.present) ||
                (arg1->segment.present && arg2->segment.present && arg1->segment.reg == arg2->segment.reg),
            false);

    switch (arg1->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
            return true;

        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
            return arg1->int_immediate == arg2->int_immediate;

        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
            return arg1->uint_immediate == arg2->uint_immediate;

        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            return arg1->phreg == arg2->phreg;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            return arg1->vreg.index == arg2->vreg.index && arg1->vreg.variant == arg2->vreg.variant;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            REQUIRE(arg1->indirect.type == arg2->indirect.type, false);
            REQUIRE(arg1->indirect.offset == arg2->indirect.offset, false);
            REQUIRE(arg1->indirect.variant == arg2->indirect.variant, false);
            switch (arg1->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    return arg1->indirect.base.phreg == arg2->indirect.base.phreg;

                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    return arg1->indirect.base.vreg == arg2->indirect.base.vreg;

                case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS:
                    return arg1->indirect.base.internal_label == arg2->indirect.base.internal_label;

                case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                    return strcmp(arg1->indirect.base.external_label, arg2->indirect.base.external_label) == 0;

                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                    return arg1->indirect.base.spill_index == arg2->indirect.base.spill_index;

                case KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS:
                    return true;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
            return arg1->rip_indirection.internal == arg2->rip_indirection.internal &&
                   arg1->rip_indirection.variant == arg2->rip_indirection.variant;

        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
            return strcmp(arg1->rip_indirection.external, arg2->rip_indirection.external) == 0 &&
                   arg1->rip_indirection.variant == arg2->rip_indirection.variant;

        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
            return arg1->internal_label == arg2->internal_label;

        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
            return strcmp(arg1->external_label.symbolic, arg2->external_label.symbolic) == 0 &&
                   arg1->external_label.offset == arg2->external_label.offset;

        case KEFIR_ASMCMP_VALUE_TYPE_X87:
            return arg1->x87 == arg2->x87;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
            return arg1->stash_idx == arg2->stash_idx;

        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            return arg1->inline_asm_idx == arg2->inline_asm_idx;
    }
    return false;
}

static kefir_result_t generate_instr(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen,
                                     const struct kefir_asmcmp_amd64 *target,
                                     const struct kefir_codegen_amd64_stack_frame *stack_frame,
                                     kefir_asmcmp_instruction_index_t index) {
    struct kefir_asmcmp_instruction *instr;
    REQUIRE_OK(kefir_asmcmp_context_instr_at(&target->context, index, &instr));
    for (kefir_asmcmp_label_index_t label = kefir_asmcmp_context_instr_label_head(&target->context, index);
         label != KEFIR_ASMCMP_INDEX_NONE; label = kefir_asmcmp_context_instr_label_next(&target->context, label)) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, KEFIR_AMD64_LABEL, target->function_name, label));

        kefir_result_t res;
        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&target->context.labels[label].public_labels, &iter); res == KEFIR_OK;
             res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(const char *, symbolic_label, iter.entry);
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(xasmgen, "%s", symbolic_label));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    struct instruction_argument_state arg_state[3] = {0};
    switch (instr->opcode) {
#define DEF_OPCODE0_(_opcode)                        \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):         \
        REQUIRE_OK(xasmgen->instr._opcode(xasmgen)); \
        break;
#define DEF_OPCODE0_REPEATABLE(_opcode)                    \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):               \
        REQUIRE_OK(xasmgen->instr._opcode(xasmgen, true)); \
        break;
#define DEF_OPCODE0_PREFIX(_opcode) DEF_OPCODE0_(_opcode)
#define DEF_OPCODE0(_opcode, _mnemonic, _variant, _flag) DEF_OPCODE0_##_variant(_opcode)
#define DEF_OPCODE1(_opcode, _mnemonic, _variant, _flag, _op1)                                                         \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                           \
        REQUIRE_OK(                                                                                                    \
            build_operand(target, stack_frame, &instr->args[0], &arg_state[0], KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT)); \
        REQUIRE_OK(xasmgen->instr._opcode(xasmgen, arg_state[0].operand));                                             \
        break;
#define DEF_OPCODE2(_opcode, _mnemonic, _variant, _flag, _op1, _op2)                                                   \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                           \
        REQUIRE_OK(                                                                                                    \
            build_operand(target, stack_frame, &instr->args[0], &arg_state[0], KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT)); \
        REQUIRE_OK(                                                                                                    \
            build_operand(target, stack_frame, &instr->args[1], &arg_state[1], KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT)); \
        REQUIRE_OK(xasmgen->instr._opcode(xasmgen, arg_state[0].operand, arg_state[1].operand));                       \
        break;
#define DEF_OPCODE3(_opcode, _mnemonic, _variant, _flag, _op1, _op2, _op3)                                             \
    case KEFIR_ASMCMP_AMD64_OPCODE(_opcode):                                                                           \
        REQUIRE_OK(                                                                                                    \
            build_operand(target, stack_frame, &instr->args[0], &arg_state[0], KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT)); \
        REQUIRE_OK(                                                                                                    \
            build_operand(target, stack_frame, &instr->args[1], &arg_state[1], KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT)); \
        REQUIRE_OK(                                                                                                    \
            build_operand(target, stack_frame, &instr->args[2], &arg_state[2], KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT)); \
        REQUIRE_OK(xasmgen->instr._opcode(xasmgen, arg_state[0].operand, arg_state[1].operand, arg_state[2].operand)); \
        break;

        KEFIR_AMD64_INSTRUCTION_DATABASE(DEF_OPCODE0, DEF_OPCODE1, DEF_OPCODE2, DEF_OPCODE3, )

#undef DEF_OPCODE0_REPEATABLE
#undef DEF_OPCODE0_PREFIX
#undef DEF_OPCODE0_
#undef DEF_OPCODE0
#undef DEF_OPCODE1
#undef DEF_OPCODE2
#undef DEF_OPCODE3

        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_register_link):
            if (!same_operands(&instr->args[0], &instr->args[1])) {
                REQUIRE_OK(build_operand(target, stack_frame, &instr->args[0], &arg_state[0],
                                         KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT));
                REQUIRE_OK(build_operand(target, stack_frame, &instr->args[1], &arg_state[1],
                                         KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT));
                if ((arg_state[0].operand->klass == KEFIR_AMD64_XASMGEN_OPERAND_REGISTER &&
                     kefir_asm_amd64_xasmgen_register_is_floating_point(arg_state[0].operand->reg)) ||
                    (arg_state[1].operand->klass == KEFIR_AMD64_XASMGEN_OPERAND_REGISTER &&
                     kefir_asm_amd64_xasmgen_register_is_floating_point(arg_state[1].operand->reg))) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(xasmgen, arg_state[0].operand, arg_state[1].operand));
                } else {
                    REQUIRE_OK(build_operand(target, stack_frame, &instr->args[0], &arg_state[0],
                                             KEFIR_ASMCMP_OPERAND_VARIANT_64BIT));
                    REQUIRE_OK(build_operand(target, stack_frame, &instr->args[1], &arg_state[1],
                                             KEFIR_ASMCMP_OPERAND_VARIANT_64BIT));
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(xasmgen, arg_state[0].operand, arg_state[1].operand));
                }
            }
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(function_prologue):
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_prologue(xasmgen, target->abi_variant, stack_frame,
                                                                target->function_name));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(function_epilogue):
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_epilogue(xasmgen, target->abi_variant, stack_frame));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(tail_call):
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_epilogue(xasmgen, target->abi_variant, stack_frame));
            REQUIRE_OK(build_operand(target, stack_frame, &instr->args[0], &arg_state[0],
                                     KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(xasmgen, arg_state[0].operand));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(data_word):
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, 1,
                kefir_asm_amd64_xasmgen_operand_immu(&arg_state[0].base_operands[0], instr->args[0].uint_immediate)));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(inline_assembly):
            REQUIRE_OK(format_inline_assembly(mem, xasmgen, target, stack_frame, instr->args[0].inline_asm_idx));
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(touch_virtual_register):
        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_begin):
        case KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_end):
        case KEFIR_ASMCMP_AMD64_OPCODE(noop):
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_AMD64_OPCODE(stash_activate):
        case KEFIR_ASMCMP_AMD64_OPCODE(stash_deactivate):
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected stash (de-)activation instruction");
    }

    return KEFIR_OK;
}

kefir_result_t kefir_asmcmp_amd64_generate_code(struct kefir_mem *mem, struct kefir_amd64_xasmgen *xasmgen,
                                                kefir_amd64_xasmgen_debug_info_tracker_t debug_info_tracker,
                                                const struct kefir_asmcmp_amd64 *target,
                                                const struct kefir_codegen_amd64_stack_frame *stack_frame) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xasmgen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 assembly generator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid asmcmp amd64 target"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));

    struct kefir_hashtreeset_iterator external_iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&target->externals, &external_iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&external_iter)) {
        ASSIGN_DECL_CAST(const char *, external, external_iter.entry);
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(xasmgen, "%s", external));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    const struct kefir_source_location *last_source_location = NULL;
    for (kefir_asmcmp_instruction_index_t idx = kefir_asmcmp_context_instr_head(&target->context);
         idx != KEFIR_ASMCMP_INDEX_NONE; idx = kefir_asmcmp_context_instr_next(&target->context, idx)) {

        if (debug_info_tracker != NULL) {
            const struct kefir_source_location *source_location = NULL;
            res = kefir_asmcmp_source_map_at(&target->context.debug_info.source_map, idx, &source_location);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                if (last_source_location != source_location) {
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DEBUG_INFO_SOURCE_LOCATION(mem, xasmgen, debug_info_tracker,
                                                                              source_location));
                }
                last_source_location = source_location;
            } else {
                last_source_location = NULL;
            }
        }

        REQUIRE_OK(generate_instr(mem, xasmgen, target, stack_frame, idx));
    }

    return KEFIR_OK;
}
