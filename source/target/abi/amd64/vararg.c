#include "kefir/target/abi/amd64/vararg.h"
#include "kefir/target/abi/amd64/parameters.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_abi_amd64_vararg_save_area_requirements(kefir_abi_amd64_variant_t abi_variant, kefir_size_t *size,
                                                             kefir_size_t *alignment) {
    switch (abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            ASSIGN_PTR(size,
                       kefir_abi_amd64_num_of_general_purpose_parameter_registers(abi_variant) * KEFIR_AMD64_ABI_QWORD +
                           kefir_abi_amd64_num_of_sse_parameter_registers(abi_variant) * 2 * KEFIR_AMD64_ABI_QWORD);
            ASSIGN_PTR(alignment, 2 * KEFIR_AMD64_ABI_QWORD);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }

    return KEFIR_OK;
}
