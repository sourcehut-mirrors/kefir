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

#include <stdio.h>
#include <stdarg.h>
#include "kefir/codegen/system-v-amd64/module.h"
#include "kefir/codegen/system-v-amd64/opcodes.h"
#include "kefir/codegen/system-v-amd64/symbolic_labels.h"
#include "kefir/codegen/system-v-amd64/abi.h"
#include "kefir/codegen/system-v-amd64/symbolic_labels.h"
#include "kefir/codegen/system-v-amd64/instr.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t function_gate_removal(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                     kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    ASSIGN_DECL_CAST(struct kefir_abi_amd64_sysv_function_decl *, sysv_decl, value);
    REQUIRE_OK(kefir_abi_amd64_sysv_function_decl_free(mem, sysv_decl));
    KEFIR_FREE(mem, sysv_decl);
    return KEFIR_OK;
}

kefir_result_t type_layout_removal(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                   kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_type_layout *, layout, value);
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, layout));
    KEFIR_FREE(mem, layout);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_sysv_module_alloc(struct kefir_mem *mem,
                                                     struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                     struct kefir_ir_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sysv_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V module"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    sysv_module->module = module;
    REQUIRE_OK(kefir_hashtree_init(&sysv_module->function_gates, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&sysv_module->function_gates, function_gate_removal, NULL));
    REQUIRE_OK(kefir_hashtree_init(&sysv_module->function_vgates, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&sysv_module->function_vgates, function_gate_removal, NULL));
    REQUIRE_OK(kefir_hashtree_init(&sysv_module->type_layouts, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&sysv_module->type_layouts, type_layout_removal, NULL));
    REQUIRE_OK(kefir_hashtree_init(&sysv_module->tls_entries, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_init(&sysv_module->inline_assembly, &kefir_hashtree_uint_ops));
    sysv_module->inline_assembly_next_id = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_sysv_module_free(struct kefir_mem *mem,
                                                    struct kefir_codegen_amd64_sysv_module *sysv_module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sysv_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V module"));

    REQUIRE_OK(kefir_hashtree_free(mem, &sysv_module->inline_assembly));
    REQUIRE_OK(kefir_hashtree_free(mem, &sysv_module->tls_entries));
    REQUIRE_OK(kefir_hashtree_free(mem, &sysv_module->type_layouts));
    REQUIRE_OK(kefir_hashtree_free(mem, &sysv_module->function_gates));
    REQUIRE_OK(kefir_hashtree_free(mem, &sysv_module->function_vgates));
    sysv_module->module = NULL;
    return KEFIR_OK;
}

struct kefir_abi_amd64_sysv_function_decl *kefir_codegen_amd64_sysv_module_function_decl(
    struct kefir_mem *mem, struct kefir_codegen_amd64_sysv_module *sysv_module, kefir_id_t func_id, bool virtualDecl) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(sysv_module != NULL, NULL);

    struct kefir_hashtree_node *node = NULL;
    struct kefir_hashtree *tree = virtualDecl ? &sysv_module->function_vgates : &sysv_module->function_gates;
    kefir_result_t res = kefir_hashtree_at(tree, (kefir_hashtree_key_t) func_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        const struct kefir_ir_function_decl *decl = kefir_ir_module_get_declaration(sysv_module->module, func_id);
        REQUIRE(decl != NULL, NULL);
        struct kefir_abi_amd64_sysv_function_decl *sysv_decl =
            KEFIR_MALLOC(mem, sizeof(struct kefir_abi_amd64_sysv_function_decl));
        REQUIRE(sysv_decl != NULL, NULL);
        res = kefir_abi_amd64_sysv_function_decl_alloc(mem, decl, sysv_decl);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, sysv_decl);
            return NULL;
        });
        res = kefir_hashtree_insert(mem, tree, (kefir_hashtree_key_t) func_id, (kefir_hashtree_value_t) sysv_decl);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_abi_amd64_sysv_function_decl_free(mem, sysv_decl);
            KEFIR_FREE(mem, sysv_decl);
            return NULL;
        });
        return sysv_decl;
    } else if (res == KEFIR_OK) {
        return (struct kefir_abi_amd64_sysv_function_decl *) node->value;
    } else {
        return NULL;
    }
}

struct kefir_abi_sysv_amd64_type_layout *kefir_codegen_amd64_sysv_module_type_layout(
    struct kefir_mem *mem, struct kefir_codegen_amd64_sysv_module *sysv_module, kefir_id_t id) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(sysv_module != NULL, NULL);

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&sysv_module->type_layouts, (kefir_hashtree_key_t) id, &node);
    if (res == KEFIR_OK) {
        return (struct kefir_abi_sysv_amd64_type_layout *) node->value;
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, NULL);
    }
    struct kefir_ir_type *type = kefir_ir_module_get_named_type(sysv_module->module, id);
    REQUIRE(type != NULL, NULL);
    struct kefir_abi_sysv_amd64_type_layout *layout =
        KEFIR_MALLOC(mem, sizeof(struct kefir_abi_sysv_amd64_type_layout));
    REQUIRE(layout != NULL, NULL);
    res = kefir_abi_sysv_amd64_type_layout(type, mem, layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, layout);
        return NULL;
    });
    res = kefir_hashtree_insert(mem, &sysv_module->type_layouts, (kefir_hashtree_key_t) id,
                                (kefir_hashtree_value_t) layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(mem, layout);
        KEFIR_FREE(mem, layout);
        return NULL;
    });
    return layout;
}

kefir_result_t kefir_codegen_amd64_sysv_module_declare_tls(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                           const char *identifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sysv_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V codegen module"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));

    identifier = kefir_string_pool_insert(mem, &sysv_module->module->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));
    kefir_result_t res = kefir_hashtree_insert(mem, &sysv_module->tls_entries, (kefir_hashtree_key_t) identifier,
                                               (kefir_hashtree_value_t) 0);
    if (res != KEFIR_ALREADY_EXISTS) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
