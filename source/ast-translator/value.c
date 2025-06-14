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

#include "kefir/ast-translator/value.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ast-translator/type.h"
#include "kefir/ast-translator/temporaries.h"
#include "kefir/ast/type_conv.h"
#include "kefir/ast/downcast.h"
#include "kefir/ast/type_completion.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_uint64_t retrieve_memflags(const struct kefir_ast_type *type) {
    kefir_uint64_t mem_flags = KEFIR_IR_MEMORY_FLAG_NONE;
    if (type != NULL && type->tag == KEFIR_AST_TYPE_QUALIFIED && type->qualified_type.qualification.volatile_type) {
        mem_flags |= KEFIR_IR_MEMORY_FLAG_VOLATILE;
    }
    return mem_flags;
}

static kefir_result_t load_bitfield(struct kefir_irbuilder_block *builder, struct kefir_ast_type_layout *layout,
                                    const struct kefir_ir_type *ir_type,
                                    const struct kefir_ast_translator_configuration *config,
                                    const struct kefir_ir_typeentry **typeentry_ptr) {
    const kefir_uint64_t mem_flags = retrieve_memflags(layout->qualified_type);
    struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(ir_type, layout->value);
    REQUIRE(typeentry->typecode == KEFIR_IR_TYPE_BITFIELD,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected a bit-field"));
    ASSIGN_PTR(typeentry_ptr, typeentry);

    kefir_size_t byte_offset = layout->bitfield_props.offset / 8;
    kefir_size_t bit_offset = layout->bitfield_props.offset % 8;

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, byte_offset));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));

    kefir_size_t bits = bit_offset + layout->bitfield_props.width;
    if (bits <= 8) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xff));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
    } else if (bits <= 16) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffff));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
    } else if (bits <= 24 && config->precise_bitfield_load_store) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xff));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 16));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_LSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffff));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_OR, 0));
    } else if (bits <= 32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffffffULL));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
    } else if (bits <= 40 && config->precise_bitfield_load_store) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 4));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xff));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 32));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_LSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffffffULL));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_OR, 0));
    } else if (bits <= 48 && config->precise_bitfield_load_store) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 4));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffff));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 32));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_LSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffffffULL));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_OR, 0));
    } else if (bits <= 56 && config->precise_bitfield_load_store) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 6));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xff));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 48));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_LSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 4));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffff));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 32));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_LSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_LOAD, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0xffffffffULL));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_OR, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_OR, 0));
    } else if (bits <= 64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_LOAD, mem_flags));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Bit-field exceeds storage unit width");
    }
    return KEFIR_OK;
}

static kefir_result_t resolve_bitfield_layout(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                              const struct kefir_ast_struct_member *node,
                                              struct kefir_ast_translator_type **translator_type,
                                              struct kefir_ast_type_layout **member_layout) {
    const struct kefir_ast_type *structure_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->structure->properties.type);
    if (structure_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        structure_type = kefir_ast_unqualified_type(structure_type->referenced_type);
    } else if (KEFIR_AST_TYPE_IS_INCOMPLETE(structure_type)) {
        REQUIRE_OK(kefir_ast_type_completion(mem, context->ast_context, &structure_type, structure_type));
    }

    REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                             structure_type, 0, translator_type, &node->base.source_location));

    struct kefir_ast_designator designator = {
        .type = KEFIR_AST_DESIGNATOR_MEMBER, .member = node->member, .next = NULL};
    kefir_result_t res =
        kefir_ast_type_layout_resolve((*translator_type)->object.layout, &designator, member_layout, NULL, NULL);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_translator_type_free(mem, *translator_type);
        *translator_type = NULL;
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t resolve_bitfield(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                       struct kefir_irbuilder_block *builder,
                                       const struct kefir_ast_struct_member *node) {
    struct kefir_ast_translator_type *translator_type = NULL;
    struct kefir_ast_type_layout *member_layout = NULL;
    REQUIRE_OK(resolve_bitfield_layout(mem, context, node, &translator_type, &member_layout));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, load_bitfield(builder, member_layout, translator_type->object.ir_type,
                                      context->environment->configuration, NULL));

    kefir_bool_t signedness = false;
    REQUIRE_CHAIN(&res, kefir_ast_type_is_signed(context->ast_context->type_traits, member_layout->type, &signedness));

    kefir_size_t bit_offset = member_layout->bitfield_props.offset % 8;
    if (signedness) {
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, bit_offset,
                                                            member_layout->bitfield_props.width));
    } else {
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, bit_offset,
                                                            member_layout->bitfield_props.width));
    }

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_translator_type_free(mem, translator_type);
        return res;
    });
    REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
    return KEFIR_OK;
}

