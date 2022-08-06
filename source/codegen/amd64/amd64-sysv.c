/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "kefir/codegen/amd64-sysv.h"
#include "kefir/codegen/amd64/opcodes.h"
#include "kefir/codegen/amd64/labels.h"
#include "kefir/codegen/amd64/tls.h"
#include "kefir/codegen/amd64/system-v/abi.h"
#include "kefir/codegen/amd64/system-v/runtime.h"
#include "kefir/codegen/amd64/system-v/instr.h"
#include "kefir/codegen/amd64/system-v/inline_assembly.h"
#include "kefir/codegen/amd64/system-v/abi/tls.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t cg_declare_opcode_handler(kefir_iropcode_t opcode, const char *handler, void *payload) {
    UNUSED(opcode);
    struct kefir_amd64_xasmgen *xasmgen = (struct kefir_amd64_xasmgen *) payload;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(xasmgen, "%s", handler));
    return KEFIR_OK;
}

static kefir_result_t cg_module_globals(struct kefir_codegen_amd64_sysv_module *module,
                                        struct kefir_codegen_amd64 *codegen) {
    struct kefir_hashtree_node_iterator globals_iter;
    kefir_ir_identifier_type_t global_type;
    for (const char *global = kefir_ir_module_globals_iter(module->module, &globals_iter, &global_type); global != NULL;
         global = kefir_ir_module_globals_iter_next(&globals_iter, &global_type)) {

        if (!codegen->config->emulated_tls || global_type != KEFIR_IR_IDENTIFIER_THREAD_LOCAL) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, "%s", global));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, KEFIR_AMD64_EMUTLS_V, global));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_GLOBAL(&codegen->xasmgen, KEFIR_AMD64_EMUTLS_T, global));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t cg_module_externals(struct kefir_codegen_amd64_sysv_module *module,
                                          struct kefir_codegen_amd64 *codegen) {
    struct kefir_hashtree_node_iterator externals_iter;
    kefir_ir_identifier_type_t external_type;
    for (const char *external = kefir_ir_module_externals_iter(module->module, &externals_iter, &external_type);
         external != NULL; external = kefir_ir_module_externals_iter_next(&externals_iter, &external_type)) {

        if (!codegen->config->emulated_tls || external_type != KEFIR_IR_IDENTIFIER_THREAD_LOCAL) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", external));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, KEFIR_AMD64_EMUTLS_V, external));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t cg_module_prologue(struct kefir_codegen_amd64_sysv_module *module,
                                         struct kefir_codegen_amd64 *codegen) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_PROLOGUE(&codegen->xasmgen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".text"));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%s", "Opcode handlers"));
    REQUIRE_OK(kefir_amd64_iropcode_handler_list(cg_declare_opcode_handler, &codegen->xasmgen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%s", "Runtime functions"));
    for (kefir_size_t i = 0; i < KEFIR_AMD64_SYSTEM_V_RUNTIME_SYMBOL_COUNT; i++) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_EXTERNAL(&codegen->xasmgen, "%s", KEFIR_AMD64_SYSTEM_V_RUNTIME_SYMBOLS[i]));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%s", "Externals"));
    REQUIRE_OK(cg_module_externals(module, codegen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%s", "Globals"));
    REQUIRE_OK(cg_module_globals(module, codegen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    return KEFIR_OK;
}

static kefir_result_t cg_function_prologue(struct kefir_codegen_amd64 *codegen,
                                           const struct kefir_amd64_sysv_function *func) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSV_PROCEDURE_LABEL, func->func->name));
    REQUIRE_OK(kefir_amd64_sysv_function_prologue(codegen, func));
    return KEFIR_OK;
}

static kefir_result_t cg_function_epilogue(struct kefir_codegen_amd64 *codegen,
                                           const struct kefir_amd64_sysv_function *func) {
    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSV_PROCEDURE_EPILOGUE_LABEL, func->func->name));
    REQUIRE_OK(kefir_amd64_sysv_function_epilogue(codegen, func));
    return KEFIR_OK;
}

#define MAX_LABEL_LEN 1024

static kefir_result_t cg_function_body(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                       struct kefir_codegen_amd64_sysv_module *sysv_module,
                                       struct kefir_amd64_sysv_function *sysv_func) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_PROGRAM_REG),
        kefir_amd64_xasmgen_operand_indirect(
            &codegen->xasmgen_helpers.operands[0],
            kefir_amd64_xasmgen_operand_label(
                &codegen->xasmgen_helpers.operands[1],
                kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_SYSV_PROCEDURE_BODY_LABEL,
                                                   sysv_func->func->name)),
            0)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_PROGRAM_REG), 0)));
    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSV_PROCEDURE_BODY_LABEL, sysv_func->func->name));
    kefir_size_t instr_width;
    for (kefir_size_t pc = 0; pc < kefir_irblock_length(&sysv_func->func->body); pc += instr_width) {
        REQUIRE_OK(kefir_amd64_sysv_instruction(mem, codegen, sysv_func, sysv_module, &sysv_func->func->body, pc,
                                                &instr_width));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
        kefir_amd64_xasmgen_operand_label(
            &codegen->xasmgen_helpers.operands[0],
            kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_SYSV_PROCEDURE_EPILOGUE_LABEL,
                                               sysv_func->func->name)),
        kefir_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[1], 0)));
    return KEFIR_OK;
}

