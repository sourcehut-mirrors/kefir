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

#include "kefir/ir/format.h"
#include "kefir/core/hashtree.h"
#include "kefir/ir/builtins.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/ir/mnemonic.h"
#include "kefir/ir/format_impl.h"
#include "kefir/util/json.h"

kefir_result_t kefir_ir_format_instr_none(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                          const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_i64(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                         const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_integer(json, instr->arg.i64));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_u64(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                         const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->arg.u64));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_bool(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                          const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_boolean(json, instr->arg.u64 != 0));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_u32(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                         const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->arg.u32[0]));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->arg.u32[1]));
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_f32(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                         const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_float(json, instr->arg.f32[0]));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_f64(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                         const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_float(json, instr->arg.f64));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_ldouble(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                             const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_long_double(json, instr->arg.long_double));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_typeref(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                             const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->arg.u32[0]));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->arg.u32[1]));
    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_identifier(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                                const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "data"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_string_pool_get(&module->symbols, instr->arg.u64)));
    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_funcref(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                             const struct kefir_irinstr *instr) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR module"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));
    const struct kefir_ir_function_decl *decl = kefir_ir_module_get_declaration(module, (kefir_id_t) instr->arg.u64);

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
    REQUIRE_OK(kefir_json_output_uinteger(json, decl->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "name"));
    if (decl->name != NULL) {
        REQUIRE_OK(kefir_json_output_string(json, decl->name));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_coderef(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                             const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->arg.u64));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_string(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                            const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "arg"));
    REQUIRE_OK(kefir_json_output_integer(json, instr->arg.i64));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_memflags(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                              const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));
    REQUIRE_OK(kefir_json_output_object_key(json, "memory_flags"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, (instr->arg.u64 & KEFIR_IR_MEMORY_FLAG_VOLATILE) != 0));
    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_memory_order(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                                  const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));

    REQUIRE_OK(kefir_json_output_object_key(json, "memory_order"));
    switch (instr->arg.i64) {
        case KEFIR_IR_MEMORY_ORDER_SEQ_CST:
            REQUIRE_OK(kefir_json_output_string(json, "seq_cst"));
            break;
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_instr_atomic_typeref(struct kefir_json_output *json,
                                                    const struct kefir_ir_module *module,
                                                    const struct kefir_irinstr *instr) {
    UNUSED(module);
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid json output"));
    REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR instruction"));

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode"));
    REQUIRE_OK(kefir_json_output_string(json, kefir_iropcode_mnemonic(instr->opcode)));

    REQUIRE_OK(kefir_json_output_object_key(json, "memory_order"));
    switch (instr->arg.u32[0]) {
        case KEFIR_IR_MEMORY_ORDER_SEQ_CST:
            REQUIRE_OK(kefir_json_output_string(json, "seq_cst"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->arg.u32[1]));
    REQUIRE_OK(kefir_json_output_object_key(json, "index"));
    REQUIRE_OK(kefir_json_output_uinteger(json, instr->arg.u32[2]));

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

struct format_param {
    struct kefir_json_output *json;
    struct kefir_ir_type_visitor *visitor;
};

static const char *typecode_to_string(kefir_ir_typecode_t typecode) {
    switch (typecode) {
        case KEFIR_IR_TYPE_STRUCT:
            return "struct";

        case KEFIR_IR_TYPE_ARRAY:
            return "array";

        case KEFIR_IR_TYPE_UNION:
            return "union";

        case KEFIR_IR_TYPE_INT8:
            return "int8";

        case KEFIR_IR_TYPE_INT16:
            return "int16";

        case KEFIR_IR_TYPE_INT32:
            return "int32";

        case KEFIR_IR_TYPE_INT64:
            return "int64";

        case KEFIR_IR_TYPE_FLOAT32:
            return "float";

        case KEFIR_IR_TYPE_FLOAT64:
            return "double";

        case KEFIR_IR_TYPE_LONG_DOUBLE:
            return "long_double";

        case KEFIR_IR_TYPE_BOOL:
            return "bool";

        case KEFIR_IR_TYPE_CHAR:
            return "char";

        case KEFIR_IR_TYPE_SHORT:
            return "short";

        case KEFIR_IR_TYPE_INT:
            return "int";

        case KEFIR_IR_TYPE_LONG:
            return "long";

        case KEFIR_IR_TYPE_WORD:
            return "word";

        case KEFIR_IR_TYPE_BITS:
            return "bits";

        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
            return "complex_float32";

        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
            return "complex_float64";

        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
            return "complex_long_double";

        case KEFIR_IR_TYPE_BUILTIN:
            return "builtin";

        case KEFIR_IR_TYPE_NONE:
            return "none";

        default:
            return NULL;
    }
}

static kefir_result_t format_type_default(const struct kefir_ir_type *type, kefir_size_t index,
                                          const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct format_param *, param, payload);

    REQUIRE_OK(kefir_json_output_object_key(param->json, "type"));

    const char *type_literal = typecode_to_string(typeentry->typecode);
    REQUIRE(type_literal != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown IR type code"));
    REQUIRE_OK(kefir_json_output_string(param->json, type_literal));

    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_FLOAT32:
        case KEFIR_IR_TYPE_FLOAT64:
        case KEFIR_IR_TYPE_LONG_DOUBLE:
        case KEFIR_IR_TYPE_WORD:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
            break;

        case KEFIR_IR_TYPE_BITS: {
            kefir_size_t bits = typeentry->param;

            REQUIRE_OK(kefir_json_output_object_key(param->json, "width"));
            REQUIRE_OK(kefir_json_output_integer(param->json, bits));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid type code");
    }
    return KEFIR_OK;
}

static kefir_result_t format_type_struct_union(const struct kefir_ir_type *type, kefir_size_t index,
                                               const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct format_param *, param, payload);

    REQUIRE_OK(kefir_json_output_object_key(param->json, "type"));
    REQUIRE_OK(kefir_json_output_string(param->json, typeentry->typecode == KEFIR_IR_TYPE_STRUCT ? "struct" : "union"));
    REQUIRE_OK(kefir_json_output_object_key(param->json, "fields"));
    REQUIRE_OK(kefir_json_output_array_begin(param->json));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, payload, index + 1, typeentry->param));
    REQUIRE_OK(kefir_json_output_array_end(param->json));
    return KEFIR_OK;
}