static kefir_result_t store_bitfield(struct kefir_irbuilder_block *builder, const struct kefir_ir_type *ir_type,
                                     struct kefir_ast_type_layout *member_layout,
                                     const struct kefir_ast_translator_configuration *configuration) {
    const kefir_uint64_t mem_flags = retrieve_memflags(member_layout->qualified_type);
    const struct kefir_ir_typeentry *typeentry = NULL;

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
    REQUIRE_OK(load_bitfield(builder, member_layout, ir_type, configuration, &typeentry));

    kefir_size_t byte_offset = member_layout->bitfield_props.offset / 8;
    kefir_size_t bit_offset = member_layout->bitfield_props.offset % 8;

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_BITS_INSERT, bit_offset,
                                               member_layout->bitfield_props.width));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, byte_offset));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));

    kefir_size_t bits = bit_offset + member_layout->bitfield_props.width;
    if (bits <= 8) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_STORE, mem_flags));
    } else if (bits <= 16) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_STORE, mem_flags));
    } else if (bits <= 24) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 16));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_RSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_STORE, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_STORE, mem_flags));
    } else if (bits <= 32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_STORE, mem_flags));
    } else if (bits <= 40) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 4));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 32));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_RSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_STORE, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_STORE, mem_flags));
    } else if (bits <= 48) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 4));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 32));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_RSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_STORE, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_STORE, mem_flags));
    } else if (bits <= 56) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 6));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 48));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_RSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_STORE, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 4));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 32));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_RSHIFT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_STORE, mem_flags));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_STORE, mem_flags));
    } else if (bits <= 64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_STORE, mem_flags));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Bit-field exceeds storage unit width");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_resolve_lvalue(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                   struct kefir_irbuilder_block *builder,
                                                   const struct kefir_ast_node_base *node,
                                                   kefir_bool_t *atomic_aggregate) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    REQUIRE(atomic_aggregate != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *atomic_aggregate = false;
    if (node->properties.expression_props.bitfield_props.bitfield) {
        struct kefir_ast_struct_member *struct_member = NULL;
        kefir_result_t res;
        REQUIRE_MATCH_OK(
            &res, kefir_ast_downcast_any_struct_member(node, &struct_member, false),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected bit-field node to be a direct/indirect structure member"));
        REQUIRE_OK(resolve_bitfield(mem, context, builder, struct_member));
    } else if (node->properties.expression_props.atomic) {
        REQUIRE_OK(kefir_ast_translator_atomic_load_value(node->properties.type, context->ast_context->type_traits,
                                                          builder, atomic_aggregate));
    } else {
        REQUIRE_OK(kefir_ast_translator_load_value(node->properties.type, context->ast_context->type_traits, builder));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_store_layout_value(struct kefir_mem *mem,
                                                       struct kefir_ast_translator_context *context,
                                                       struct kefir_irbuilder_block *builder,
                                                       const struct kefir_ir_type *ir_type,
                                                       struct kefir_ast_type_layout *layout,
                                                       const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type layout"));

    if (layout->bitfield) {
        REQUIRE_OK(store_bitfield(builder, ir_type, layout, context->environment->configuration));
    } else {
        REQUIRE_OK(kefir_ast_translator_store_value(mem, layout->qualified_type, context, builder, source_location));
    }
    return KEFIR_OK;
}

static kefir_result_t atomic_store_value(struct kefir_mem *, const struct kefir_ast_type *,
                                         struct kefir_ast_translator_context *, struct kefir_irbuilder_block *,
                                         const struct kefir_source_location *);

kefir_result_t kefir_ast_translator_store_lvalue(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                 struct kefir_irbuilder_block *builder,
                                                 const struct kefir_ast_node_base *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));

    if (node->properties.expression_props.bitfield_props.bitfield) {
        struct kefir_ast_struct_member *struct_member = NULL;
        struct kefir_ast_translator_type *translator_type = NULL;
        struct kefir_ast_type_layout *member_layout = NULL;

        kefir_result_t res;
        REQUIRE_MATCH_OK(
            &res, kefir_ast_downcast_any_struct_member(node, &struct_member, false),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected bit-field node to be a direct/indirect structure member"));

        REQUIRE_OK(resolve_bitfield_layout(mem, context, struct_member, &translator_type, &member_layout));
        REQUIRE_CHAIN(&res,
                      kefir_ast_translator_store_layout_value(mem, context, builder, translator_type->object.ir_type,
                                                              member_layout, &node->source_location));

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_translator_type_free(mem, translator_type);
            return res;
        });
        REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
    } else if (node->properties.expression_props.atomic) {
        REQUIRE_OK(atomic_store_value(mem, node->properties.type, context, builder, &node->source_location));
    } else {
        REQUIRE_OK(
            kefir_ast_translator_store_value(mem, node->properties.type, context, builder, &node->source_location));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_load_value(const struct kefir_ast_type *type,
                                               const struct kefir_ast_type_traits *type_traits,
                                               struct kefir_irbuilder_block *builder) {
    UNUSED(type_traits);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));

    REQUIRE(!KEFIR_AST_TYPE_IS_ATOMIC(type),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Atomic value loads shall use specialized function"));

    const kefir_uint64_t mem_flags = retrieve_memflags(type);
    const struct kefir_ast_type *normalizer = kefir_ast_translator_normalize_type(type);

    kefir_ast_type_data_model_classification_t normalized_data_model;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, normalizer, &normalized_data_model));
    switch (normalized_data_model) {
        case KEFIR_AST_TYPE_DATA_MODEL_VOID:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot load variable with void type");

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_LOAD, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_LOAD, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_LOAD, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_LOAD, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_LOAD, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_LOAD, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_LOAD, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_LOAD, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT: {
            kefir_bool_t is_signed;
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, normalizer, &is_signed));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32_4(builder, KEFIR_IR_OPCODE_BITINT_LOAD,
                                                         normalizer->bitprecise.width, is_signed, mem_flags, 0));
        } break;

        case KEFIR_AST_TYPE_DATA_MODEL_AGGREGATE:
        case KEFIR_AST_TYPE_DATA_MODEL_FUNCTION:
            // Intentionally left blank
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_AUTO:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected auto type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_atomic_load_value(const struct kefir_ast_type *type,
                                                      const struct kefir_ast_type_traits *type_traits,
                                                      struct kefir_irbuilder_block *builder,
                                                      kefir_bool_t *atomic_aggregate) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(atomic_aggregate != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_ast_type *normalizer = kefir_ast_translator_normalize_type(type);

    const kefir_int64_t atomic_memory_order = KEFIR_IR_MEMORY_ORDER_SEQ_CST;

    *atomic_aggregate = false;
    kefir_bool_t normalizer_signedness;
    kefir_ast_type_data_model_classification_t normalized_data_model;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, normalizer, &normalized_data_model));
    switch (normalized_data_model) {
        case KEFIR_AST_TYPE_DATA_MODEL_VOID:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot load variable with void type");

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, normalizer, &normalizer_signedness));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD8, atomic_memory_order));
            if (normalizer_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_8BITS, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, normalizer, &normalizer_signedness));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD16, atomic_memory_order));
            if (normalizer_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_16BITS, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, normalizer, &normalizer_signedness));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD32, atomic_memory_order));
            if (normalizer_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_32BITS, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD32, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD64, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD_LONG_DOUBLE, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT32,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT64,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            return KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Full bit-precise integer support is not implemented yet");

        case KEFIR_AST_TYPE_DATA_MODEL_AGGREGATE:
        case KEFIR_AST_TYPE_DATA_MODEL_FUNCTION:
            *atomic_aggregate = true;
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_AUTO:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected auto type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_load_atomic_aggregate_value(
    struct kefir_mem *mem, const struct kefir_ast_type *type, struct kefir_ast_translator_context *context,
    struct kefir_irbuilder_block *builder, const struct kefir_ast_temporary_identifier *temporary_identifier,
    const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(temporary_identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST temporary identifier"));

    REQUIRE_OK(kefir_ast_translator_fetch_temporary(mem, context, builder, temporary_identifier));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));

    struct kefir_ast_translator_type *translator_type = NULL;
    REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module, type, 0,
                                             &translator_type, source_location));

    kefir_result_t res = KEFIR_IRBUILDER_BLOCK_APPENDU32_4(
        builder, KEFIR_IR_OPCODE_ATOMIC_COPY_MEMORY_FROM, KEFIR_IR_MEMORY_ORDER_SEQ_CST,
        translator_type->object.ir_type_id, translator_type->object.layout->value, 0);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_translator_type_free(mem, translator_type);
        return res;
    });
    REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
    return KEFIR_OK;
}

