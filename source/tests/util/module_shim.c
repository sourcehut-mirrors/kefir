#include "kefir/test/module_shim.h"

kefir_result_t kefir_ir_module_declare_global(struct kefir_mem *mem, struct kefir_ir_module *module, const char *symbol,
                                              kefir_ir_identifier_type_t type) {
    struct kefir_ir_identifier identifier = {.type = type,
                                             .scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT,
                                             .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                                             .alias = NULL};

    return kefir_ir_module_declare_identifier(mem, module, symbol, &identifier);
}

kefir_result_t kefir_ir_module_declare_external(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                const char *symbol, kefir_ir_identifier_type_t type) {
    struct kefir_ir_identifier identifier = {.type = type,
                                             .scope = KEFIR_IR_IDENTIFIER_SCOPE_IMPORT,
                                             .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                                             .alias = NULL};

    return kefir_ir_module_declare_identifier(mem, module, symbol, &identifier);
}

kefir_result_t kefir_ir_module_declare_local(struct kefir_mem *mem, struct kefir_ir_module *module, const char *symbol,
                                              kefir_ir_identifier_type_t type) {
    struct kefir_ir_identifier identifier = {.type = type,
                                             .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                                             .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                                             .alias = NULL};

    return kefir_ir_module_declare_identifier(mem, module, symbol, &identifier);
}