struct function_translator {
    struct kefir_codegen_amd64 *codegen;
};

static kefir_result_t cg_translate_function(struct kefir_mem *mem, const struct kefir_ir_function *func,
                                            struct kefir_codegen_amd64_sysv_module *sysv_module,
                                            struct kefir_codegen_amd64 *codegen) {
    struct kefir_amd64_sysv_function sysv_func;
    REQUIRE_OK(kefir_amd64_sysv_function_alloc(mem, sysv_module, func, &sysv_func));
    kefir_result_t res = cg_function_prologue(codegen, &sysv_func);
    REQUIRE_CHAIN(&res, cg_function_body(mem, codegen, sysv_module, &sysv_func));
    REQUIRE_CHAIN(&res, cg_function_epilogue(codegen, &sysv_func));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_amd64_sysv_function_free(mem, &sysv_func);
        return res;
    });

    struct kefir_hashtree_node_iterator appendix_iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&sysv_func.appendix, &appendix_iter);
         res == KEFIR_OK && node != NULL; node = kefir_hashtree_next(&appendix_iter)) {
        ASSIGN_DECL_CAST(struct kefir_amd64_sysv_function_appendix_data *, appendix, node->value);
        REQUIRE_CHAIN(
            &res, appendix->callback(codegen, sysv_module, &sysv_func, (const char *) node->key, appendix->payload));
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_amd64_sysv_function_free(mem, &sysv_func);
        return res;
    });

    REQUIRE_OK(kefir_amd64_sysv_function_free(mem, &sysv_func));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    return KEFIR_OK;
}

static kefir_result_t cg_translate_function_gates(struct kefir_codegen_amd64 *codegen,
                                                  const struct kefir_hashtree *tree, bool virtualDecl) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(tree, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_amd64_sysv_function_decl *, sysv_decl, node->value);
        if (virtualDecl) {
            if (sysv_decl->decl->name == NULL) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSV_FUNCTION_VIRTUAL_GATE_ID_LABEL,
                                                     sysv_decl->decl->id));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(
                    &codegen->xasmgen, KEFIR_AMD64_SYSV_FUNCTION_VIRTUAL_GATE_NAMED_LABEL, sysv_decl->decl->name));
            }
        } else {
            if (sysv_decl->decl->name == NULL || sysv_decl->decl->vararg) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSV_FUNCTION_GATE_ID_LABEL,
                                                     sysv_decl->decl->id));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSV_FUNCTION_GATE_NAMED_LABEL,
                                                     sysv_decl->decl->name, sysv_decl->decl->id));
            }
        }
        REQUIRE_OK(kefir_amd64_sysv_function_invoke(codegen, sysv_decl, virtualDecl));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
            kefir_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_indirect(
                                   &codegen->xasmgen_helpers.operands[0],
                                   kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    }
    return KEFIR_OK;
}

static kefir_result_t cg_translate_tls_entries(struct kefir_codegen_amd64 *codegen, const struct kefir_hashtree *tree,
                                               const struct kefir_ir_module *module) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(tree, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, identifier, node->key);

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSV_FUNCTION_TLS_ENTRY, identifier));
        REQUIRE_OK(kefir_amd64_sysv_thread_local_reference(codegen, identifier,
                                                           !kefir_ir_module_has_external(module, identifier)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
            kefir_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_indirect(
                                   &codegen->xasmgen_helpers.operands[0],
                                   kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    }
    return KEFIR_OK;
}

static kefir_result_t cg_translate_strings(struct kefir_codegen_amd64 *codegen,
                                           struct kefir_codegen_amd64_sysv_module *module) {

    kefir_bool_t first = true;
    struct kefir_hashtree_node_iterator iter;
    kefir_id_t id;
    kefir_ir_string_literal_type_t literal_type;
    kefir_bool_t public;
    const void *content = NULL;
    kefir_size_t length = 0;
    kefir_result_t res = KEFIR_OK;
    for (res =
             kefir_ir_module_string_literal_iter(module->module, &iter, &id, &literal_type, &public, &content, &length);
         res == KEFIR_OK;
         res = kefir_ir_module_string_literal_next(&iter, &id, &literal_type, &public, &content, &length)) {
        if (!public) {
            continue;
        }
        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata"));
            first = false;
        }

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSTEM_V_RUNTIME_STRING_LITERAL, id));
        switch (literal_type) {
            case KEFIR_IR_STRING_LITERAL_MULTIBYTE:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_ASCII, 1,
                                                    kefir_amd64_xasmgen_operand_string_literal(
                                                        &codegen->xasmgen_helpers.operands[0], content, length)));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE16:
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_BINDATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_WORD, content, length));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE32:
                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_BINDATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, content, length));
                break;
        }
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    return KEFIR_OK;
}

