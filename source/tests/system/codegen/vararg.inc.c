static kefir_result_t generate_va_list_type(struct kefir_mem *mem, struct kefir_ir_type *type) {
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_ARRAY, 0, 1));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_STRUCT, 0, 4));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type, KEFIR_IR_TYPE_WORD, 0, 0));
    return KEFIR_OK;
}
