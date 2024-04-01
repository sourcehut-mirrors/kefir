#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/layout.h"
#include "kefir/ast/type_layout.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/core/util.h"

struct param {
    struct kefir_mem *mem;
    const struct kefir_ast_node_base *base;
    struct kefir_irbuilder_block *builder;
    struct kefir_ast_translator_context *context;
    struct kefir_ast_type_layout *type_layout;
};

static kefir_result_t visit_non_designator(const struct kefir_ast_visitor *visitor,
                                           const struct kefir_ast_node_base *base, void *payload) {
    UNUSED(visitor);
    UNUSED(base);
    UNUSED(payload);
    return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location, "Expected a member designator");
}

static kefir_result_t visit_identifier(const struct kefir_ast_visitor *visitor,
                                       const struct kefir_ast_identifier *identifier, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST identifier"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct param *, param, payload);

    struct kefir_ast_designator designator = {
        .type = KEFIR_AST_DESIGNATOR_MEMBER, .member = identifier->identifier, .next = NULL};

    struct kefir_ast_type_layout *sublayout = NULL;
    REQUIRE_OK(kefir_ast_type_layout_resolve(param->type_layout, &designator, &sublayout, NULL, NULL));
    REQUIRE_OK(
        KEFIR_IRBUILDER_BLOCK_APPENDU64(param->builder, KEFIR_IROPCODE_PUSHU64, sublayout->properties.relative_offset));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(param->builder, KEFIR_IROPCODE_IADD64, 0));
    param->type_layout = sublayout;
    return KEFIR_OK;
}

static kefir_result_t visit_struct_member(const struct kefir_ast_visitor *visitor,
                                          const struct kefir_ast_struct_member *member, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    REQUIRE(member != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST structure member"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct param *, param, payload);

    REQUIRE_OK(KEFIR_AST_NODE_VISIT(visitor, member->structure, payload));

    struct kefir_ast_designator designator = {
        .type = KEFIR_AST_DESIGNATOR_MEMBER, .member = member->member, .next = NULL};

    struct kefir_ast_type_layout *sublayout = NULL;
    REQUIRE_OK(kefir_ast_type_layout_resolve(param->type_layout, &designator, &sublayout, NULL, NULL));
    REQUIRE_OK(
        KEFIR_IRBUILDER_BLOCK_APPENDU64(param->builder, KEFIR_IROPCODE_PUSHU64, sublayout->properties.relative_offset));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(param->builder, KEFIR_IROPCODE_IADD64, 0));
    param->type_layout = sublayout;
    return KEFIR_OK;
}

static kefir_result_t visit_array_subscript(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_array_subscript *subscript, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    REQUIRE(subscript != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST array subscript"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct param *, param, payload);

    REQUIRE_OK(KEFIR_AST_NODE_VISIT(visitor, subscript->array, payload));

    REQUIRE(param->type_layout->type->tag == KEFIR_AST_TYPE_ARRAY,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &subscript->array->source_location,
                                   "Expected designation of array-type element"));

    struct kefir_ast_type_layout *sublayout = param->type_layout->array_layout.element_type;
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(param->builder, KEFIR_IROPCODE_PUSHU64, sublayout->properties.size));
    REQUIRE_OK(kefir_ast_translate_expression(param->mem, subscript->subscript, param->builder, param->context));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(param->builder, KEFIR_IROPCODE_IMUL64, 0));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(param->builder, KEFIR_IROPCODE_IADD64, 0));
    param->type_layout = sublayout;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_member_designator(struct kefir_mem *mem, const struct kefir_ast_node_base *base,
                                                     const struct kefir_ast_type *type,
                                                     struct kefir_irbuilder_block *builder,
                                                     struct kefir_ast_translator_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR builder block"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));

    struct param param = {.mem = mem, .context = context, .base = base, .builder = builder};
    struct kefir_ast_visitor visitor;
    REQUIRE_OK(kefir_ast_visitor_init(&visitor, visit_non_designator));
    visitor.identifier = visit_identifier;
    visitor.struct_member = visit_struct_member;
    visitor.array_subscript = visit_array_subscript;

    struct kefir_ir_type ir_type;
    struct kefir_irbuilder_type ir_builder;
    struct kefir_ast_type_layout *layout;

    REQUIRE_OK(kefir_ir_type_alloc(mem, 0, &ir_type));
    kefir_result_t res = kefir_irbuilder_type_init(mem, &ir_builder, &ir_type);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_type_free(mem, &ir_type);
        return res;
    });

    res = kefir_ast_translate_object_type(mem, context->ast_context, type, KEFIR_AST_DEFAULT_ALIGNMENT,
                                          context->environment, &ir_builder, &layout, &base->source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_IRBUILDER_TYPE_FREE(&ir_builder);
        kefir_ir_type_free(mem, &ir_type);
        return res;
    });

    res = KEFIR_IRBUILDER_TYPE_FREE(&ir_builder);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_type_layout_free(mem, layout);
        kefir_ir_type_free(mem, &ir_type);
        return res;
    });

    res = kefir_ast_translator_evaluate_type_layout(mem, context->environment, layout, &ir_type);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_type_layout_free(mem, layout);
        kefir_ir_type_free(mem, &ir_type);
        return res;
    });

    param.type_layout = layout;
    res = KEFIR_AST_NODE_VISIT(&visitor, base, &param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_type_layout_free(mem, layout);
        kefir_ir_type_free(mem, &ir_type);
        return res;
    });

    res = kefir_ast_type_layout_free(mem, layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_type_free(mem, &ir_type);
        return res;
    });

    REQUIRE_OK(kefir_ir_type_free(mem, &ir_type));
    return KEFIR_OK;
}
