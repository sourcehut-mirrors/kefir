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

#include "kefir/codegen/opt-system-v-amd64/inline_assembly.h"
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t pointer_operand(
    const struct kefir_ir_inline_assembly_parameter *asm_param,
    struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry,
    const struct kefir_asm_amd64_xasmgen_operand *base, struct kefir_asm_amd64_xasmgen_operand *new_op,
    const struct kefir_asm_amd64_xasmgen_operand **result, kefir_size_t override_size) {

    struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(asm_param->type.type, asm_param->type.index);
    kefir_size_t param_size = override_size == 0 ? entry->parameter_props.size : override_size;

    if (override_size == 0 &&
        entry->parameter_props.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_AGGREGATE &&
        typeentry->typecode != KEFIR_IR_TYPE_LONG_DOUBLE) {
        *result = base;
        return KEFIR_OK;
    }

    if (param_size <= 1) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_BYTE, base);
    } else if (param_size <= 2) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_WORD, base);
    } else if (param_size <= 4) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_DWORD, base);
    } else if (param_size <= 8) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_QWORD, base);
    } else if (override_size == 10 || typeentry->typecode == KEFIR_IR_TYPE_LONG_DOUBLE) {
        *result = kefir_asm_amd64_xasmgen_operand_pointer(new_op, KEFIR_AMD64_XASMGEN_POINTER_TBYTE, base);
    } else {
        *result = base;
    }
    return KEFIR_OK;
}

static kefir_result_t match_parameter(const struct kefir_ir_inline_assembly *inline_asm, const char **input_str,
                                      const struct kefir_ir_inline_assembly_parameter **asm_param_ptr,
                                      const struct kefir_ir_inline_assembly_jump_target **jump_target_ptr) {
    *asm_param_ptr = NULL;
    *jump_target_ptr = NULL;
    struct kefir_hashtree_node_iterator iter;
    kefir_size_t last_match_length = 0;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->parameters, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, node->value);

        kefir_size_t template_parameter_length = strlen(identifier);
        if (template_parameter_length > last_match_length &&
            strncmp(*input_str, identifier, template_parameter_length) == 0) {
            *asm_param_ptr = asm_param;
            last_match_length = template_parameter_length;
        }
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_jump_target *, jump_target, node->value);

        kefir_size_t identifier_length = strlen(identifier);
        if (identifier_length > last_match_length && strncmp(*input_str, identifier, identifier_length) == 0) {
            *asm_param_ptr = NULL;
            *jump_target_ptr = jump_target;
            last_match_length = identifier_length;
        }
    }

    if (last_match_length == 0) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Inline assembly parameter not found");
    } else {
        *input_str += last_match_length;
        return KEFIR_OK;
    }
}

static kefir_result_t format_label_parameter(struct kefir_mem *mem,
                                             struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                             const struct kefir_ir_inline_assembly_jump_target *jump_target) {
    char jump_target_text[1024];
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(
        &context->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(
            &context->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_helpers_format(
                &context->codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_INLINE_ASSEMBLY_JUMP_TRAMPOLINE_LABEL,
                context->function->ir_func->name, context->inline_assembly->node_id, jump_target->uid)),
        jump_target_text, sizeof(jump_target_text) - 1));

    REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%s", jump_target_text));
    return KEFIR_OK;
}

