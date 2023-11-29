# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2020-2023 Jevgenijs Protopopovs
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

.intel_syntax noprefix

.section .note.GNU-stack,"",%progbits

.section .text

.global __kefirrt_opt_amd64_sysv_vararg_save
.hidden __kefirrt_opt_amd64_sysv_vararg_save
__kefirrt_opt_amd64_sysv_vararg_save:
    test    al, al
    je      __kefirrt_save_int_registers
    movaps  [r10 + 48], xmm0
    movaps  [r10 + 64], xmm1
    movaps  [r10 + 80], xmm2
    movaps  [r10 + 96], xmm3
    movaps  [r10 + 112], xmm4
    movaps  [r10 + 128], xmm5
    movaps  [r10 + 144], xmm6
    movaps  [r10 + 160], xmm7
__kefirrt_save_int_registers:
    mov     [r10], rdi
    mov     [r10 + 8], rsi
    mov     [r10 + 16], rdx
    mov     [r10 + 24], rcx
    mov     [r10 + 32], r8
    mov     [r10 + 40], r9
    ret

.global __kefirrt_opt_load_int_vararg
.hidden __kefirrt_opt_load_int_vararg
__kefirrt_opt_load_int_vararg:
# Determine whether the argument is in reg_save or overflow area
    mov eax, [rdi]
    cmp eax, 48
    jae __kefirrt_opt_load_int_vararg_overflow
# Update gp_offset: next_gp_offset = gp_offset + 8
    lea eax, [eax + 8]
    mov [rdi], eax
# Calculate reg_save area pointer as reg_save_area + next_gp_offset - 8
    add rax, [rdi + 2*8]
    mov rax, [rax - 8]
    ret
__kefirrt_opt_load_int_vararg_overflow:
# Load current overflow area pointer
    mov rax, [rdi + 8]
# Calculate next overflow area pointer and update it
    lea rax, [rax + 8]
    mov [rdi + 8], rax
# Load from overflow area
    mov rax, [rax - 8]
    ret

.global __kefirrt_opt_load_sse_vararg
.hidden __kefirrt_opt_load_sse_vararg
__kefirrt_opt_load_sse_vararg:
# Determine whether the argument is in reg_save or overflow area
    mov eax, [rdi + 4]
    cmp eax, 176
    jae __kefirrt_opt_load_sse_vararg_overflow
# Update gp_offset: next_fp_offset = fp_offset + 16
    lea eax, [eax + 16]
    mov [rdi + 4], eax
# Calculate reg_save area pointer as reg_save_area + next_fp_offset - 16
    add rax, [rdi + 2*8]
    movaps xmm0, [rax - 16]
    ret
__kefirrt_opt_load_sse_vararg_overflow:
# Load current overflow area pointer
    mov rax, [rdi + 8]
# Calculate next overflow area pointer and update it
    lea rax, [rax + 8]
    mov [rdi + 8], rax
# Load from overflow area
    movq xmm0, [rax - 8]
    ret

.global __kefirrt_opt_float32_to_uint
.hidden __kefirrt_opt_float32_to_uint
__kefirrt_opt_float32_to_uint:
    comiss xmm0, DWORD PTR __kefirrt_opt_float32_to_uint_constant[rip]
    jnb __kefirrt_opt_float32_to_uint_overflow
    cvttss2si rax, xmm0
    ret
__kefirrt_opt_float32_to_uint_overflow:
    subss xmm0, DWORD PTR __kefirrt_opt_float32_to_uint_constant[rip]
    cvttss2si rax, xmm0
    btc rax, 63
    ret

.global __kefirrt_opt_float64_to_uint
.hidden __kefirrt_opt_float64_to_uint
__kefirrt_opt_float64_to_uint:
    comisd xmm0, QWORD PTR __kefirrt_opt_float64_to_uint_constant[rip]
    jnb __kefirrt_opt_float64_to_uint_overflow
    cvttsd2si rax, xmm0
    ret
__kefirrt_opt_float64_to_uint_overflow:
    subsd xmm0, QWORD PTR __kefirrt_opt_float64_to_uint_constant[rip]
    cvttsd2si rax, xmm0
    btc rax, 63
    ret

.global __kefirrt_opt_long_double_to_int
.hidden __kefirrt_opt_long_double_to_int
__kefirrt_opt_long_double_to_int:
    fnstcw WORD PTR [rsp - 2]
    movzx eax, WORD PTR [rsp - 2]
    or eax, 3072
    mov WORD PTR [rsp - 2], ax
    fldcw WORD PTR [rsp - 2]
    fistp QWORD PTR [rsp - 16]
    mov rax, QWORD PTR [rsp - 16]
    fldcw WORD PTR [rsp - 2]
    ret

