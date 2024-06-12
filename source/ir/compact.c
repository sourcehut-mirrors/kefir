/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/ir/compact.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <stdio.h>

static kefir_hashtree_hash_t ir_type_hash(kefir_hashtree_key_t key, void *data) {
    UNUSED(data);
    const struct kefir_ir_type *type = (const struct kefir_ir_type *) key;
    REQUIRE(type != NULL, 0);
    kefir_hashtree_hash_t hash = 0;

    for (kefir_size_t i = 0; i < kefir_ir_type_length(type); i++) {
        const struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(type, i);

        hash *= 37;
        hash += ((((kefir_uint64_t) typeentry->typecode) << 8) | typeentry->alignment) ^ typeentry->param;
    }
    return hash;
}

static kefir_int_t ir_type_compare(kefir_hashtree_key_t key1, kefir_hashtree_key_t key2, void *data) {
    UNUSED(data);
    const struct kefir_ir_type *type1 = (const struct kefir_ir_type *) key1;
    const struct kefir_ir_type *type2 = (const struct kefir_ir_type *) key2;

    if ((type1 == NULL && type2 == NULL) || type1 == type2) {
        return 0;
    } else if (type1 == NULL) {
        return -1;
    } else if (type2 == NULL) {
        return 1;
    }

    kefir_size_t len1 = kefir_ir_type_length(type1);
    kefir_size_t len2 = kefir_ir_type_length(type2);
    if (len1 < len2) {
        return -1;
    } else if (len1 > len2) {
        return 1;
    }

    for (kefir_size_t i = 0; i < kefir_ir_type_length(type1); i++) {
        const struct kefir_ir_typeentry *typeentry1 = kefir_ir_type_at(type1, i);
        const struct kefir_ir_typeentry *typeentry2 = kefir_ir_type_at(type2, i);

        if ((kefir_int64_t) typeentry1->typecode < (kefir_int64_t) typeentry2->typecode) {
            return -1;
        } else if ((kefir_int64_t) typeentry1->typecode > (kefir_int64_t) typeentry2->typecode) {
            return 1;
        }

        if (typeentry1->alignment < typeentry2->alignment) {
            return -1;
        } else if (typeentry1->alignment < typeentry2->alignment) {
            return 1;
        }

        if (typeentry1->param < typeentry2->param) {
            return -1;
        } else if (typeentry1->param < typeentry2->param) {
            return 1;
        }
    }
    return 0;
}

const struct kefir_hashtree_ops kefir_hashtree_ir_type_ops = {
    .hash = ir_type_hash, .compare = ir_type_compare, .data = NULL};

struct compact_params {
    struct kefir_hashtree type_index;
    struct kefir_hashtree function_decl_index;
    struct kefir_hashtree symbol_index;
    struct kefir_hashtree inline_asm_index;
    struct kefir_hashtree string_literal_index;

    struct kefir_list symbol_scan_queue;
};

static kefir_result_t compact_type(struct kefir_mem *mem, struct compact_params *params,
                                   const struct kefir_ir_type **type, kefir_id_t *type_id) {
    struct kefir_hashtree_node *type_index_node = NULL;
    kefir_result_t res = kefir_hashtree_at(&params->type_index, (kefir_hashtree_key_t) *type, &type_index_node);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &params->type_index, (kefir_hashtree_key_t) *type,
                                         (kefir_hashtree_value_t) *type_id));
    } else {
        REQUIRE_OK(res);
        *type = (const struct kefir_ir_type *) type_index_node->key;
        *type_id = (kefir_id_t) type_index_node->value;
    }
    return KEFIR_OK;
}

static kefir_result_t compact_function_decl(struct kefir_mem *mem, struct compact_params *params,
                                            struct kefir_ir_function_decl *decl) {
    REQUIRE_OK(compact_type(mem, params, (const struct kefir_ir_type **) &decl->params, &decl->params_type_id));
    REQUIRE_OK(compact_type(mem, params, (const struct kefir_ir_type **) &decl->result, &decl->result_type_id));
    kefir_result_t res = kefir_hashtree_insert(mem, &params->function_decl_index, (kefir_hashtree_key_t) decl->id,
                                               (kefir_hashtree_value_t) 0);
    if (res != KEFIR_ALREADY_EXISTS) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t compact_inline_asm(struct kefir_mem *mem, struct compact_params *params,
                                         struct kefir_ir_inline_assembly *inline_asm) {
    kefir_result_t res = kefir_hashtree_insert(mem, &params->inline_asm_index, (kefir_hashtree_key_t) inline_asm->id,
                                               (kefir_hashtree_value_t) 0);
    if (res != KEFIR_ALREADY_EXISTS) {
        REQUIRE_OK(res);
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_parameter *, param, iter->value);
        switch (param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                REQUIRE_OK(compact_type(mem, params, (const struct kefir_ir_type **) &param->read_type.type,
                                        &param->read_type.type_id));
                // Fallthrough

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
                REQUIRE_OK(
                    compact_type(mem, params, (const struct kefir_ir_type **) &param->type.type, &param->type.type_id));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                REQUIRE_OK(
                    compact_type(mem, params, (const struct kefir_ir_type **) &param->type.type, &param->type.type_id));
                switch (param->immediate_type) {
                    case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED:
                        // Intentionally left blank
                        break;

                    case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_LITERAL_BASED:
                        res = kefir_hashtree_insert(mem, &params->string_literal_index,
                                                    (kefir_hashtree_key_t) param->immediate_literal_base,
                                                    (kefir_hashtree_value_t) 0);
                        if (res != KEFIR_ALREADY_EXISTS) {
                            REQUIRE_OK(res);
                        }
                        break;
                }
                break;
        }
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->jump_target_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_jump_target *, jump_target, iter->value);

        REQUIRE_OK(kefir_list_insert_after(mem, &params->symbol_scan_queue, kefir_list_tail(&params->symbol_scan_queue),
                                           (void *) jump_target->target_function));
    }

    return KEFIR_OK;
}