static kefir_result_t format_type_array(const struct kefir_ir_type *type, kefir_size_t index,
                                        const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct format_param *, param, payload);

    REQUIRE_OK(kefir_json_output_object_key(param->json, "type"));
    REQUIRE_OK(kefir_json_output_string(param->json, "array"));
    REQUIRE_OK(kefir_json_output_object_key(param->json, "length"));
    REQUIRE_OK(kefir_json_output_integer(param->json, typeentry->param));
    REQUIRE_OK(kefir_json_output_object_key(param->json, "element_type"));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, payload, index + 1, 1));
    return KEFIR_OK;
}

static kefir_result_t format_type_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                          const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct format_param *, param, payload);

    REQUIRE_OK(kefir_json_output_object_key(param->json, "type"));
    REQUIRE_OK(kefir_json_output_string(param->json, "builtin"));
    REQUIRE_OK(kefir_json_output_object_key(param->json, "class"));
    switch (typeentry->param) {
        case KEFIR_IR_TYPE_BUILTIN_VARARG:
            REQUIRE_OK(kefir_json_output_string(param->json, "vararg"));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid built-in type code");
    }
    return KEFIR_OK;
}

static kefir_result_t format_type_prehook(const struct kefir_ir_type *type, kefir_size_t index,
                                          const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct format_param *, param, payload);

    REQUIRE_OK(kefir_json_output_object_begin(param->json));
    return KEFIR_OK;
}