static kefir_result_t cg_translate_data_storage(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                struct kefir_codegen_amd64_sysv_module *module,
                                                kefir_ir_data_storage_t storage, kefir_bool_t defined,
                                                const char *section) {
    bool first = true;
    struct kefir_hashtree_node_iterator iter;
    const char *identifier = NULL;
    for (const struct kefir_ir_data *data = kefir_ir_module_named_data_iter(module->module, &iter, &identifier);
         data != NULL; data = kefir_ir_module_named_data_next(&iter, &identifier)) {
        if (data->storage != storage) {
            continue;
        }
        if (data->defined != defined) {
            continue;
        }

        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, section));
            first = false;
        }
        REQUIRE_OK(kefir_amd64_sysv_static_data(mem, codegen, data, identifier));
    }
    return KEFIR_OK;
}

static kefir_result_t cg_translate_emulated_tls(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                struct kefir_codegen_amd64_sysv_module *module) {
    bool first = true;
    struct kefir_hashtree_node_iterator iter;
    const char *identifier = NULL;
    char emutls_identifier[1024] = {0};
    for (const struct kefir_ir_data *data = kefir_ir_module_named_data_iter(module->module, &iter, &identifier);
         data != NULL; data = kefir_ir_module_named_data_next(&iter, &identifier)) {
        if (data->storage != KEFIR_IR_DATA_THREAD_LOCAL_STORAGE) {
            continue;
        }

        if (!data->defined) {
            continue;
        }

        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".rodata"));
            first = false;
        }

        snprintf(emutls_identifier, sizeof(emutls_identifier) - 1, KEFIR_AMD64_EMUTLS_T, identifier);
        REQUIRE_OK(kefir_amd64_sysv_static_data(mem, codegen, data, emutls_identifier));
    }

    first = true;
    for (const struct kefir_ir_data *data = kefir_ir_module_named_data_iter(module->module, &iter, &identifier);
         data != NULL; data = kefir_ir_module_named_data_next(&iter, &identifier)) {
        if (data->storage != KEFIR_IR_DATA_THREAD_LOCAL_STORAGE) {
            continue;
        }

        if (first) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&codegen->xasmgen, ".data"));
            first = false;
        }

        char emutls_identifier[1024] = {0};
        snprintf(emutls_identifier, sizeof(emutls_identifier) - 1, KEFIR_AMD64_EMUTLS_V, identifier);

        struct kefir_vector type_layout;
        REQUIRE_OK(kefir_amd64_sysv_type_layout(data->type, mem, &type_layout));
        kefir_size_t total_size, total_alignment;
        kefir_result_t res =
            kefir_amd64_sysv_calculate_type_properties(data->type, &type_layout, &total_size, &total_alignment);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_vector_free(mem, &type_layout);
            return res;
        });
        REQUIRE_OK(kefir_vector_free(mem, &type_layout));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&codegen->xasmgen, 8));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", emutls_identifier));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
            &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 4,
            kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], total_size),
            kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], total_alignment),
            kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[2], 0),
            data->defined
                ? kefir_amd64_xasmgen_operand_label(
                      &codegen->xasmgen_helpers.operands[3],
                      kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_EMUTLS_T, identifier))
                : kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[3], 0)));
    }
    return KEFIR_OK;
}

static kefir_result_t cg_translate_data(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                        struct kefir_codegen_amd64_sysv_module *module) {
    REQUIRE_OK(cg_translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_GLOBAL_STORAGE, true, ".data"));
    REQUIRE_OK(cg_translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_GLOBAL_STORAGE, false, ".bss"));
    if (!codegen->config->emulated_tls) {
        REQUIRE_OK(cg_translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, true, ".tdata"));
        REQUIRE_OK(cg_translate_data_storage(mem, codegen, module, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, false, ".tbss"));
    } else {
        REQUIRE_OK(cg_translate_emulated_tls(mem, codegen, module));
    }
    REQUIRE_OK(cg_translate_strings(codegen, module));
    return KEFIR_OK;
}