static kefir_result_t compact_function(struct kefir_mem *mem, struct kefir_ir_module *module,
                                       struct compact_params *params, struct kefir_ir_function *function) {
    REQUIRE_OK(compact_function_decl(mem, params, function->declaration));
    REQUIRE_OK(compact_type(mem, params, (const struct kefir_ir_type **) &function->locals, &function->locals_type_id));

    for (kefir_size_t i = 0; i < kefir_irblock_length(&function->body); i++) {
        struct kefir_irinstr *instr = kefir_irblock_at(&function->body, i);
        switch (instr->opcode) {
            case KEFIR_IROPCODE_GETLOCAL:
            case KEFIR_IROPCODE_BZERO:
            case KEFIR_IROPCODE_BCOPY:
            case KEFIR_IROPCODE_VARARG_GET: {
                kefir_id_t type_id = (kefir_id_t) instr->arg.u32[0];
                struct kefir_hashtree_node *node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&module->named_types, type_id, &node));
                ASSIGN_DECL_CAST(struct kefir_ir_type *, type, node->value);
                REQUIRE_OK(compact_type(mem, params, (const struct kefir_ir_type **) &type, &type_id));
                instr->arg.u32[0] = (kefir_uint32_t) type_id;
            } break;

            case KEFIR_IROPCODE_ATOMIC_BCOPY_FROM:
            case KEFIR_IROPCODE_ATOMIC_BCOPY_TO:
            case KEFIR_IROPCODE_ATOMIC_CMPXCHG_MEMORY: {
                kefir_id_t type_id = (kefir_id_t) instr->arg.u32[1];
                struct kefir_hashtree_node *node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&module->named_types, type_id, &node));
                ASSIGN_DECL_CAST(struct kefir_ir_type *, type, node->value);
                REQUIRE_OK(compact_type(mem, params, (const struct kefir_ir_type **) &type, &type_id));
                instr->arg.u32[1] = (kefir_uint32_t) type_id;
            } break;

            case KEFIR_IROPCODE_INVOKE:
            case KEFIR_IROPCODE_INVOKEV: {
                kefir_id_t id = (kefir_id_t) instr->arg.u64;
                struct kefir_hashtree_node *node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&module->function_declarations, id, &node));
                ASSIGN_DECL_CAST(struct kefir_ir_function_decl *, decl, node->value);
                REQUIRE_OK(compact_function_decl(mem, params, decl));
                if (decl->name != NULL) {
                    REQUIRE_OK(kefir_list_insert_after(mem, &params->symbol_scan_queue,
                                                       kefir_list_tail(&params->symbol_scan_queue),
                                                       (void *) decl->name));
                }
            } break;

            case KEFIR_IROPCODE_GETGLOBAL:
            case KEFIR_IROPCODE_GETTHRLOCAL: {
                kefir_id_t id = (kefir_id_t) instr->arg.u64;
                const char *symbol = kefir_ir_module_get_named_symbol(module, id);
                REQUIRE_OK(kefir_list_insert_after(mem, &params->symbol_scan_queue,
                                                   kefir_list_tail(&params->symbol_scan_queue), (void *) symbol));
            } break;

            case KEFIR_IROPCODE_INLINEASM: {
                kefir_id_t id = (kefir_id_t) instr->arg.u64;
                struct kefir_hashtree_node *node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&module->inline_assembly, (kefir_hashtree_key_t) id, &node));
                REQUIRE_OK(compact_inline_asm(mem, params, (struct kefir_ir_inline_assembly *) node->value));
            } break;

            case KEFIR_IROPCODE_PUSHSTRING: {
                kefir_id_t id = (kefir_id_t) instr->arg.u64;
                kefir_result_t res = kefir_hashtree_insert(mem, &params->string_literal_index,
                                                           (kefir_hashtree_key_t) id, (kefir_hashtree_value_t) 0);
                if (res != KEFIR_ALREADY_EXISTS) {
                    REQUIRE_OK(res);
                }
            } break;

            default:
                // Intentionally left blank
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t compact_data(struct kefir_mem *mem, struct kefir_ir_data *data, struct compact_params *params) {
    REQUIRE_OK(compact_type(mem, params, &data->type, &data->type_id));
    for (kefir_size_t i = 0; i < data->total_length; i++) {
        const struct kefir_ir_data_value *value;
        REQUIRE_OK(kefir_ir_data_value_at(data, i, &value));

        if (!value->defined) {
            continue;
        }

        switch (value->type) {
            case KEFIR_IR_DATA_VALUE_UNDEFINED:
            case KEFIR_IR_DATA_VALUE_INTEGER:
            case KEFIR_IR_DATA_VALUE_FLOAT32:
            case KEFIR_IR_DATA_VALUE_FLOAT64:
            case KEFIR_IR_DATA_VALUE_LONG_DOUBLE:
            case KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT32:
            case KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT64:
            case KEFIR_IR_DATA_VALUE_COMPLEX_LONG_DOUBLE:
            case KEFIR_IR_DATA_VALUE_STRING:
            case KEFIR_IR_DATA_VALUE_RAW:
            case KEFIR_IR_DATA_VALUE_AGGREGATE:
                // Intentionally left blank
                break;

            case KEFIR_IR_DATA_VALUE_STRING_POINTER: {
                kefir_result_t res = kefir_hashtree_insert(mem, &params->string_literal_index,
                                                           (kefir_hashtree_key_t) value->value.string_ptr.id,
                                                           (kefir_hashtree_value_t) 0);
                if (res != KEFIR_ALREADY_EXISTS) {
                    REQUIRE_OK(res);
                }
            } break;

            case KEFIR_IR_DATA_VALUE_POINTER:
                REQUIRE_OK(kefir_list_insert_after(mem, &params->symbol_scan_queue,
                                                   kefir_list_tail(&params->symbol_scan_queue),
                                                   (void *) value->value.pointer.reference));
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_unused_named_types(struct kefir_mem *mem, struct kefir_ir_module *module,
                                              struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->named_types, &iter); node != NULL;) {

        ASSIGN_DECL_CAST(struct kefir_ir_type *, type, node->value);

        struct kefir_hashtree_node *index_node = NULL;
        kefir_result_t res = kefir_hashtree_at(&params->type_index, (kefir_hashtree_key_t) type, &index_node);
        if (res == KEFIR_NOT_FOUND || node->key != index_node->value) {
            REQUIRE(res == KEFIR_NOT_FOUND || res == KEFIR_OK, res);
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->named_types, node->key));
            node = kefir_hashtree_iter(&module->named_types, &iter);
        } else {
            REQUIRE_OK(res);
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_unused_function_decls(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                 struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->function_declarations, &iter);
         node != NULL;) {

        ASSIGN_DECL_CAST(struct kefir_ir_function_decl *, decl, node->value);

        if (!kefir_hashtree_has(&params->function_decl_index, decl->id)) {
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->function_declarations, node->key));
            node = kefir_hashtree_iter(&module->function_declarations, &iter);
        } else {
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_unused_functions(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->functions, &iter); node != NULL;) {

        ASSIGN_DECL_CAST(struct kefir_ir_function *, func, node->value);

        if (!kefir_hashtree_has(&params->symbol_index, (kefir_hashtree_key_t) func->name)) {
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->functions, node->key));
            node = kefir_hashtree_iter(&module->functions, &iter);
        } else {
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_unused_data(struct kefir_mem *mem, struct kefir_ir_module *module,
                                       struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->named_data, &iter); node != NULL;) {

        ASSIGN_DECL_CAST(const char *, name, node->key);

        if (!kefir_hashtree_has(&params->symbol_index, (kefir_hashtree_key_t) name)) {
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->named_data, node->key));
            node = kefir_hashtree_iter(&module->named_data, &iter);
        } else {
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_unused_externals(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->externals, &iter); node != NULL;) {

        ASSIGN_DECL_CAST(const char *, symbol, node->key);

        if (!kefir_hashtree_has(&params->symbol_index, (kefir_hashtree_key_t) symbol)) {
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->externals, node->key));
            node = kefir_hashtree_iter(&module->externals, &iter);
        } else {
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_unused_aliases(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->aliases, &iter); node != NULL;) {

        ASSIGN_DECL_CAST(const char *, symbol, node->key);

        if (!kefir_hashtree_has(&params->symbol_index, (kefir_hashtree_key_t) symbol)) {
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->aliases, node->key));
            node = kefir_hashtree_iter(&module->aliases, &iter);
        } else {
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_unused_inline_asm(struct kefir_mem *mem, struct kefir_ir_module *module,
                                             struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->inline_assembly, &iter); node != NULL;) {

        ASSIGN_DECL_CAST(kefir_id_t, id, node->key);

        if (!kefir_hashtree_has(&params->inline_asm_index, (kefir_hashtree_key_t) id)) {
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->inline_assembly, node->key));
            node = kefir_hashtree_iter(&module->inline_assembly, &iter);
        } else {
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t drop_unused_string_literals(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                  struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->string_literals, &iter); node != NULL;) {

        ASSIGN_DECL_CAST(kefir_id_t, id, node->key);
        ASSIGN_DECL_CAST(const struct kefir_ir_module_string_literal *, literal, node->value);

        if (!kefir_hashtree_has(&params->string_literal_index, (kefir_hashtree_key_t) id) && literal->public) {
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->string_literals, node->key));
            node = kefir_hashtree_iter(&module->string_literals, &iter);
        } else {
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t compact_impl(struct kefir_mem *mem, struct kefir_ir_module *module,
                                   struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->global_symbols, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, symbol, node->key);
        REQUIRE_OK(kefir_list_insert_after(mem, &params->symbol_scan_queue, kefir_list_tail(&params->symbol_scan_queue),
                                           (void *) symbol));
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->global_inline_asm, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly *, inline_asm, node->value);
        REQUIRE_OK(compact_inline_asm(mem, params, inline_asm));
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&params->symbol_scan_queue); iter != NULL;
         kefir_list_pop(mem, &params->symbol_scan_queue, iter), iter = kefir_list_head(&params->symbol_scan_queue)) {

        ASSIGN_DECL_CAST(const char *, symbol, iter->value);
        if (kefir_hashtree_has(&params->symbol_index, (kefir_hashtree_key_t) symbol)) {
            continue;
        }
        REQUIRE_OK(kefir_hashtree_insert(mem, &params->symbol_index, (kefir_hashtree_key_t) symbol,
                                         (kefir_hashtree_value_t) 0));

        struct kefir_hashtree_node *node = NULL;
        kefir_result_t res = kefir_hashtree_at(&module->functions, (kefir_hashtree_key_t) symbol, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct kefir_ir_function *, func, node->value);
            REQUIRE_OK(compact_function(mem, module, params, func));
        }

        res = kefir_hashtree_at(&module->named_data, (kefir_hashtree_key_t) symbol, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct kefir_ir_data *, data, node->value);
            REQUIRE_OK(compact_data(mem, data, params));
        }

        res = kefir_hashtree_at(&module->aliases, (kefir_hashtree_key_t) symbol, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(const char *, original, node->value);
            REQUIRE_OK(kefir_list_insert_after(mem, &params->symbol_scan_queue, kefir_list_tail(&params->symbol_scan_queue), (void *) original));
        }
    }

    REQUIRE_OK(drop_unused_named_types(mem, module, params));
    REQUIRE_OK(drop_unused_function_decls(mem, module, params));
    REQUIRE_OK(drop_unused_externals(mem, module, params));
    REQUIRE_OK(drop_unused_aliases(mem, module, params));
    REQUIRE_OK(drop_unused_inline_asm(mem, module, params));
    REQUIRE_OK(drop_unused_functions(mem, module, params));
    REQUIRE_OK(drop_unused_data(mem, module, params));
    REQUIRE_OK(drop_unused_string_literals(mem, module, params));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_compact(struct kefir_mem *mem, struct kefir_ir_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));

    struct compact_params params;
    REQUIRE_OK(kefir_list_init(&params.symbol_scan_queue));
    REQUIRE_OK(kefir_hashtree_init(&params.type_index, &kefir_hashtree_ir_type_ops));
    REQUIRE_OK(kefir_hashtree_init(&params.function_decl_index, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&params.symbol_index, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_init(&params.inline_asm_index, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&params.string_literal_index, &kefir_hashtree_uint_ops));

    kefir_result_t res = compact_impl(mem, module, &params);
    REQUIRE_CHAIN(&res, kefir_list_free(mem, &params.symbol_scan_queue));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.string_literal_index));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.inline_asm_index));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.symbol_index));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.function_decl_index));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.type_index));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &params.symbol_scan_queue);
        kefir_hashtree_free(mem, &params.string_literal_index);
        kefir_hashtree_free(mem, &params.inline_asm_index);
        kefir_hashtree_free(mem, &params.symbol_index);
        kefir_hashtree_free(mem, &params.function_decl_index);
        kefir_hashtree_free(mem, &params.type_index);
        return res;
    });
    return KEFIR_OK;
}
