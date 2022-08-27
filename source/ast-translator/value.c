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

#include "kefir/ast-translator/value.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ast-translator/type.h"
#include "kefir/ast/type_conv.h"
#include "kefir/ast/downcast.h"
#include "kefir/ast/type_completion.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_uint64_t retrieve_memflags(const struct kefir_ast_type_qualification *qualification) {
    kefir_uint64_t mem_flags = KEFIR_IR_MEMORY_FLAG_NONE;
    if (qualification->volatile_type) {
        mem_flags |= KEFIR_IR_MEMORY_FLAG_VOLATILE;
    }
    return mem_flags;
}

static kefir_result_t load_bitfield(struct kefir_irbuilder_block *builder, struct kefir_ast_type_layout *layout,
                                    const struct kefir_ir_type *ir_type,
                                    const struct kefir_ir_typeentry **typeentry_ptr) {
    kefir_uint64_t mem_flags = retrieve_memflags(&layout->type_qualification);
    struct kefir_ir_typeentry *typeentry = kefir_ir_type_at(ir_type, layout->value);
    REQUIRE(typeentry->typecode == KEFIR_IR_TYPE_BITS,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected a bit-field"));
    ASSIGN_PTR(typeentry_ptr, typeentry);

    kefir_size_t byte_offset = layout->bitfield_props.offset / 8;
    kefir_size_t bit_offset = layout->bitfield_props.offset % 8;

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IADD1, byte_offset));

    kefir_size_t bits = bit_offset + layout->bitfield_props.width;
    if (bits <= 8) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD8U, mem_flags));
    } else if (bits <= 16) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD16U, mem_flags));
    } else if (bits <= 32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD32U, mem_flags));
    } else if (bits <= 64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD64, mem_flags));
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
    REQUIRE_CHAIN(&res, load_bitfield(builder, member_layout, translator_type->object.ir_type, NULL));

    kefir_bool_t signedness = false;
    REQUIRE_CHAIN(&res, kefir_ast_type_is_signed(context->ast_context->type_traits, member_layout->type, &signedness));

    kefir_size_t bit_offset = member_layout->bitfield_props.offset % 8;
    if (signedness) {
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IROPCODE_EXTSBITS, bit_offset,
                                                            member_layout->bitfield_props.width));
    } else {
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IROPCODE_EXTUBITS, bit_offset,
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
                                     struct kefir_ast_type_layout *member_layout) {
    kefir_uint64_t mem_flags = retrieve_memflags(&member_layout->type_qualification);
    const struct kefir_ir_typeentry *typeentry = NULL;

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PICK, 1));
    REQUIRE_OK(load_bitfield(builder, member_layout, ir_type, &typeentry));

    kefir_size_t byte_offset = member_layout->bitfield_props.offset / 8;
    kefir_size_t bit_offset = member_layout->bitfield_props.offset % 8;

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IROPCODE_INSERTBITS, bit_offset,
                                               member_layout->bitfield_props.width));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IADD1, byte_offset));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_XCHG, 1));

    kefir_size_t bits = bit_offset + member_layout->bitfield_props.width;
    if (bits <= 8) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORE8, mem_flags));
    } else if (bits <= 16) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORE16, mem_flags));
    } else if (bits <= 32) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORE32, mem_flags));
    } else if (bits <= 64) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORE64, mem_flags));
    } else {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Bit-field exceeds storage unit width");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_resolve_lvalue(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                   struct kefir_irbuilder_block *builder,
                                                   const struct kefir_ast_node_base *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));

    if (node->properties.expression_props.bitfield_props.bitfield) {
        struct kefir_ast_struct_member *struct_member = NULL;
        kefir_result_t res;
        REQUIRE_MATCH_OK(
            &res, kefir_ast_downcast_any_struct_member(node, &struct_member, false),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected bit-field node to be a direct/indirect structure member"));
        REQUIRE_OK(resolve_bitfield(mem, context, builder, struct_member));
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
        REQUIRE_OK(store_bitfield(builder, ir_type, layout));
    } else {
        REQUIRE_OK(kefir_ast_translator_store_value(mem, layout->type, context, builder, source_location));
    }
    return KEFIR_OK;
}

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
    } else {
        REQUIRE_OK(
            kefir_ast_translator_store_value(mem, node->properties.type, context, builder, &node->source_location));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_load_value(const struct kefir_ast_type *type,
                                               const struct kefir_ast_type_traits *type_traits,
                                               struct kefir_irbuilder_block *builder) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));

    kefir_uint64_t mem_flags = type->tag == KEFIR_AST_TYPE_QUALIFIED
                                   ? retrieve_memflags(&type->qualified_type.qualification)
                                   : KEFIR_IR_MEMORY_FLAG_NONE;
    const struct kefir_ast_type *normalizer = kefir_ast_translator_normalize_type(type);

    switch (normalizer->tag) {
        case KEFIR_AST_TYPE_VOID:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot load variable with void type");

        case KEFIR_AST_TYPE_SCALAR_BOOL:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD8U, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
            if (type_traits->character_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD8I, mem_flags));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD8U, mem_flags));
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD8U, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD8I, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD16U, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD16I, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD32U, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD32I, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD64, mem_flags));
            break;

        case KEFIR_AST_TYPE_ENUMERATION:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected enumeration type");

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION:
        case KEFIR_AST_TYPE_ARRAY:
        case KEFIR_AST_TYPE_FUNCTION:
        case KEFIR_AST_TYPE_VA_LIST:
            // Intentionally left blank
            break;

        case KEFIR_AST_TYPE_QUALIFIED:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected qualified type");
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

    const struct kefir_ast_type *normalizer = kefir_ast_translator_normalize_type(type);
    kefir_uint64_t mem_flags = type->tag == KEFIR_AST_TYPE_QUALIFIED
                                   ? retrieve_memflags(&type->qualified_type.qualification)
                                   : KEFIR_IR_MEMORY_FLAG_NONE;

    switch (normalizer->tag) {
        case KEFIR_AST_TYPE_VOID:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot store value with void type");

        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORE8, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORE16, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORE32, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORE64, mem_flags));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_STORELD, mem_flags));
            break;

        case KEFIR_AST_TYPE_ENUMERATION:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected enumeration type");

        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION:
        case KEFIR_AST_TYPE_ARRAY: {
            struct kefir_ast_translator_type *translator_type = NULL;
            REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                                     type, 0, &translator_type, source_location));

            kefir_result_t res =
                KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IROPCODE_BCOPY, translator_type->object.ir_type_id,
                                                translator_type->object.layout->value);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_translator_type_free(mem, translator_type);
                return res;
            });
            REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
        } break;

        case KEFIR_AST_TYPE_FUNCTION:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Cannot store value with function type");

        case KEFIR_AST_TYPE_QUALIFIED:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected qualified type");

        case KEFIR_AST_TYPE_VA_LIST:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_VARARG_COPY, 0));
            break;
    }
    return KEFIR_OK;
}