static kefir_result_t format_type_posthook(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct format_param *, param, payload);

    if (typeentry->alignment > 0) {
        REQUIRE_OK(kefir_json_output_object_key(param->json, "alignment"));
        REQUIRE_OK(kefir_json_output_uinteger(param->json, typeentry->alignment));
    }
    if (typeentry->atomic) {
        REQUIRE_OK(kefir_json_output_object_key(param->json, "atomic"));
        REQUIRE_OK(kefir_json_output_boolean(param->json, true));
    }
    REQUIRE_OK(kefir_json_output_object_end(param->json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_type_json(struct kefir_json_output *json, const struct kefir_ir_type *type) {
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, format_type_default));
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = format_type_struct_union;
    visitor.visit[KEFIR_IR_TYPE_UNION] = format_type_struct_union;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = format_type_array;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = format_type_builtin;
    visitor.prehook = format_type_prehook;
    visitor.posthook = format_type_posthook;

    struct format_param param = {.json = json, .visitor = &visitor};
    REQUIRE_OK(kefir_json_output_array_begin(json));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, &visitor, &param, 0, kefir_ir_type_length(type)));
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_type(FILE *fp, const struct kefir_ir_type *type) {
    REQUIRE(fp != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file pointer"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, fp, 4));
    REQUIRE_OK(kefir_ir_format_type_json(&json, type));
    REQUIRE_OK(kefir_json_output_finalize(&json));
    return KEFIR_OK;
}

static kefir_result_t kefir_ir_format_function_declaration(struct kefir_json_output *json,
                                                           struct kefir_ir_function_decl *decl) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
    REQUIRE_OK(kefir_json_output_uinteger(json, decl->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "name"));
    if (decl->name != NULL) {
        REQUIRE_OK(kefir_json_output_string(json, decl->name));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "parameters"));
    if (decl->params_type_id != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_json_output_uinteger(json, decl->params_type_id));
    } else {
        REQUIRE_OK(kefir_ir_format_type_json(json, decl->params));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "vararg"));
    REQUIRE_OK(kefir_json_output_boolean(json, decl->vararg));
    REQUIRE_OK(kefir_json_output_object_key(json, "returns"));
    if (decl->result_type_id != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_json_output_uinteger(json, decl->result_type_id));
    } else {
        REQUIRE_OK(kefir_ir_format_type_json(json, decl->result));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "returns_twice"));
    REQUIRE_OK(kefir_json_output_boolean(json, decl->returns_twice));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t kefir_ir_format_function(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                               const struct kefir_ir_function *func, kefir_bool_t debug_info) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
    REQUIRE_OK(kefir_json_output_uinteger(json, func->declaration->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "name"));
    REQUIRE_OK(kefir_json_output_string(json, func->name));

    if (func->locals_type_id != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_json_output_object_key(json, "locals"));
        REQUIRE_OK(kefir_json_output_uinteger(json, func->locals_type_id));
    }

    REQUIRE_OK(kefir_json_output_object_key(json, "body"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (kefir_size_t i = 0; i < kefir_irblock_length(&func->body); i++) {
        const struct kefir_irinstr *instr = kefir_irblock_at(&func->body, i);
        REQUIRE_OK(kefir_ir_format_instr(json, module, instr));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    struct kefir_hashtree_node_iterator iter;
    REQUIRE_OK(kefir_json_output_object_key(json, "public_labels"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&func->body.public_labels, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, label, node->key);
        ASSIGN_DECL_CAST(kefir_size_t, location, node->value);

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "label"));
        REQUIRE_OK(kefir_json_output_string(json, label));
        REQUIRE_OK(kefir_json_output_object_key(json, "location"));
        REQUIRE_OK(kefir_json_output_uinteger(json, location));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    if (debug_info) {
        REQUIRE_OK(kefir_json_output_object_key(json, "source_map"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        struct kefir_ir_source_map_iterator source_iter;
        const struct kefir_ir_source_location *source_location;
        kefir_result_t res;
        for (res = kefir_ir_source_map_iter(&func->debug_info.source_map, &source_iter, &source_location);
             res == KEFIR_OK; res = kefir_ir_source_map_next(&source_iter, &source_location)) {
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "source_id"));
            REQUIRE_OK(kefir_json_output_string(json, source_location->location.source));
            REQUIRE_OK(kefir_json_output_object_key(json, "line"));
            REQUIRE_OK(kefir_json_output_uinteger(json, source_location->location.line));
            REQUIRE_OK(kefir_json_output_object_key(json, "column"));
            REQUIRE_OK(kefir_json_output_uinteger(json, source_location->location.column));
            REQUIRE_OK(kefir_json_output_object_key(json, "begin"));
            REQUIRE_OK(kefir_json_output_uinteger(json, source_location->begin));
            REQUIRE_OK(kefir_json_output_object_key(json, "end"));
            REQUIRE_OK(kefir_json_output_uinteger(json, source_location->end));
            REQUIRE_OK(kefir_json_output_object_end(json));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
    }

    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_identifiers(struct kefir_json_output *json, const struct kefir_ir_module *module, kefir_bool_t debug_info) {
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    const struct kefir_ir_identifier *identifier;
    for (const char *symbol = kefir_ir_module_identifiers_iter(module, &iter, &identifier); symbol != NULL;
         symbol = kefir_ir_module_identifiers_next(&iter, &identifier)) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "symbol"));
        REQUIRE_OK(kefir_json_output_string(json, symbol));

        REQUIRE_OK(kefir_json_output_object_key(json, "type"));
        switch (identifier->type) {
            case KEFIR_IR_IDENTIFIER_FUNCTION:
                REQUIRE_OK(kefir_json_output_string(json, "function"));
                break;

            case KEFIR_IR_IDENTIFIER_GLOBAL_DATA:
                REQUIRE_OK(kefir_json_output_string(json, "global_data"));
                break;

            case KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA:
                REQUIRE_OK(kefir_json_output_string(json, "thread_local_data"));
                break;
        }

        REQUIRE_OK(kefir_json_output_object_key(json, "scope"));
        switch (identifier->scope) {
            case KEFIR_IR_IDENTIFIER_SCOPE_EXPORT:
                REQUIRE_OK(kefir_json_output_string(json, "export"));
                break;

            case KEFIR_IR_IDENTIFIER_SCOPE_EXPORT_WEAK:
                REQUIRE_OK(kefir_json_output_string(json, "export_weak"));
                break;

            case KEFIR_IR_IDENTIFIER_SCOPE_IMPORT:
                REQUIRE_OK(kefir_json_output_string(json, "import"));
                break;

            case KEFIR_IR_IDENTIFIER_SCOPE_LOCAL:
                REQUIRE_OK(kefir_json_output_string(json, "local"));
                break;
        }

        REQUIRE_OK(kefir_json_output_object_key(json, "visibility"));
        switch (identifier->visibility) {
            case KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT:
                REQUIRE_OK(kefir_json_output_string(json, "default"));
                break;

            case KEFIR_IR_IDENTIFIER_VISIBILITY_HIDDEN:
                REQUIRE_OK(kefir_json_output_string(json, "hidden"));
                break;

            case KEFIR_IR_IDENTIFIER_VISIBILITY_INTERNAL:
                REQUIRE_OK(kefir_json_output_string(json, "internal"));
                break;

            case KEFIR_IR_IDENTIFIER_VISIBILITY_PROTECTED:
                REQUIRE_OK(kefir_json_output_string(json, "protected"));
                break;
        }

        REQUIRE_OK(kefir_json_output_object_key(json, "alias"));
        if (identifier->alias != NULL) {
            REQUIRE_OK(kefir_json_output_string(json, identifier->alias));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }

        if (debug_info) {
            REQUIRE_OK(kefir_json_output_object_key(json, "debug_info"));
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "type"));
            if (identifier->debug_info.type != KEFIR_IR_DEBUG_ENTRY_ID_NONE) {
                REQUIRE_OK(kefir_json_output_uinteger(json, identifier->debug_info.type));
            } else {
                REQUIRE_OK(kefir_json_output_null(json));
            }
            REQUIRE_OK(kefir_json_output_object_end(json));
        }

        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

