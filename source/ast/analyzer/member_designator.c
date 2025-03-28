#include "kefir/ast/analyzer/member_designator.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/core/util.h"

struct param {
    struct kefir_mem *mem;
    const struct kefir_ast_context *context;
    const struct kefir_ast_type *type;
    struct kefir_ast_node_base *base;
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

    REQUIRE(param->type->tag == KEFIR_AST_TYPE_STRUCTURE || param->type->tag == KEFIR_AST_TYPE_UNION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &identifier->base.source_location,
                                   "Expected designation for AST structure or union type"));

    const struct kefir_ast_struct_field *field = NULL;
    kefir_result_t res =
        kefir_ast_struct_type_resolve_field(&param->type->structure_type, identifier->identifier, &field);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &identifier->base.source_location,
                                      "Unable to find member '%s' in structure", identifier->identifier);
    }
    REQUIRE_OK(res);

    param->base->properties.category = KEFIR_AST_NODE_CATEGORY_MEMBER_DESIGNATOR;
    param->base->properties.type = field->type;
    return KEFIR_OK;
}

static kefir_result_t visit_struct_member(const struct kefir_ast_visitor *visitor,
                                          const struct kefir_ast_struct_member *member, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    REQUIRE(member != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST struct member"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct param *, param, payload);

    REQUIRE_OK(kefir_ast_analyze_member_designator(param->mem, param->context, param->type, member->structure));
    REQUIRE(member->structure->properties.category == KEFIR_AST_NODE_CATEGORY_MEMBER_DESIGNATOR,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &member->structure->source_location,
                                   "Expected member designator"));
    const struct kefir_ast_type *base_type = member->structure->properties.type;

    REQUIRE(base_type->tag == KEFIR_AST_TYPE_STRUCTURE || base_type->tag == KEFIR_AST_TYPE_UNION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &member->base.source_location,
                                   "Expected designation for AST structure or union type"));

    const struct kefir_ast_struct_field *field = NULL;
    kefir_result_t res = kefir_ast_struct_type_resolve_field(&base_type->structure_type, member->member, &field);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &member->base.source_location,
                                      "Unable to find member '%s' in structure", member->member);
    }
    REQUIRE_OK(res);

    param->base->properties.category = KEFIR_AST_NODE_CATEGORY_MEMBER_DESIGNATOR;
    param->base->properties.type = field->type;
    return KEFIR_OK;
}

static kefir_result_t visit_array_subscript(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_array_subscript *subscript, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    REQUIRE(subscript != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST array subscript"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct param *, param, payload);

    REQUIRE_OK(kefir_ast_analyze_member_designator(param->mem, param->context, param->type, subscript->array));
    REQUIRE(
        subscript->array->properties.category == KEFIR_AST_NODE_CATEGORY_MEMBER_DESIGNATOR,
        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &subscript->array->source_location, "Expected member designator"));
    const struct kefir_ast_type *base_type = subscript->array->properties.type;

    REQUIRE(base_type->tag == KEFIR_AST_TYPE_ARRAY,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &subscript->array->source_location,
                                   "Expected designation for AST array type"));
    const struct kefir_ast_type *type = base_type->array_type.element_type;

    REQUIRE_OK(kefir_ast_analyze_node(param->mem, param->context, subscript->subscript));
    REQUIRE(subscript->subscript->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &subscript->subscript->source_location,
                                   "Expected array subscript"));

    param->base->properties.category = KEFIR_AST_NODE_CATEGORY_MEMBER_DESIGNATOR;
    param->base->properties.type = type;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_member_designator(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                   const struct kefir_ast_type *type,
                                                   struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));

    struct param param = {.mem = mem, .context = context, .type = type, .base = base};
    struct kefir_ast_visitor visitor;
    REQUIRE_OK(kefir_ast_visitor_init(&visitor, visit_non_designator));
    visitor.identifier = visit_identifier;
    visitor.struct_member = visit_struct_member;
    visitor.array_subscript = visit_array_subscript;
    REQUIRE_OK(KEFIR_AST_NODE_VISIT(&visitor, base, &param));
    return KEFIR_OK;
}
