#ifndef KEFIR_TEST_MODULE_H_
#define KEFIR_TEST_MODULE_H_

#include "kefir/ir/module.h"

kefir_result_t kefir_ir_module_declare_global(struct kefir_mem *, struct kefir_ir_module *, const char *,
                                              kefir_ir_identifier_type_t);
kefir_result_t kefir_ir_module_declare_external(struct kefir_mem *, struct kefir_ir_module *, const char *,
                                                kefir_ir_identifier_type_t);

#endif