.global __kefirrt_opt_long_double_to_uint
.hidden __kefirrt_opt_long_double_to_uint
__kefirrt_opt_long_double_to_uint:
    fld DWORD PTR __kefirrt_opt_long_double_to_uint_constant[rip]
    fstp st(2)
    fcomi st, st(1)
    jnb __kefirrt_opt_long_double_to_uint_overflow
    fstp st(1)
    fnstcw WORD PTR [rsp - 10]
    movzx eax, WORD PTR [rsp - 10]
    or ah, 12
    mov WORD PTR [rsp - 12], ax
    fldcw WORD PTR [rsp - 12]
    fistp QWORD PTR [rsp - 24]
    fldcw WORD PTR [rsp - 10]
    mov rax, QWORD PTR [rsp - 24]
    ret
__kefirrt_opt_long_double_to_uint_overflow:
    fnstcw WORD PTR [rsp - 10]
    fsubrp st(1), st
    movzx eax, WORD PTR [rsp - 10]
    or ah, 12
    mov WORD PTR [rsp - 12], ax
    fldcw WORD PTR [rsp - 12]
    fistp QWORD PTR [rsp - 24]
    fldcw WORD PTR [rsp - 10]
    mov rax, QWORD PTR [rsp - 24]
    btc rax, 63
    ret

.global __kefirrt_opt_long_double_trunc_1bit
.hidden __kefirrt_opt_long_double_trunc_1bit
__kefirrt_opt_long_double_trunc_1bit:
    xor rax, rax
    fldz
    fstp st(2)
    fucomip st(0), st(1)
    fstp st(0)
    setnp ah
    sete al
    and al, ah
    xor ah, ah
    xor rax, 1
    ret

.global __kefirrt_trap
.hidden __kefirrt_trap
__kefirrt_trap:
    ud2

__kefirrt_return_frame_address:
    mov rdx, rbp                 # Current frame address
    mov rax, QWORD PTR [rdx + 8] # Current return address
__kefirrt_return_frame_address_loop:
    cmp rdi, 0
    je __kefirrt_return_frame_address_done
    xor rax, rax
    mov rdx, QWORD PTR [rdx]
    test rdx, rdx
    jz __kefirrt_return_frame_address_done
    mov rax, QWORD PTR [rdx + 8]
    dec rdi
    jmp __kefirrt_return_frame_address_loop
__kefirrt_return_frame_address_done:
    ret

.global __kefirrt_return_address
.hidden __kefirrt_return_address
__kefirrt_return_address:
    jmp __kefirrt_return_frame_address

.global __kefirrt_frame_address
.hidden __kefirrt_frame_address
__kefirrt_frame_address:
    call __kefirrt_return_frame_address
    mov rax, rdx
    ret

.global __kefirrt_ffs
.hidden __kefirrt_ffs
__kefirrt_ffs:
    bsf eax, edi
    mov edx, -1
    cmove eax, edx
    add eax, 1
    ret

.global __kefirrt_clz
.hidden __kefirrt_clz
__kefirrt_clz:
    bsr eax, edi
    xor eax, 31
    ret

.global __kefirrt_ctz
.hidden __kefirrt_ctz
__kefirrt_ctz:
    xor eax, eax
    rep bsf eax, edi
    ret

.global __kefirrt_clrsb
.hidden __kefirrt_clrsb
__kefirrt_clrsb:
    lea eax, [rdi + rdi]
    sar edi, 31
    xor eax, edi
    or eax, 1
    bsr eax, eax
    xor eax, 31
    ret

.global __kefirrt_popcount
.hidden __kefirrt_popcount
__kefirrt_popcount:
    mov eax, edi
    shr eax
    and eax, 1431655765
    sub edi, eax
    mov eax, edi
    and eax, 858993459
    shr edi, 2
    and edi, 858993459
    add edi, eax
    mov eax, edi
    shr eax, 4
    add eax, edi
    and eax, 252645135
    imul eax, eax, 16843009
    shr eax, 24
    ret

.global __kefirrt_parity
.hidden __kefirrt_parity
__kefirrt_parity:
    mov eax, edi
    shr eax, 16
    xor eax, edi
    xor al, ah
    setnp al
    movzx eax, al
    ret

.global __kefirrt_ffsl
.global __kefirrt_ffsll
.hidden __kefirrt_ffsl
.hidden __kefirrt_ffsll
__kefirrt_ffsl:
__kefirrt_ffsll:
    bsf rdi, rdi
    mov rax, -1
    cmove rdi, rax
    lea eax, [rdi+1]
    ret

.global __kefirrt_clzl
.global __kefirrt_clzll
.global __kefirrt_clzl
.hidden __kefirrt_clzll
__kefirrt_clzl:
__kefirrt_clzll:
    bsr rax, rdi
    xor eax, 63
    ret

.global __kefirrt_ctzl
.global __kefirrt_ctzll
.hidden __kefirrt_ctzl
.hidden __kefirrt_ctzll
__kefirrt_ctzl:
__kefirrt_ctzll:
    xor eax, eax
    rep bsf rax, rdi
    ret