#undef FORMAT_IDENTIFIER_ITER

static kefir_result_t format_types(struct kefir_json_output *json, const struct kefir_ir_module *module) {
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->named_types, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ir_type *, type, node->value);
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
        REQUIRE_OK(kefir_json_output_uinteger(json, node->key));
        REQUIRE_OK(kefir_json_output_object_key(json, "type"));
        REQUIRE_OK(kefir_ir_format_type_json(json, type));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_datum(struct kefir_json_output *json, const struct kefir_ir_data *data) {
    REQUIRE_OK(kefir_json_output_array_begin(json));
    kefir_size_t undefined_count = 0;
    for (kefir_size_t i = 0; i < data->total_length; i++) {
        const struct kefir_ir_data_value *value;
        REQUIRE_OK(kefir_ir_data_value_at(data, i, &value));

        if (value->type != KEFIR_IR_DATA_VALUE_UNDEFINED) {
            if (undefined_count > 0) {
                REQUIRE_OK(kefir_json_output_object_begin(json));
                REQUIRE_OK(kefir_json_output_object_key(json, "class"));
                REQUIRE_OK(kefir_json_output_string(json, "undefined"));
                if (undefined_count > 1) {
                    REQUIRE_OK(kefir_json_output_object_key(json, "count"));
                    REQUIRE_OK(kefir_json_output_uinteger(json, undefined_count));
                }
                REQUIRE_OK(kefir_json_output_object_end(json));
                undefined_count = 0;
            }
            REQUIRE_OK(kefir_json_output_object_begin(json));
            REQUIRE_OK(kefir_json_output_object_key(json, "class"));
        }

        switch (value->type) {
            case KEFIR_IR_DATA_VALUE_UNDEFINED:
                undefined_count++;
                break;

            case KEFIR_IR_DATA_VALUE_INTEGER:
                REQUIRE_OK(kefir_json_output_string(json, "integer"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_integer(json, value->value.integer));
                break;

            case KEFIR_IR_DATA_VALUE_FLOAT32:
                REQUIRE_OK(kefir_json_output_string(json, "float32"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_float(json, value->value.float32));
                break;

            case KEFIR_IR_DATA_VALUE_FLOAT64:
                REQUIRE_OK(kefir_json_output_string(json, "float64"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_float(json, value->value.float64));
                break;

            case KEFIR_IR_DATA_VALUE_LONG_DOUBLE:
                REQUIRE_OK(kefir_json_output_string(json, "long_double"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_long_double(json, value->value.long_double));
                break;

            case KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT32:
                REQUIRE_OK(kefir_json_output_string(json, "complex_float32"));
                REQUIRE_OK(kefir_json_output_object_key(json, "real_value"));
                REQUIRE_OK(kefir_json_output_float(json, value->value.complex_float32.real));
                REQUIRE_OK(kefir_json_output_object_key(json, "imaginary_value"));
                REQUIRE_OK(kefir_json_output_float(json, value->value.complex_float32.imaginary));
                break;

            case KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT64:
                REQUIRE_OK(kefir_json_output_string(json, "complex_float64"));
                REQUIRE_OK(kefir_json_output_object_key(json, "real_value"));
                REQUIRE_OK(kefir_json_output_float(json, value->value.complex_float64.real));
                REQUIRE_OK(kefir_json_output_object_key(json, "imaginary_value"));
                REQUIRE_OK(kefir_json_output_float(json, value->value.complex_float64.imaginary));
                break;

            case KEFIR_IR_DATA_VALUE_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(kefir_json_output_string(json, "complex_long_double"));
                REQUIRE_OK(kefir_json_output_object_key(json, "real_value"));
                REQUIRE_OK(kefir_json_output_long_double(json, value->value.complex_long_double.real));
                REQUIRE_OK(kefir_json_output_object_key(json, "imaginary_value"));
                REQUIRE_OK(kefir_json_output_long_double(json, value->value.complex_long_double.imaginary));
                break;

            case KEFIR_IR_DATA_VALUE_STRING:
                REQUIRE_OK(kefir_json_output_string(json, "string"));
                REQUIRE_OK(kefir_json_output_object_key(json, "content"));
                REQUIRE_OK(
                    kefir_json_output_raw_string(json, (const char *) value->value.raw.data, value->value.raw.length));
                REQUIRE_OK(kefir_json_output_object_key(json, "length"));
                REQUIRE_OK(kefir_json_output_uinteger(json, value->value.raw.length));
                break;

            case KEFIR_IR_DATA_VALUE_POINTER:
                REQUIRE_OK(kefir_json_output_string(json, "pointer"));
                REQUIRE_OK(kefir_json_output_object_key(json, "reference"));
                REQUIRE_OK(kefir_json_output_string(json, value->value.pointer.reference));
                REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
                REQUIRE_OK(kefir_json_output_integer(json, value->value.pointer.offset));
                break;

            case KEFIR_IR_DATA_VALUE_STRING_POINTER: {
                REQUIRE_OK(kefir_json_output_string(json, "string_pointer"));
                REQUIRE_OK(kefir_json_output_object_key(json, "string"));
                REQUIRE_OK(kefir_json_output_integer(json, value->value.string_ptr.id));
                REQUIRE_OK(kefir_json_output_object_key(json, "offset"));
                REQUIRE_OK(kefir_json_output_integer(json, value->value.pointer.offset));
            } break;

            case KEFIR_IR_DATA_VALUE_RAW:
                REQUIRE_OK(kefir_json_output_string(json, "raw"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                for (kefir_size_t i = 0; i < value->value.raw.length; i++) {
                    REQUIRE_OK(kefir_json_output_integer(json, ((const char *) value->value.raw.data)[i]));
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
                break;

            case KEFIR_IR_DATA_VALUE_AGGREGATE:
                REQUIRE_OK(kefir_json_output_string(json, "aggregate"));
                break;

            case KEFIR_IR_DATA_VALUE_BITS:
                REQUIRE_OK(kefir_json_output_string(json, "bits"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                for (kefir_size_t i = 0; i < value->value.bits.length; i++) {
                    REQUIRE_OK(kefir_json_output_uinteger(json, value->value.bits.bits[i]));
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
                break;
        }
        if (value->type != KEFIR_IR_DATA_VALUE_UNDEFINED) {
            REQUIRE_OK(kefir_json_output_object_end(json));
        }
    }

    if (undefined_count > 0) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "class"));
        REQUIRE_OK(kefir_json_output_string(json, "undefined"));
        if (undefined_count > 1) {
            REQUIRE_OK(kefir_json_output_object_key(json, "count"));
            REQUIRE_OK(kefir_json_output_uinteger(json, undefined_count));
        }
        REQUIRE_OK(kefir_json_output_object_end(json));
        undefined_count = 0;
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_data(struct kefir_json_output *json, const struct kefir_ir_module *module) {
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    const char *identifier;
    for (const struct kefir_ir_data *data = kefir_ir_module_named_data_iter(module, &iter, &identifier); data != NULL;
         data = kefir_ir_module_named_data_next(&iter, &identifier)) {

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
        REQUIRE_OK(kefir_json_output_string(json, identifier));
        REQUIRE_OK(kefir_json_output_object_key(json, "storage"));
        switch (data->storage) {
            case KEFIR_IR_DATA_GLOBAL_STORAGE:
                REQUIRE_OK(kefir_json_output_string(json, "global"));
                break;

            case KEFIR_IR_DATA_THREAD_LOCAL_STORAGE:
                REQUIRE_OK(kefir_json_output_string(json, "thread_local"));
                break;
        }
        REQUIRE_OK(kefir_json_output_object_key(json, "type"));
        REQUIRE_OK(kefir_json_output_uinteger(json, data->type_id));
        REQUIRE_OK(kefir_json_output_object_key(json, "value"));
        REQUIRE_OK(format_datum(json, data));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_string_literal(struct kefir_json_output *json, const struct kefir_ir_module *module) {
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    kefir_id_t id;
    kefir_ir_string_literal_type_t literal_type;
    kefir_bool_t public;
    const void *literal;
    kefir_size_t length;
    kefir_result_t res = KEFIR_OK;
    for (res = kefir_ir_module_string_literal_iter(module, &iter, &id, &literal_type, &public, &literal, &length);
         res == KEFIR_OK;
         res = kefir_ir_module_string_literal_next(&iter, &id, &literal_type, &public, &literal, &length)) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "id"));
        REQUIRE_OK(kefir_json_output_integer(json, id));
        REQUIRE_OK(kefir_json_output_object_key(json, "public"));
        REQUIRE_OK(kefir_json_output_boolean(json, public));
        REQUIRE_OK(kefir_json_output_object_key(json, "type"));
        switch (literal_type) {
            case KEFIR_IR_STRING_LITERAL_MULTIBYTE:
                REQUIRE_OK(kefir_json_output_string(json, "multibyte"));
                REQUIRE_OK(kefir_json_output_object_key(json, "literal"));
                REQUIRE_OK(kefir_json_output_raw_string(json, literal, length));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE16:
                REQUIRE_OK(kefir_json_output_string(json, "unicode16"));
                REQUIRE_OK(kefir_json_output_object_key(json, "literal"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                for (kefir_size_t i = 0; i < length; i++) {
                    kefir_char16_t chr = ((kefir_char16_t *) literal)[i];
                    REQUIRE_OK(kefir_json_output_uinteger(json, chr));
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
                break;

            case KEFIR_IR_STRING_LITERAL_UNICODE32:
                REQUIRE_OK(kefir_json_output_string(json, "unicode32"));
                REQUIRE_OK(kefir_json_output_object_key(json, "literal"));
                REQUIRE_OK(kefir_json_output_array_begin(json));
                for (kefir_size_t i = 0; i < length; i++) {
                    kefir_char32_t chr = ((kefir_char32_t *) literal)[i];
                    REQUIRE_OK(kefir_json_output_uinteger(json, chr));
                }
                REQUIRE_OK(kefir_json_output_array_end(json));
                break;
        }
        REQUIRE_OK(kefir_json_output_object_key(json, "length"));
        REQUIRE_OK(kefir_json_output_uinteger(json, length));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_function_declarations(struct kefir_json_output *json,
                                                   const struct kefir_ir_module *module) {
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&module->function_declarations, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {
        REQUIRE_OK(kefir_ir_format_function_declaration(json, (struct kefir_ir_function_decl *) node->value));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_functions(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                       kefir_bool_t debug_info) {
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *func = kefir_ir_module_function_iter(module, &iter); func != NULL;
         func = kefir_ir_module_function_next(&iter)) {
        REQUIRE_OK(kefir_ir_format_function(json, module, func, debug_info));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_inline_assembly_fragment(struct kefir_json_output *json,
                                                      const struct kefir_ir_module *module,
                                                      const struct kefir_ir_inline_assembly *inline_asm) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
    REQUIRE_OK(kefir_json_output_uinteger(json, inline_asm->id));
    REQUIRE_OK(kefir_json_output_object_key(json, "global"));
    REQUIRE_OK(kefir_json_output_boolean(
        json, kefir_hashtree_has(&module->global_inline_asm, (kefir_hashtree_key_t) inline_asm->id)));
    REQUIRE_OK(kefir_json_output_object_key(json, "template"));
    REQUIRE_OK(kefir_json_output_string(json, inline_asm->template));
    REQUIRE_OK(kefir_json_output_object_key(json, "parameters"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_parameter *, param, iter->value);
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
        REQUIRE_OK(kefir_json_output_uinteger(json, param->parameter_id));
        REQUIRE_OK(kefir_json_output_object_key(json, "names"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (const struct kefir_list_entry *iter = kefir_list_head(&param->identifiers); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(const char *, name, iter->value);
            REQUIRE_OK(kefir_json_output_string(json, name));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "class"));
        switch (param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
                REQUIRE_OK(kefir_json_output_string(json, "read"));
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->read_type.type_id));
                REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->read_type.index));
                REQUIRE_OK(kefir_json_output_object_key(json, "from"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->read_index));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
                REQUIRE_OK(kefir_json_output_string(json, "load"));
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.type_id));
                REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.index));
                REQUIRE_OK(kefir_json_output_object_key(json, "from"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->load_store_index));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
                REQUIRE_OK(kefir_json_output_string(json, "store"));
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.type_id));
                REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.index));
                REQUIRE_OK(kefir_json_output_object_key(json, "to"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->load_store_index));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
                REQUIRE_OK(kefir_json_output_string(json, "load_store"));
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.type_id));
                REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.index));
                REQUIRE_OK(kefir_json_output_object_key(json, "from_to"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->load_store_index));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                REQUIRE_OK(kefir_json_output_string(json, "read_store"));
                REQUIRE_OK(kefir_json_output_object_key(json, "from_type"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->read_type.type_id));
                REQUIRE_OK(kefir_json_output_object_key(json, "from_type_index"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->read_type.index));
                REQUIRE_OK(kefir_json_output_object_key(json, "to_type"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.type_id));
                REQUIRE_OK(kefir_json_output_object_key(json, "to_type_index"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.index));
                REQUIRE_OK(kefir_json_output_object_key(json, "from"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->read_index));
                REQUIRE_OK(kefir_json_output_object_key(json, "to"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->load_store_index));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                REQUIRE_OK(kefir_json_output_string(json, "immediate"));
                REQUIRE_OK(kefir_json_output_object_key(json, "variant"));
                if (param->immediate_type == KEFIR_IR_INLINE_ASSEMBLY_IMMEDIATE_IDENTIFIER_BASED) {
                    REQUIRE_OK(kefir_json_output_string(json, "identifier_based"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "base"));
                    if (param->immediate_identifier_base != NULL) {
                        REQUIRE_OK(kefir_json_output_string(json, param->immediate_identifier_base));
                    } else {
                        REQUIRE_OK(kefir_json_output_null(json));
                    }
                } else {
                    REQUIRE_OK(kefir_json_output_string(json, "literal_based"));
                    REQUIRE_OK(kefir_json_output_object_key(json, "base"));
                    REQUIRE_OK(kefir_json_output_uinteger(json, param->immediate_literal_base));
                }
                REQUIRE_OK(kefir_json_output_object_key(json, "type"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.type_id));
                REQUIRE_OK(kefir_json_output_object_key(json, "type_index"));
                REQUIRE_OK(kefir_json_output_uinteger(json, param->type.index));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_integer(json, param->immediate_value));
                break;
        }
        REQUIRE_OK(kefir_json_output_object_key(json, "constraints"));
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "general_purpose_register"));
        REQUIRE_OK(kefir_json_output_boolean(json, param->constraints.general_purpose_register));
        REQUIRE_OK(kefir_json_output_object_key(json, "memory_location"));
        REQUIRE_OK(kefir_json_output_boolean(json, param->constraints.memory_location));
        REQUIRE_OK(kefir_json_output_object_key(json, "immediate"));
        REQUIRE_OK(kefir_json_output_boolean(json, param->constraints.immediate));
        REQUIRE_OK(kefir_json_output_object_key(json, "strict_immediate"));
        REQUIRE_OK(kefir_json_output_boolean(json, param->constraints.strict_immediate));
        REQUIRE_OK(kefir_json_output_object_key(json, "explicit_register"));
        if (param->constraints.explicit_register != NULL) {
            REQUIRE_OK(kefir_json_output_string(json, param->constraints.explicit_register));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
        REQUIRE_OK(kefir_json_output_object_end(json));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "clobbers"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->clobbers, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(const char *, clobber, node->key);
        REQUIRE_OK(kefir_json_output_string(json, clobber));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "jump_targets"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->jump_target_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_jump_target *, jump_target, iter->value);
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
        REQUIRE_OK(kefir_json_output_uinteger(json, jump_target->uid));
        REQUIRE_OK(kefir_json_output_object_key(json, "names"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (const struct kefir_list_entry *iter2 = kefir_list_head(&jump_target->identifiers); iter2 != NULL;
             kefir_list_next(&iter2)) {
            REQUIRE_OK(kefir_json_output_string(json, (const char *) iter2->value));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "function"));
        REQUIRE_OK(kefir_json_output_string(json, jump_target->target_function));
        REQUIRE_OK(kefir_json_output_object_key(json, "target"));
        REQUIRE_OK(kefir_json_output_uinteger(json, jump_target->target));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_inline_assembly(struct kefir_json_output *json, const struct kefir_ir_module *module) {
    REQUIRE_OK(kefir_json_output_array_begin(json));
    kefir_id_t id;
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_inline_assembly *inline_asm = kefir_ir_module_inline_assembly_iter(module, &iter, &id);
         inline_asm != NULL; inline_asm = kefir_ir_module_inline_assembly_next(&iter, &id)) {
        REQUIRE_OK(format_inline_assembly_fragment(json, module, inline_asm));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_debug_entry(struct kefir_json_output *json, const struct kefir_ir_module *module, const struct kefir_ir_debug_entry *entry) {
    kefir_result_t res;
    struct kefir_ir_debug_entry_attribute_iterator attr_iter;
    const struct kefir_ir_debug_entry_attribute *entry_attr;
    kefir_ir_debug_entry_id_t child_entry_id;
    
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
    REQUIRE_OK(kefir_json_output_uinteger(json, entry->identifier));
    REQUIRE_OK(kefir_json_output_object_key(json, "tag"));
    switch (entry->tag) {
        case KEFIR_IR_DEBUG_ENTRY_TYPE_VOID:
            REQUIRE_OK(kefir_json_output_string(json, "type_void"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_BOOLEAN:
            REQUIRE_OK(kefir_json_output_string(json, "type_boolean"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_CHARACTER:
            REQUIRE_OK(kefir_json_output_string(json, "type_character"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_CHARACTER:
            REQUIRE_OK(kefir_json_output_string(json, "type_unsigned_character"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_CHARACTER:
            REQUIRE_OK(kefir_json_output_string(json, "type_signed_character"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT:
            REQUIRE_OK(kefir_json_output_string(json, "type_unsigned_int"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_INT:
            REQUIRE_OK(kefir_json_output_string(json, "type_signed_int"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_FLOAT:
            REQUIRE_OK(kefir_json_output_string(json, "type_float"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(kefir_json_output_string(json, "type_complex_float"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_POINTER:
            REQUIRE_OK(kefir_json_output_string(json, "type_pointer"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_ENUMERATION:
            REQUIRE_OK(kefir_json_output_string(json, "type_enumeration"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_STRUCTURE:
            REQUIRE_OK(kefir_json_output_string(json, "type_structure"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_UNION:
            REQUIRE_OK(kefir_json_output_string(json, "type_union"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_ARRAY:
            REQUIRE_OK(kefir_json_output_string(json, "type_array"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_FUNCTION:
            REQUIRE_OK(kefir_json_output_string(json, "type_function"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_TYPE_BUILTIN:
            REQUIRE_OK(kefir_json_output_string(json, "type_builtin"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_ENUMERATOR:
            REQUIRE_OK(kefir_json_output_string(json, "enumeration_enumerator"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER:
            REQUIRE_OK(kefir_json_output_string(json, "structure_member"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER:
            REQUIRE_OK(kefir_json_output_string(json, "function_parameter"));
            break;

        case KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG:
            REQUIRE_OK(kefir_json_output_string(json, "function_vararg"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "attributes"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (res = kefir_ir_debug_entry_attribute_iter(&module->debug_info.entries, entry->identifier, &attr_iter, &entry_attr);
        res == KEFIR_OK;
        res = kefir_ir_debug_entry_attribute_next(&attr_iter, &entry_attr)) {
        
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "tag"));
        switch (entry_attr->tag) {
            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_NAME:
                REQUIRE_OK(kefir_json_output_string(json, "name"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_string(json, entry_attr->name));
                break;

            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_SIZE:
                REQUIRE_OK(kefir_json_output_string(json, "size"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_uinteger(json, entry_attr->size));
                break;

            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_ALIGNMENT:
                REQUIRE_OK(kefir_json_output_string(json, "alignment"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_uinteger(json, entry_attr->alignment));
                break;

            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_LENGTH:
                REQUIRE_OK(kefir_json_output_string(json, "length"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_uinteger(json, entry_attr->length));
                break;

            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_CONSTANT_UINT:
                REQUIRE_OK(kefir_json_output_string(json, "constant_uint"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_uinteger(json, entry_attr->constant_uint));
                break;

            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_OFFSET:
                REQUIRE_OK(kefir_json_output_string(json, "offset"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_uinteger(json, entry_attr->offset));
                break;

            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITWIDTH:
                REQUIRE_OK(kefir_json_output_string(json, "bitwidth"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_uinteger(json, entry_attr->bitwidth));
                break;

            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_BITOFFSET:
                REQUIRE_OK(kefir_json_output_string(json, "bitoffset"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_uinteger(json, entry_attr->bitoffset));
                break;

            case KEFIR_IR_DEBUG_ENTRY_ATTRIBUTE_TYPE:
                REQUIRE_OK(kefir_json_output_string(json, "type"));
                REQUIRE_OK(kefir_json_output_object_key(json, "value"));
                REQUIRE_OK(kefir_json_output_uinteger(json, entry_attr->entry_id));
                break;
        }

        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    struct kefir_ir_debug_entry_child_iterator child_iter;
    REQUIRE_OK(kefir_json_output_object_key(json, "children"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (res = kefir_ir_debug_entry_child_iter(&module->debug_info.entries, entry->identifier, &child_iter, &child_entry_id);
        res == KEFIR_OK;
        res = kefir_ir_debug_entry_child_next(&child_iter, &child_entry_id)) {
        
        const struct kefir_ir_debug_entry *child_entry;
        REQUIRE_OK(kefir_ir_debug_entry_get(&module->debug_info.entries, child_entry_id, &child_entry));
        REQUIRE_OK(format_debug_entry(json, module, child_entry));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    
    REQUIRE_OK(kefir_json_output_object_end(json));
    
    return KEFIR_OK;
}

static kefir_result_t format_debug_entries(struct kefir_json_output *json, const struct kefir_ir_module *module) {
    struct kefir_ir_debug_entry_iterator iter;
    kefir_result_t res;
    kefir_ir_debug_entry_id_t entry_id;

    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (res = kefir_ir_debug_entry_iter(&module->debug_info.entries, &iter, &entry_id);
        res == KEFIR_OK;
        res = kefir_ir_debug_entry_next(&iter, &entry_id)) {

        const struct kefir_ir_debug_entry *entry;
        REQUIRE_OK(kefir_ir_debug_entry_get(&module->debug_info.entries, entry_id, &entry));

        REQUIRE_OK(format_debug_entry(json, module, entry));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_module_json(struct kefir_json_output *json, const struct kefir_ir_module *module,
                                           kefir_bool_t debug_info) {
    REQUIRE_OK(kefir_json_output_object_begin(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "identifiers"));
    REQUIRE_OK(format_identifiers(json, module, debug_info));
    REQUIRE_OK(kefir_json_output_object_key(json, "types"));
    REQUIRE_OK(format_types(json, module));
    REQUIRE_OK(kefir_json_output_object_key(json, "data"));
    REQUIRE_OK(format_data(json, module));
    REQUIRE_OK(kefir_json_output_object_key(json, "string_literals"));
    REQUIRE_OK(format_string_literal(json, module));
    REQUIRE_OK(kefir_json_output_object_key(json, "function_declarations"));
    REQUIRE_OK(format_function_declarations(json, module));
    REQUIRE_OK(kefir_json_output_object_key(json, "functions"));
    REQUIRE_OK(format_functions(json, module, debug_info));
    REQUIRE_OK(kefir_json_output_object_key(json, "inline_assembly"));
    REQUIRE_OK(format_inline_assembly(json, module));

    if (debug_info) {
        REQUIRE_OK(kefir_json_output_object_key(json, "debug_info"));
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "entries"));
        REQUIRE_OK(format_debug_entries(json, module));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }

    REQUIRE_OK(kefir_json_output_object_key(json, "meta_info"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "opcode_rev"));
    REQUIRE_OK(kefir_json_output_uinteger(json, KEFIR_IR_OPCODES_REVISION));
    REQUIRE_OK(kefir_json_output_object_end(json));

    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_finalize(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ir_format_module(FILE *fp, const struct kefir_ir_module *module, kefir_bool_t debug_info) {
    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, fp, 4));
    REQUIRE_OK(kefir_ir_format_module_json(&json, module, debug_info));
    REQUIRE_OK(kefir_json_output_finalize(&json));
    return KEFIR_OK;
}
