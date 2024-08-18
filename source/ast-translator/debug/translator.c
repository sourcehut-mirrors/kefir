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

#include "kefir/ast-translator/debug/translator.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast/type_completion.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_ast_translator_debug_entries_init(struct kefir_ast_translator_debug_entries *type_bundle) {
    REQUIRE(type_bundle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST translator debug type bundle"));

    REQUIRE_OK(kefir_hashtree_init(&type_bundle->type_index, &kefir_hashtree_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_debug_entries_free(struct kefir_mem *mem, struct kefir_ast_translator_debug_entries *type_bundle) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type_bundle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator debug type bundle"));
    
    REQUIRE_OK(kefir_hashtree_free(mem, &type_bundle->type_index));
    memset(type_bundle, 0, sizeof(struct kefir_ast_translator_debug_entries));
    return KEFIR_OK;
}

static kefir_result_t kefir_ast_translate_debug_type_with_layout(struct kefir_mem *, const struct kefir_ast_context *,
                                              const struct kefir_ast_translator_environment *,
                                              struct kefir_ir_module *,
                                              struct kefir_ast_translator_debug_entries *,
                                              const struct kefir_ast_type *,
                                              const struct kefir_ast_type_layout *,
                                              kefir_ir_debug_entry_id_t *);

static kefir_result_t translate_debug_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const struct kefir_ast_translator_environment *translator_env,
                                              struct kefir_ir_module *module,
                                              struct kefir_ast_translator_debug_entries *debug_entries,
                                              const struct kefir_ast_type *type,
                                              const struct kefir_ast_type_layout *type_layout,
                                              kefir_ir_debug_entry_id_t *entry_id_ptr) {
    switch (type->tag) {
        case KEFIR_AST_TYPE_VOID:
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_VOID, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("void")));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_BOOLEAN, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("bool")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_CHARACTER, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("char")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_CHARACTER, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("unsigned char")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_CHARACTER, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("signed char")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("unsigned short")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_INT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("signed short")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("unsigned int")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_INT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("signed int")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("unsigned long")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_INT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("signed long")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_UNSIGNED_INT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("unsigned long long")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_SIGNED_INT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("signed long long")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_FLOAT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("float")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_FLOAT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("double")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_FLOAT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("long double")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_COMPLEX_FLOAT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("_Complex float")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_COMPLEX_FLOAT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("_Complex double")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_COMPLEX_FLOAT, entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("_Complex long double")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            break;

        case KEFIR_AST_TYPE_SCALAR_POINTER: {
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_POINTER, entry_id_ptr));

            kefir_ir_debug_entry_id_t referenced_entry_type_id;
            REQUIRE_OK(kefir_ast_translate_debug_type(mem, context, translator_env, module, debug_entries, type->referenced_type, &referenced_entry_type_id));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(referenced_entry_type_id)));
        } break;
        
        case KEFIR_AST_TYPE_ENUMERATION: {
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_ENUMERATION, entry_id_ptr));

            if (type->enumeration_type.identifier != NULL) {
                REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME(type->enumeration_type.identifier)));
            }

            if (type->enumeration_type.complete) {
                kefir_uint64_t next_value = 0;
                for (const struct kefir_list_entry *iter = kefir_list_head(&type->enumeration_type.enumerators); iter != NULL;
                    kefir_list_next(&iter)) {
                    ASSIGN_DECL_CAST(struct kefir_ast_enum_enumerator *, enumerator, iter->value);
                    kefir_uint64_t value = next_value;
                    if (enumerator->value != NULL) {
                        value = enumerator->value->value.uinteger;
                    }
                    next_value = value + 1;

                    kefir_ir_debug_entry_id_t child_entry_id;
                    REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, &module->debug_info.entries, *entry_id_ptr, KEFIR_IR_DEBUG_ENTRY_ENUMERATOR, &child_entry_id));
                    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, child_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME(enumerator->identifier)));                    
                    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, child_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_CONSTANT_UINT(value)));                    
                }
            }

            kefir_ir_debug_entry_id_t underlying_entry_type_id;
            REQUIRE_OK(kefir_ast_translate_debug_type(mem, context, translator_env, module, debug_entries, type->enumeration_type.underlying_type, &underlying_entry_type_id));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(underlying_entry_type_id)));
        } break;
        
        case KEFIR_AST_TYPE_ARRAY: {
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_ARRAY, entry_id_ptr));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            if (type->array_type.boundary == KEFIR_AST_ARRAY_BOUNDED || type->array_type.boundary == KEFIR_AST_ARRAY_BOUNDED_STATIC) {
                REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
                REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
                REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_LENGTH(type->array_type.const_length->value.uinteger)));
            }

            kefir_ir_debug_entry_id_t element_entry_type_id;
            if (KEFIR_AST_TYPE_IS_VL_ARRAY(type)) {
                REQUIRE_OK(kefir_ast_translate_debug_type(mem, context, translator_env, module, debug_entries, type->array_type.element_type, &element_entry_type_id));
            } else {
                REQUIRE_OK(kefir_ast_translate_debug_type_with_layout(mem, context, translator_env, module, debug_entries, type_layout->array_layout.element_type->type, type_layout->array_layout.element_type, &element_entry_type_id));
            }
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(element_entry_type_id)));
        } break;

        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION:
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, type->tag == KEFIR_AST_TYPE_STRUCTURE ? KEFIR_IR_DEBUG_ENTRY_TYPE_STRUCTURE : KEFIR_IR_DEBUG_ENTRY_TYPE_UNION, entry_id_ptr));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));

            if (type->structure_type.identifier != NULL) {
                REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME(type->structure_type.identifier)));                    
            }

            if (type->structure_type.complete) {
                REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
                REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
                REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(type_layout->properties.alignment)));

                for (const struct kefir_list_entry *iter = kefir_list_head(&type_layout->structure_layout.member_list); iter != NULL;
                    kefir_list_next(&iter)) {
                    ASSIGN_DECL_CAST(const struct kefir_ast_type_layout_structure_member *, field,
                        iter->value);
                    
                    kefir_ir_debug_entry_id_t field_entry_id;
                    REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, &module->debug_info.entries, *entry_id_ptr, KEFIR_IR_DEBUG_ENTRY_STRUCTURE_MEMBER, &field_entry_id));

                    if (field->identifier != NULL) {
                        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, field_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME(field->identifier)));                    
                    }

                    if (field->layout->bitfield) {
                        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, field_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_BITWIDTH(field->layout->bitfield_props.width)));
                        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, field_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_BITOFFSET(field->layout->bitfield_props.offset + field->layout->properties.relative_offset * 8)));
                    } else {
                        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, field_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(field->layout->properties.size)));
                        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, field_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(field->layout->properties.alignment)));
                        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, field_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_OFFSET(field->layout->properties.relative_offset)));
                    }

                    kefir_ir_debug_entry_id_t field_entry_type_id;
                    REQUIRE_OK(kefir_ast_translate_debug_type_with_layout(mem, context, translator_env, module, debug_entries, field->layout->type, field->layout, &field_entry_type_id));
                    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, field_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(field_entry_type_id)));
                }
            }
            break;

        case KEFIR_AST_TYPE_FUNCTION:
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_FUNCTION, entry_id_ptr));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));

            kefir_ir_debug_entry_id_t return_entry_type_id, parameter_entry_id;
            REQUIRE_OK(kefir_ast_translate_debug_type(mem, context, translator_env, module, debug_entries, type->function_type.return_type, &return_entry_type_id));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(return_entry_type_id)));

            for (kefir_size_t i = 0; i < kefir_ast_type_function_parameter_count(&type->function_type); i++) {
                const struct kefir_ast_function_type_parameter *parameter;
                REQUIRE_OK(kefir_ast_type_function_get_parameter(&type->function_type, i, &parameter));

                REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, &module->debug_info.entries, *entry_id_ptr, KEFIR_IR_DEBUG_ENTRY_FUNCTION_PARAMETER, &parameter_entry_id));

                if (parameter->type != NULL) {
                    kefir_ir_debug_entry_id_t parameter_entry_type_id;
                    REQUIRE_OK(kefir_ast_translate_debug_type(mem, context, translator_env, module, debug_entries, parameter->type, &parameter_entry_type_id));
                    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, parameter_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(parameter_entry_type_id)));
                }

                if (parameter->name != NULL) {
                    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, parameter_entry_id, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME(parameter->name)));                    
                }
            }

            if (type->function_type.ellipsis) {
                REQUIRE_OK(kefir_ir_debug_entry_new_child(mem, &module->debug_info.entries, *entry_id_ptr, KEFIR_IR_DEBUG_ENTRY_FUNCTION_VARARG, &parameter_entry_id));
            }
            break;

        case KEFIR_AST_TYPE_VA_LIST:
            REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid AST type layout"));
            REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_TYPE_BUILTIN, entry_id_ptr));
            REQUIRE_OK(kefir_hashtree_insert(mem, &debug_entries->type_index, (kefir_hashtree_key_t) type, (kefir_hashtree_value_t) *entry_id_ptr));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME("__builtin_va_list")));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_SIZE(type_layout->properties.size)));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols, *entry_id_ptr, &KEFIR_IR_DEBUG_ENTRY_ATTR_ALIGNMENT(type_layout->properties.alignment)));
            break;

        case KEFIR_AST_TYPE_QUALIFIED:
        case KEFIR_AST_TYPE_AUTO:   
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST type");
    }

    return KEFIR_OK;
}

