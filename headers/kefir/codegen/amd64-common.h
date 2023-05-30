#ifndef KEFIR_CODEGEN_AMD64_COMMON_H_
#define KEFIR_CODEGEN_AMD64_COMMON_H_

#define KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_PREFIX "x86_64-intel_prefix"
#define KEFIR_CODEGEN_SYNTAX_X86_64_INTEL_NOPREFIX "x86_64-intel_noprefix"
#define KEFIR_CODEGEN_SYNTAX_X86_64_ATT "x86_64-att"

#include "kefir/codegen/codegen.h"
#include "kefir/target/asm/amd64/xasmgen.h"

kefir_result_t kefir_codegen_match_syntax(const char *, kefir_asm_amd64_xasmgen_syntax_t *);

#endif
