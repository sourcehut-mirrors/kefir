#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_RUNTIME_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_RUNTIME_H_

#include "kefir/core/basic-types.h"

#define KEFIR_AMD64_THREAD_LOCAL "%s@tpoff"
#define KEFIR_AMD64_THREAD_LOCAL_GOT "%s@gottpoff"
#define KEFIR_AMD64_EMUTLS_V "__emutls_v.%s"
#define KEFIR_AMD64_EMUTLS_T "__emutls_t.%s"
#define KEFIR_AMD64_EMUTLS_GOT "__emutls_v.%s@GOTPCREL"
#define KEFIR_AMD64_EMUTLS_GET_ADDR "__emutls_get_address@PLT"
#define KEFIR_AMD64_SYSTEM_V_STRING_LITERAL "__kefir_string_literal" KEFIR_ID_FMT
#define KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_STRING_LITERAL "__kefirrt_string_literal" KEFIR_ID_FMT
#define KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK "__kefir_func_%s_block" KEFIR_ID_FMT
#define KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL "__kefir_func_%s_block" KEFIR_ID_FMT "_label" KEFIR_ID_FMT
#define KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_LABEL "__kefir_func_%s_const" KEFIR_ID_FMT
#define KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_FLOAT32_NEG "__kefir_func_%s_f32neg"
#define KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_FLOAT64_NEG "__kefir_func_%s_f64neg"

#endif