static kefir_result_t kefir_ast_translate_debug_type_with_layout(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const struct kefir_ast_translator_environment *translator_env,
                                              struct kefir_ir_module *module,
                                              struct kefir_ast_translator_debug_entries *debug_entries,
                                              const struct kefir_ast_type *type,
                                              const struct kefir_ast_type_layout *type_layout,
                                              kefir_ir_debug_entry_id_t *entry_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(translator_env != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator environment"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(debug_entries != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator debug entries"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(entry_id_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR debug entry identifier"));

    type = kefir_ast_unqualified_type(type);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to obtain unqualified AST type"));

    REQUIRE(type_layout == NULL || type_layout->type == type, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected AST type layout match"));
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &type, type));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&debug_entries->type_index, (kefir_hashtree_key_t) type, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(kefir_ir_debug_entry_id_t, entry_id,
            node->value);
        *entry_id_ptr = entry_id;
    } else {
        kefir_bool_t free_env_type = false;
        struct kefir_ast_translator_environment_type env_type;
        if (type_layout == NULL && kefir_ast_type_is_complete(type) && type->tag != KEFIR_AST_TYPE_VOID && type->tag != KEFIR_AST_TYPE_FUNCTION) {
            REQUIRE_OK(kefir_ast_translator_environment_new_type(mem, context, translator_env, type, &env_type, NULL));
            free_env_type = true;
            type_layout = env_type.layout;
        }

        kefir_ir_debug_entry_id_t entry_id;
        kefir_result_t res = translate_debug_type(mem, context, translator_env, module, debug_entries, type, type_layout, &entry_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            if (free_env_type) {
                kefir_ast_translator_environment_free_type(mem, translator_env, &env_type);
            }
            return res;
        });
        if (free_env_type) {
            REQUIRE_OK(kefir_ast_translator_environment_free_type(mem, translator_env, &env_type));
        }
        *entry_id_ptr = entry_id;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_debug_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const struct kefir_ast_translator_environment *translator_env,
                                              struct kefir_ir_module *module,
                                              struct kefir_ast_translator_debug_entries *debug_entries,
                                              const struct kefir_ast_type *type,
                                              kefir_ir_debug_entry_id_t *entry_id_ptr) {
    return kefir_ast_translate_debug_type_with_layout(mem, context, translator_env, module, debug_entries, type, NULL, entry_id_ptr);
}
