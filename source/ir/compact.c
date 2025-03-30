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

#include "kefir/ir/compact.h"
#include "kefir/core/queue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>
#include <stdio.h>

struct compact_params {
    struct kefir_mem *mem;
    struct kefir_hashtree type_index;
    struct kefir_hashtree type_hashes;
    struct kefir_hashtree symbol_index;
    struct kefir_hashtree inline_asm_index;

    struct kefir_queue symbol_scan_queue;
};

static kefir_hashtree_hash_t ir_type_hash(kefir_hashtree_key_t key, void *data) {
    ASSIGN_DECL_CAST(struct compact_params *, params, data);
    REQUIRE(params != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compaction parameters"));
    const struct kefir_ir_type *type = (const struct kefir_ir_type *) key;
    REQUIRE(type != NULL, 0);

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&params->type_hashes, key, &node);
    if (res == KEFIR_OK) {
        return (kefir_hashtree_hash_t) node->value;
    }

    kefir_hashtree_hash_t hash = 0;

    for (kefir_size_t i = 0; i < kefir_ir_type_length(type); i++) {
        const struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(type, i);

        hash *= 37;
        hash += ((((kefir_uint64_t) typeentry->typecode) << 8) | typeentry->alignment) ^ typeentry->param;
    }

    kefir_hashtree_insert(params->mem, &params->type_hashes, key, (kefir_hashtree_value_t) hash);
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

    for (kefir_size_t i = 0; i < len1; i++) {
        const struct kefir_ir_typeentry *typeentry1 = kefir_ir_type_at(type1, i);
        const struct kefir_ir_typeentry *typeentry2 = kefir_ir_type_at(type2, i);

        if ((kefir_int64_t) typeentry1->typecode < (kefir_int64_t) typeentry2->typecode) {
            return -1;
        } else if ((kefir_int64_t) typeentry1->typecode > (kefir_int64_t) typeentry2->typecode) {
            return 1;
        }

        if (typeentry1->alignment < typeentry2->alignment) {
            return -1;
        } else if (typeentry1->alignment > typeentry2->alignment) {
            return 1;
        }

        if (typeentry1->param < typeentry2->param) {
            return -1;
        } else if (typeentry1->param > typeentry2->param) {
            return 1;
        }

        if (!typeentry1->atomic && typeentry2->atomic) {
            return -1;
        } else if (typeentry1->atomic && !typeentry2->atomic) {
            return 1;
        }
    }
    return 0;
}

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
                        REQUIRE_OK(kefir_queue_push(mem, &params->symbol_scan_queue,
                                                    (kefir_queue_entry_t) param->immediate_identifier_base));
                        break;

                    case KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_LITERAL_BASED:
                        // Intentionally left blank
                        break;
                }
                break;
        }
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->jump_target_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_jump_target *, jump_target, iter->value);

        REQUIRE_OK(
            kefir_queue_push(mem, &params->symbol_scan_queue, (kefir_queue_entry_t) jump_target->target_function));
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
            case KEFIR_IR_OPCODE_GET_LOCAL:
            case KEFIR_IR_OPCODE_ZERO_MEMORY:
            case KEFIR_IR_OPCODE_COPY_MEMORY:
            case KEFIR_IR_OPCODE_VARARG_GET:
            case KEFIR_IR_OPCODE_ADD_OVERFLOW:
            case KEFIR_IR_OPCODE_SUB_OVERFLOW:
            case KEFIR_IR_OPCODE_MUL_OVERFLOW: {
                kefir_id_t type_id = (kefir_id_t) instr->arg.u32[0];
                struct kefir_hashtree_node *node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&module->named_types, type_id, &node));
                ASSIGN_DECL_CAST(struct kefir_ir_type *, type, node->value);
                REQUIRE_OK(compact_type(mem, params, (const struct kefir_ir_type **) &type, &type_id));
                instr->arg.u32[0] = (kefir_uint32_t) type_id;
            } break;

            case KEFIR_IR_OPCODE_ATOMIC_COPY_MEMORY_FROM:
            case KEFIR_IR_OPCODE_ATOMIC_COPY_MEMORY_TO:
            case KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_MEMORY: {
                kefir_id_t type_id = (kefir_id_t) instr->arg.u32[1];
                struct kefir_hashtree_node *node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&module->named_types, type_id, &node));
                ASSIGN_DECL_CAST(struct kefir_ir_type *, type, node->value);
                REQUIRE_OK(compact_type(mem, params, (const struct kefir_ir_type **) &type, &type_id));
                instr->arg.u32[1] = (kefir_uint32_t) type_id;
            } break;

            case KEFIR_IR_OPCODE_INVOKE:
            case KEFIR_IR_OPCODE_INVOKE_VIRTUAL: {
                kefir_id_t id = (kefir_id_t) instr->arg.u64;
                struct kefir_hashtree_node *node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&module->function_declarations, id, &node));
                ASSIGN_DECL_CAST(struct kefir_ir_function_decl *, decl, node->value);
                REQUIRE_OK(compact_function_decl(mem, params, decl));
                if (decl->name != NULL) {
                    REQUIRE_OK(kefir_queue_push(mem, &params->symbol_scan_queue, (kefir_queue_entry_t) decl->name));
                }
            } break;

            case KEFIR_IR_OPCODE_GET_GLOBAL:
            case KEFIR_IR_OPCODE_GET_THREAD_LOCAL: {
                kefir_id_t id = (kefir_id_t) instr->arg.u64;
                const char *symbol = kefir_ir_module_get_named_symbol(module, id);
                REQUIRE_OK(kefir_queue_push(mem, &params->symbol_scan_queue, (kefir_queue_entry_t) symbol));
            } break;

            case KEFIR_IR_OPCODE_INLINE_ASSEMBLY: {
                kefir_id_t id = (kefir_id_t) instr->arg.u64;
                struct kefir_hashtree_node *node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&module->inline_assembly, (kefir_hashtree_key_t) id, &node));
                REQUIRE_OK(compact_inline_asm(mem, params, (struct kefir_ir_inline_assembly *) node->value));
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
            case KEFIR_IR_DATA_VALUE_BITS:
            case KEFIR_IR_DATA_VALUE_STRING_POINTER:
                // Intentionally left blank
                break;

            case KEFIR_IR_DATA_VALUE_POINTER:
                REQUIRE_OK(kefir_queue_push(mem, &params->symbol_scan_queue,
                                            (kefir_queue_entry_t) value->value.pointer.reference));
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
            const struct kefir_hashtree_node *next_node = kefir_hashtree_next(&iter);
            REQUIRE_OK(kefir_hashtree_delete(mem, &module->named_types, node->key));
            node = next_node;
        } else {
            REQUIRE_OK(res);
            node = kefir_hashtree_next(&iter);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t compact_impl(struct kefir_mem *mem, struct kefir_ir_module *module,
                                   struct compact_params *params) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->identifiers, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, symbol, node->key);
        REQUIRE_OK(kefir_queue_push(mem, &params->symbol_scan_queue, (kefir_queue_entry_t) symbol));
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->functions, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, symbol, node->key);
        REQUIRE_OK(kefir_queue_push(mem, &params->symbol_scan_queue, (kefir_queue_entry_t) symbol));
    }

    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->global_inline_asm, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly *, inline_asm, node->value);
        REQUIRE_OK(compact_inline_asm(mem, params, inline_asm));
    }

    while (!kefir_queue_is_empty(&params->symbol_scan_queue)) {
        kefir_queue_entry_t entry;
        REQUIRE_OK(kefir_queue_pop_first(params->mem, &params->symbol_scan_queue, &entry));

        ASSIGN_DECL_CAST(const char *, symbol, entry);
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

        res = kefir_hashtree_at(&module->identifiers, (kefir_hashtree_key_t) symbol, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct kefir_ir_identifier *, ir_identifier, node->value);
            if (ir_identifier->alias != NULL) {
                REQUIRE_OK(
                    kefir_queue_push(mem, &params->symbol_scan_queue, (kefir_queue_entry_t) ir_identifier->alias));
            }
        }
    }

    REQUIRE_OK(drop_unused_named_types(mem, module, params));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_module_compact(struct kefir_mem *mem, struct kefir_ir_module *module) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));

    struct compact_params params = {.mem = mem};
    const struct kefir_hashtree_ops kefir_hashtree_ir_type_ops = {
        .hash = ir_type_hash, .compare = ir_type_compare, .data = &params};
    REQUIRE_OK(kefir_queue_init(&params.symbol_scan_queue));
    REQUIRE_OK(kefir_hashtree_init(&params.type_index, &kefir_hashtree_ir_type_ops));
    REQUIRE_OK(kefir_hashtree_init(&params.type_hashes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&params.symbol_index, &kefir_hashtree_str_ops));
    REQUIRE_OK(kefir_hashtree_init(&params.inline_asm_index, &kefir_hashtree_uint_ops));

    kefir_result_t res = compact_impl(mem, module, &params);
    REQUIRE_CHAIN(&res, kefir_queue_free(mem, &params.symbol_scan_queue));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.inline_asm_index));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.symbol_index));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.type_hashes));
    REQUIRE_CHAIN(&res, kefir_hashtree_free(mem, &params.type_index));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_queue_free(mem, &params.symbol_scan_queue);
        kefir_hashtree_free(mem, &params.inline_asm_index);
        kefir_hashtree_free(mem, &params.symbol_index);
        kefir_hashtree_free(mem, &params.type_hashes);
        kefir_hashtree_free(mem, &params.type_index);
        return res;
    });
    return KEFIR_OK;
}