static kefir_result_t atomic_store_value(struct kefir_mem *mem, const struct kefir_ast_type *type,
                                         struct kefir_ast_translator_context *context,
                                         struct kefir_irbuilder_block *builder,
                                         const struct kefir_source_location *source_location) {
    const struct kefir_ast_type *normalizer = kefir_ast_translator_normalize_type(type);

    const kefir_int64_t atomic_memory_order = KEFIR_IR_MEMORY_ORDER_SEQ_CST;

    kefir_ast_type_data_model_classification_t normalized_data_model;
    REQUIRE_OK(
        kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalizer, &normalized_data_model));
    switch (normalized_data_model) {
        case KEFIR_AST_TYPE_DATA_MODEL_VOID:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot store value with void type");

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_STORE8, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_STORE16, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_STORE32, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_STORE64, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_STORE_LONG_DOUBLE,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT32,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT64,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_STORE_COMPLEX_LONG_DOUBLE,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_AGGREGATE: {
            struct kefir_ast_translator_type *translator_type = NULL;
            REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                                     type, 0, &translator_type, source_location));

            kefir_result_t res = KEFIR_IRBUILDER_BLOCK_APPENDU32_4(
                builder, KEFIR_IR_OPCODE_ATOMIC_COPY_MEMORY_TO, atomic_memory_order, translator_type->object.ir_type_id,
                translator_type->object.layout->value, 0);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_translator_type_free(mem, translator_type);
                return res;
            });
            REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
        } break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            return KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Full bit-precise integer support is not implemented yet");

        case KEFIR_AST_TYPE_DATA_MODEL_FUNCTION:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot store value with function type");

        case KEFIR_AST_TYPE_DATA_MODEL_AUTO:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected auto type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_store_value(struct kefir_mem *mem, const struct kefir_ast_type *type,
                                                struct kefir_ast_translator_context *context,
                                                struct kefir_irbuilder_block *builder,
                                                const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));

    if (KEFIR_AST_TYPE_IS_ATOMIC(type)) {
        REQUIRE_OK(atomic_store_value(mem, type, context, builder, source_location));
        return KEFIR_OK;
    }

    const struct kefir_ast_type *normalizer = kefir_ast_translator_normalize_type(type);
    const kefir_uint64_t mem_flags = retrieve_memflags(type);

    kefir_ast_type_data_model_classification_t normalized_data_model;
    REQUIRE_OK(
        kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalizer, &normalized_data_model));
    switch (normalized_data_model) {
        case KEFIR_AST_TYPE_DATA_MODEL_VOID:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot store value with void type");

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT8_STORE, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT16_STORE, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT32_STORE, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_STORE, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_STORE, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_STORE, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_STORE, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_STORE, mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_AGGREGATE: {
            struct kefir_ast_translator_type *translator_type = NULL;
            REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                                     type, 0, &translator_type, source_location));

            kefir_result_t res = KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_COPY_MEMORY,
                                                                 translator_type->object.ir_type_id,
                                                                 translator_type->object.layout->value);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_translator_type_free(mem, translator_type);
                return res;
            });
            REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
        } break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_BITINT_STORE, type->bitprecise.width,
                                                       mem_flags));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_FUNCTION:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot store value with function type");

        case KEFIR_AST_TYPE_DATA_MODEL_AUTO:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected auto type");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_atomic_compare_exchange_value(struct kefir_mem *mem,
                                                                  const struct kefir_ast_type *type,
                                                                  struct kefir_ast_translator_context *context,
                                                                  struct kefir_irbuilder_block *builder,
                                                                  const struct kefir_source_location *source_location) {
    UNUSED(source_location);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));

    const struct kefir_ast_type *normalizer = kefir_ast_translator_normalize_type(type);
    const kefir_int64_t atomic_memory_order = KEFIR_IR_MEMORY_ORDER_SEQ_CST;

    kefir_ast_type_data_model_classification_t normalized_data_model;
    REQUIRE_OK(
        kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalizer, &normalized_data_model));
    switch (normalized_data_model) {
        case KEFIR_AST_TYPE_DATA_MODEL_VOID:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot store value with void type");

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG8, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG16, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG32, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG64, atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_LONG_DOUBLE,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT32,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT64,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE,
                                                       atomic_memory_order));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_AGGREGATE: {
            struct kefir_ast_translator_type *translator_type = NULL;
            REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                                     type, 0, &translator_type, source_location));

            kefir_result_t res = KEFIR_IRBUILDER_BLOCK_APPENDU32_4(
                builder, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_MEMORY, atomic_memory_order, translator_type->object.ir_type_id,
                translator_type->object.layout->value, 0);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_translator_type_free(mem, translator_type);
                return res;
            });
            REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
        } break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            return KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Full bit-precise integer support is not implemented yet");

        case KEFIR_AST_TYPE_DATA_MODEL_FUNCTION:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot store value with function type");

        case KEFIR_AST_TYPE_DATA_MODEL_AUTO:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected auto type");
    }
    return KEFIR_OK;
}