static kefir_result_t cg_translate_global_inline_assembly(struct kefir_codegen_amd64 *codegen,
                                                          struct kefir_codegen_amd64_sysv_module *module) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->module->global_inline_asm, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly *, inline_asm, node->value);
        REQUIRE(kefir_hashtree_empty(&inline_asm->parameters),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Global IR inline assembly cannot have any parameters"));
        REQUIRE(kefir_hashtree_empty(&inline_asm->clobbers),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Global IR inline assembly cannot have any clobbers"));
        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "Inline assembly fragment #" KEFIR_ID_FMT, inline_asm->id));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INLINE_ASSEMBLY(&codegen->xasmgen, inline_asm->template));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    }
    return KEFIR_OK;
}

static kefir_result_t cg_translate_inline_assembly_fragments(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                             struct kefir_codegen_amd64_sysv_module *module) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->inline_assembly, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly *, inline_asm, node->value);

        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "Inline assembly fragment #" KEFIR_ID_FMT, inline_asm->id));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_FRAGMENT,
                                             inline_asm->id));
        REQUIRE_OK(kefir_codegen_amd64_sysv_inline_assembly_embed(mem, module, codegen, inline_asm));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
            kefir_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_indirect(
                                   &codegen->xasmgen_helpers.operands[0],
                                   kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_NEWLINE(&codegen->xasmgen, 1));
    }
    return KEFIR_OK;
}

static kefir_result_t cg_translate(struct kefir_mem *mem, struct kefir_codegen *cg_iface,
                                   struct kefir_ir_module *module) {
    REQUIRE(cg_iface != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator interface"));
    REQUIRE(cg_iface->data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    struct kefir_codegen_amd64 *codegen = (struct kefir_codegen_amd64 *) cg_iface->data;
    struct kefir_codegen_amd64_sysv_module sysv_module;
    REQUIRE_OK(kefir_codegen_amd64_sysv_module_alloc(mem, &sysv_module, module));
    REQUIRE_OK(cg_module_prologue(&sysv_module, codegen));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *func = kefir_ir_module_function_iter(sysv_module.module, &iter); func != NULL;
         func = kefir_ir_module_function_next(&iter)) {
        REQUIRE_OK(cg_translate_function(mem, func, &sysv_module, codegen));
    }
    REQUIRE_OK(cg_translate_function_gates(codegen, &sysv_module.function_gates, false));
    REQUIRE_OK(cg_translate_function_gates(codegen, &sysv_module.function_vgates, true));
    REQUIRE_OK(cg_translate_global_inline_assembly(codegen, &sysv_module));
    REQUIRE_OK(cg_translate_inline_assembly_fragments(mem, codegen, &sysv_module));
    REQUIRE_OK(cg_translate_tls_entries(codegen, &sysv_module.tls_entries, sysv_module.module));
    REQUIRE_OK(cg_translate_data(mem, codegen, &sysv_module));
    REQUIRE_OK(kefir_codegen_amd64_sysv_module_free(mem, &sysv_module));
    return KEFIR_OK;
}

static kefir_result_t cg_close(struct kefir_mem *mem, struct kefir_codegen *cg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator interface"));
    REQUIRE(cg->data != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    struct kefir_codegen_amd64 *codegen = (struct kefir_codegen_amd64 *) cg->data;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_CLOSE(mem, &codegen->xasmgen));
    return KEFIR_OK;
}

static kefir_result_t match_syntax(const char *syntax_descr, kefir_amd64_xasmgen_syntax_t *syntax) {
    if (syntax_descr == NULL) {
        *syntax = KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX;
    } else if (strcmp(syntax_descr, KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_PREFIX) == 0) {
        *syntax = KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX;
    } else if (strcmp(syntax_descr, KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_NOPREFIX) == 0) {
        *syntax = KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_NOPREFIX;
    } else if (strcmp(syntax_descr, KEFIR_CODEGEN_SYNTAX_X86_64_ATT) == 0) {
        *syntax = KEFIR_AMD64_XASMGEN_SYNTAX_ATT;
    } else {
        return KEFIR_SET_ERRORF(KEFIR_INVALID_PARAMETER, "Unknown amd64 assembly syntax descriptor '%s'", syntax_descr);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_sysv_init(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen, FILE *out,
                                             const struct kefir_codegen_configuration *config) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator pointer"));
    if (config == NULL) {
        config = &KefirCodegenDefaultConfiguration;
    }

    kefir_amd64_xasmgen_syntax_t syntax = KEFIR_AMD64_XASMGEN_SYNTAX_INTEL_PREFIX;
    REQUIRE_OK(match_syntax(config->syntax, &syntax));
    REQUIRE_OK(kefir_amd64_xasmgen_init(mem, &codegen->xasmgen, out, syntax));
    codegen->iface.translate = cg_translate;
    codegen->iface.close = cg_close;
    codegen->iface.data = codegen;
    codegen->iface.self = codegen;
    codegen->config = config;
    return KEFIR_OK;
}