static kefir_result_t format_normal_parameter(struct kefir_mem *mem,
                                              struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                              const struct kefir_ir_inline_assembly_parameter *asm_param,
                                              kefir_size_t override_size) {
    if (asm_param->klass == KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE) {
        char register_symbol[128];
        const struct kefir_asm_amd64_xasmgen_operand *operand = NULL;
        switch (asm_param->immediate_type) {
            case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED:
                if (asm_param->immediate_identifier_base != NULL) {
                    operand = kefir_asm_amd64_xasmgen_operand_offset(
                        &context->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(&context->codegen->xasmgen_helpers.operands[1],
                                                              asm_param->immediate_identifier_base),
                        asm_param->immediate_value);
                } else {
                    operand = kefir_asm_amd64_xasmgen_operand_imm(&context->codegen->xasmgen_helpers.operands[0],
                                                                  asm_param->immediate_value);
                }
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_LITERAL_BASED:
                operand = kefir_asm_amd64_xasmgen_operand_offset(
                    &context->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &context->codegen->xasmgen_helpers.operands[1],
                        kefir_asm_amd64_xasmgen_helpers_format(&context->codegen->xasmgen_helpers,
                                                               KEFIR_OPT_AMD64_SYSTEM_V_STRING_LITERAL,
                                                               asm_param->immediate_identifier_base)),
                    asm_param->immediate_value);
                break;
        }
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(&context->codegen->xasmgen, operand, register_symbol,
                                                      sizeof(register_symbol) - 1));
        REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%s", register_symbol));
        return KEFIR_OK;
    }

    struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry *entry =
        &context->parameter_allocation[asm_param->parameter_id];

    kefir_size_t operand_size = override_size == 0 ? entry->parameter_props.size : override_size;

    switch (entry->allocation_type) {
        case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER: {
            kefir_asm_amd64_xasmgen_register_t reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_match_register_to_size(entry->allocation.reg,
                                                                                           operand_size, &reg));

            char register_symbol[64];
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(&context->codegen->xasmgen,
                                                          kefir_asm_amd64_xasmgen_operand_reg(reg), register_symbol,
                                                          sizeof(register_symbol) - 1));
            REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%s", register_symbol));
        } break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_REGISTER_INDIRECT: {
            char register_symbol[128];
            const struct kefir_asm_amd64_xasmgen_operand *operand = NULL;
            REQUIRE_OK(pointer_operand(
                asm_param, entry,
                kefir_asm_amd64_xasmgen_operand_indirect(&context->codegen->xasmgen_helpers.operands[0],
                                                         kefir_asm_amd64_xasmgen_operand_reg(entry->allocation.reg), 0),
                &context->codegen->xasmgen_helpers.operands[1], &operand, override_size));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(&context->codegen->xasmgen, operand, register_symbol,
                                                          sizeof(register_symbol) - 1));
            REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%s", register_symbol));
        } break;

        case KEFIR_CODEGEN_OPT_SYSV_AMD64_INLINE_ASSEMBLY_PARAMETER_ALLOCATION_STACK: {
            kefir_int64_t offset =
                context->stack_map.input_parameter_offset + entry->allocation.stack.index * KEFIR_AMD64_ABI_QWORD;
            char register_symbol[128];
            const struct kefir_asm_amd64_xasmgen_operand *operand = NULL;
            REQUIRE_OK(pointer_operand(
                asm_param, entry,
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &context->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(context->stack_input_parameters.base_register), offset),
                &context->codegen->xasmgen_helpers.operands[1], &operand, override_size));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_FORMAT_OPERAND(&context->codegen->xasmgen, operand, register_symbol,
                                                          sizeof(register_symbol) - 1));
            REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%s", register_symbol));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t default_param_modifier(struct kefir_mem *mem,
                                             struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                             const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, context, asm_param, 0));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t byte_param_modifier(struct kefir_mem *mem,
                                          struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                          const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, context, asm_param, 1));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t word_param_modifier(struct kefir_mem *mem,
                                          struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                          const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, context, asm_param, 2));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t dword_param_modifier(struct kefir_mem *mem,
                                           struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                           const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, context, asm_param, 4));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t qword_param_modifier(struct kefir_mem *mem,
                                           struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                           const char **input_str) {
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    *input_str = input;
    if (asm_param != NULL) {
        REQUIRE_OK(format_normal_parameter(mem, context, asm_param, 8));
    } else if (jump_target != NULL) {
        REQUIRE_OK(format_label_parameter(mem, context, jump_target));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result");
    }
    return KEFIR_OK;
}

static kefir_result_t label_param_modifier(struct kefir_mem *mem,
                                           struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context,
                                           const char **input_str) {
    UNUSED(mem);
    const char *input = *input_str;
    const struct kefir_ir_inline_assembly_parameter *asm_param;
    const struct kefir_ir_inline_assembly_jump_target *jump_target;
    REQUIRE_OK(match_parameter(context->ir_inline_assembly, &input, &asm_param, &jump_target));
    REQUIRE(asm_param == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                            "Inline assembly label parameter modifier cannot be applied to register parameters"));
    REQUIRE(jump_target != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected parameter matching result"));
    *input_str = input;
    REQUIRE_OK(format_label_parameter(mem, context, jump_target));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_format(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen inline assembly context"));

    kefir_id_t id = context->codegen_func->inline_asm_id++;

    const char *template_iter = context->ir_inline_assembly->template;
    while (template_iter != NULL) {
        const char *format_specifier = strchr(template_iter, '%');
        if (format_specifier == NULL) {
            REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%s", template_iter));
            template_iter = NULL;
        } else {
            REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%.*s",
                                                   format_specifier - template_iter, template_iter));
            switch (format_specifier[1]) {
                case '%':
                case '{':
                case '|':
                case '}':
                    REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%.1s", format_specifier + 1));
                    template_iter = format_specifier + 2;
                    break;

                case '=':
                    REQUIRE_OK(kefir_string_builder_printf(mem, &context->formatted_asm, "%" KEFIR_ID_FMT, id));
                    template_iter = format_specifier + 2;
                    break;

                case 'l': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = label_param_modifier(mem, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &context->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'b': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = byte_param_modifier(mem, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &context->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'w': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = word_param_modifier(mem, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &context->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'd': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = dword_param_modifier(mem, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &context->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                case 'q': {
                    template_iter = format_specifier + 2;
                    kefir_result_t res = qword_param_modifier(mem, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        template_iter = format_specifier + 1;
                        res = default_param_modifier(mem, context, &template_iter);
                    }
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &context->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;

                default: {
                    template_iter = format_specifier + 1;
                    kefir_result_t res = default_param_modifier(mem, context, &template_iter);
                    if (res == KEFIR_NOT_FOUND) {
                        res = kefir_string_builder_printf(mem, &context->formatted_asm, "%%");
                    }
                    REQUIRE_OK(res);
                } break;
            }
        }
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INLINE_ASSEMBLY(&context->codegen->xasmgen, context->formatted_asm.string));
    return KEFIR_OK;
}
