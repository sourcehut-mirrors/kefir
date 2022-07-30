#include "kefir/codegen/amd64/system-v/inline_assembly.h"
#include "kefir/codegen/amd64/system-v/runtime.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_sysv_inline_assembly_invoke(struct kefir_mem *mem,
                                                               struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                               struct kefir_codegen_amd64 *codegen,
                                                               const struct kefir_ir_inline_assembly *inline_asm) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(sysv_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen AMD64 System-V module"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen AMD64 codegen"));
    REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR inline assembly"));

    if (!kefir_hashtree_has(&sysv_module->inline_assembly, inline_asm->id)) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &sysv_module->inline_assembly, (kefir_hashtree_key_t) inline_asm->id,
                                         (kefir_hashtree_value_t) inline_asm));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
        kefir_amd64_xasmgen_operand_label(
            &codegen->xasmgen_helpers.operands[0],
            kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                               KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_FRAGMENT, inline_asm->id)),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
    return KEFIR_OK;
}