.global __kefirrt_clrsbl
.global __kefirrt_clrsbll
.hidden __kefirrt_clrsbl
.hidden __kefirrt_clrsbll
__kefirrt_clrsbl:
__kefirrt_clrsbll:
    lea rax, [rdi + rdi]
    sar rdi, 63
    xor rax, rdi
    or rax, 1
    bsr rax, rax
    xor eax, 63
    ret

.global __kefirrt_popcountl
.global __kefirrt_popcountll
.hidden __kefirrt_popcountl
.hidden __kefirrt_popcountll
__kefirrt_popcountl:
__kefirrt_popcountll:
    mov rax, rdi
    shr rax
    movabs rcx, 6148914691236517205
    and rcx, rax
    sub rdi, rcx
    movabs rax, 3689348814741910323
    mov rcx, rdi
    and rcx, rax
    shr rdi, 2
    and rax, rdi
    add rax, rcx
    mov rcx, rax
    shr rcx, 4
    add rcx, rax
    movabs rdx, 1085102592571150095
    and rdx, rcx
    movabs rax, 72340172838076673
    imul rax, rdx
    shr rax, 56
    ret

.global __kefirrt_parityl
.global __kefirrt_parityll
.hidden __kefirrt_parityl
.hidden __kefirrt_parityll
__kefirrt_parityl:
__kefirrt_parityll:
    mov rax, rdi
    shr rax, 32
    xor eax, edi
    mov edx, eax
    shr edx, 16
    xor eax, edx
    xor al, ah
    setnp al
    movzx eax, al
    ret

.global __kefirrt_huge_val
.hidden __kefirrt_huge_val
__kefirrt_huge_val:
    movsd xmm0, QWORD PTR __kefirrt_huge_val_const[rip]
    ret
.section .rodata
    .align 8
__kefirrt_huge_val_const:
    .long   0
    .long   2146435072
.section .text

.global __kefirrt_huge_valf
.hidden __kefirrt_huge_valf
__kefirrt_huge_valf:
    movss xmm0, DWORD PTR __kefirrt_huge_valf_const[rip]
    ret
.section .rodata
    .align 4
__kefirrt_huge_valf_const:
    .long   2139095040
.section .text

.global __kefirrt_huge_vall
.hidden __kefirrt_huge_vall
__kefirrt_huge_vall:
    fld DWORD PTR __kefirrt_huge_vall_const[rip]
    ret
.section .rodata
    .align 4
__kefirrt_huge_vall_const:
    .long   2139095040
.section .text

.global __kefirrt_inf
.hidden __kefirrt_inf
__kefirrt_inf:
    movsd xmm0, QWORD PTR __kefirrt_inf_const[rip]
    ret
.section .rodata
    .align 8
__kefirrt_inf_const:
    .long   0
    .long   2146435072
.section .text

.global __kefirrt_inff
.hidden __kefirrt_inff
__kefirrt_inff:
    movss xmm0, DWORD PTR __kefirrt_inff_const[rip]
    ret
.section .rodata
    .align 4
__kefirrt_inff_const:
    .long   2139095040
.section .text

.global __kefirrt_infl
.hidden __kefirrt_infl
__kefirrt_infl:
    fld DWORD PTR __kefirrt_infl_const[rip]
    ret
.section .rodata
    .align 4
__kefirrt_infl_const:
    .long   2139095040
.section .text

.global __kefirrt_bswap16
.hidden __kefirrt_bswap16
__kefirrt_bswap16:
    mov eax, edi
    rol ax, 8
    ret

.global __kefirrt_bswap32
.hidden __kefirrt_bswap32
__kefirrt_bswap32:
    mov eax, edi
    bswap eax
    ret

.global __kefirrt_bswap64
.hidden __kefirrt_bswap64
__kefirrt_bswap64:
    mov rax, rdi
    bswap rax
    ret

.section .rodata
    .align 4
__kefirrt_opt_long_double_to_uint_constant:
    .long   1593835520

    .align 4
__kefirrt_opt_float32_to_uint_constant:
    .long   1593835520

    .align 8
__kefirrt_opt_float64_to_uint_constant:
    .long   0
    .long   1138753536

    .align 16
.global __kefir_opt_uint2long_double
.hidden __kefir_opt_uint2long_double
__kefir_opt_uint2long_double:
    .long 1602224128

    .align 16
.global __kefir_opt_float32_neg
.hidden __kefir_opt_float32_neg
__kefir_opt_float32_neg:
    .long   -2147483648
    .long   0
    .long   0
    .long   0

    .align 16
.global __kefir_opt_float64_neg
.hidden __kefir_opt_float64_neg
__kefir_opt_float64_neg:
    .long   0
    .long   -2147483648
    .long   0
    .long   0
