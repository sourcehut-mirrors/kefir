#ifndef KEFIR_CODEGEN_AMD64_DWARF_H_
#define KEFIR_CODEGEN_AMD64_DWARF_H_

#include "kefir/codegen/amd64/codegen.h"

kefir_result_t kefir_codegen_amd64_generate_dwarf_debug_info(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                                             const struct kefir_ir_module *);

#endif
