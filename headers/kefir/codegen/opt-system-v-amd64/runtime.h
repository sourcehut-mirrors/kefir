#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_RUNTIME_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_RUNTIME_H_

#include "kefir/core/basic-types.h"

#define KEFIR_AMD64_EMUTLS_V "__emutls_v.%s"
#define KEFIR_AMD64_EMUTLS_T "__emutls_t.%s"
#define KEFIR_AMD64_SYSTEM_V_STRING_LITERAL "__kefir_string_literal" KEFIR_ID_FMT
#define KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_STRING_LITERAL "__kefirrt_string_literal" KEFIR_ID_FMT
#define KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK "__kefir_func_%s_block" KEFIR_ID_FMT
#define KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL "__kefir_func_%s_block" KEFIR_ID_FMT "_label" KEFIR_ID_FMT

#endif
