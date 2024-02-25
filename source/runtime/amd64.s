# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2020-2024 Jevgenijs Protopopovs
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

.global __kefirrt_opt_complex_long_double_equals
.hidden __kefirrt_opt_complex_long_double_equals
__kefirrt_opt_complex_long_double_equals:
    push rcx
    push rdx
    fxch    st(3)
    fucompi st, st(1)
    fstp    st(0)
    setnp   al
    sete    cl
    and     cl, al
    fucompi st, st(1)
    fstp    st(0)
    setnp   al
    sete    dl
    and     dl, al
    and     dl, cl
    movzx   eax, dl
    pop rdx
    pop rcx
    ret

.global __kefirrt_opt_complex_long_double_truncate_1bit
.hidden __kefirrt_opt_complex_long_double_truncate_1bit
__kefirrt_opt_complex_long_double_truncate_1bit:
    push rcx
    push rdx
    fldz
    fucomi  st, st(2)
    fstp    st(2)
    setp    al
    setne   cl
    or      cl, al
    fxch    st(1)
    fucompi st, st(1)
    fstp    st(0)
    setp    dl
    setne   al
    or      al, dl
    or      al, cl
    pop rdx
    pop rcx
    ret

.global __kefirrt_opt_complex_float32_mul
.hidden __kefirrt_opt_complex_float32_mul
__kefirrt_opt_complex_float32_mul:
    sub rsp, 16
    movdqu [rsp], xmm2
    movaps xmm2, xmm1
    shufps xmm2, xmm1, 160
    mulps xmm2, xmm0
    xorps xmm0, XMMWORD PTR __kefirrt_opt_complex_float32_mul_const[rip]
    shufps xmm0, xmm0, 177
    shufps xmm1, xmm1, 245
    mulps xmm0, xmm1
    addps xmm0, xmm2
    movdqu xmm2, [rsp]
    add rsp, 16
    ret

.global __kefirrt_opt_complex_float32_div
.hidden __kefirrt_opt_complex_float32_div
__kefirrt_opt_complex_float32_div:
    sub rsp, 80
    movdqu [rsp + 64], xmm6
    movdqu [rsp + 48], xmm5
    movdqu [rsp + 32], xmm4
    movdqu [rsp + 16], xmm3
    movdqu [rsp], xmm2
    cvtps2pd xmm5, xmm1
    cvtps2pd xmm3, xmm0
    movaps xmm6, xmm5
    movaps xmm2, xmm5
    unpcklpd xmm6, xmm5
    mulpd xmm6, xmm3
    shufpd xmm3, xmm3, 1
    unpckhpd xmm2, xmm5
    mulpd xmm5, xmm5
    mulpd xmm3, xmm2
    movaps xmm4, xmm5
    xorps xmm3, XMMWORD PTR __kefirrt_opt_complex_float32_div_const[rip]
    shufpd xmm4, xmm5, 1
    addpd xmm6, xmm3
    addpd xmm5, xmm4
    divpd xmm6, xmm5
    cvtpd2ps xmm0, xmm6
    movdqu xmm6, [rsp + 64]
    movdqu xmm5, [rsp + 48]
    movdqu xmm4, [rsp + 32]
    movdqu xmm3, [rsp + 16]
    movdqu xmm2, [rsp]
    add rsp, 80
    ret

.global __kefirrt_opt_complex_float64_mul
.hidden __kefirrt_opt_complex_float64_mul
__kefirrt_opt_complex_float64_mul:
    sub rsp, 16
    movdqu [rsp], xmm4
    unpcklpd xmm0, xmm1
    movaps xmm4, xmm0
    xorps xmm4, XMMWORD PTR __kefirrt_opt_complex_float64_mul_const[rip]
    shufpd xmm4, xmm4, 1
    unpcklpd xmm2, xmm2
    unpcklpd xmm3, xmm3
    mulpd xmm2, xmm0
    mulpd xmm3, xmm4
    addpd xmm3, xmm2
    movaps xmm1, xmm3
    movaps xmm0, xmm3
    unpckhpd xmm1, xmm3
    movdqu xmm4, [rsp]
    add rsp, 16
    ret

.global __kefirrt_opt_complex_float64_div
.hidden __kefirrt_opt_complex_float64_div
__kefirrt_opt_complex_float64_div:
    fld1
    movsd     QWORD PTR [rsp - 24], xmm2
    fld       QWORD PTR [rsp - 24]
    movsd     QWORD PTR [rsp - 24], xmm3
    fld       st(0)
    fmul      st, st(1)
    fld       QWORD PTR [rsp - 24]
    fld       st(0)
    fmul      st, st(1)
    movsd     QWORD PTR [rsp - 24], xmm0
    faddp     st(2), st
    fxch      st(1)
    fdivp     st(3), st
    fld       QWORD PTR [rsp - 24]
    movsd     QWORD PTR [rsp - 24], xmm1
    fld       st(0)
    fmul      st, st(3)
    fxch      st(1)
    fmul      st, st(2)
    fld       QWORD PTR [rsp - 24]
    fld       st(0)
    fmulp     st(4), st
    fxch      st(3)
    faddp     st(2), st
    fxch      st(1)
    fmul      st, st(4)
    fstp      QWORD PTR [rsp - 16]
    fxch      st(2)
    fmulp     st(1), st
    movsd     xmm0, QWORD PTR [rsp - 16]
    fsubrp    st(1), st
    fmulp     st(1), st
    fstp      QWORD PTR [rsp - 16]
    movsd     xmm1, QWORD PTR [rsp - 16]
    ret

.global __kefirrt_opt_complex_long_double_mul
.hidden __kefirrt_opt_complex_long_double_mul
__kefirrt_opt_complex_long_double_mul:
    sub rsp, 64
    fstp TBYTE PTR [rsp]
    fstp TBYTE PTR [rsp + 16]
    fstp TBYTE PTR [rsp + 32]
    fstp TBYTE PTR [rsp + 48]
    fld TBYTE PTR [rsp + 16]
    fld st(0)
    fld TBYTE PTR [rsp + 32]
    fld TBYTE PTR [rsp + 48]
    fmul st(2), st
    fld TBYTE PTR [rsp]
    fmul st, st(2)
    fsubrp st(3), st
    fxch st(2)
    fxch st(3)
    fmulp st(1), st
    fld TBYTE PTR [rsp]
    fmulp st(2), st
    faddp st(1), st
    fxch st(1)
    add rsp, 64
    ret

.global __kefirrt_opt_complex_long_double_div
.hidden __kefirrt_opt_complex_long_double_div
__kefirrt_opt_complex_long_double_div:
    sub rsp, 64
    fstp TBYTE PTR [rsp]
    fstp TBYTE PTR [rsp + 16]
    fstp TBYTE PTR [rsp + 32]
    fstp TBYTE PTR [rsp + 48]
    fld TBYTE PTR [rsp]
    fld TBYTE PTR [rsp + 16]
    fld TBYTE PTR [rsp + 32]
    fld st(0)
    fabs
    fld TBYTE PTR [rsp + 48]
    fld st(0)
    fabs
    fcomip st, st(2)
    fxch st(1)
    fstp st(0)
    fld TBYTE PTR __kefirrt_opt_complex_long_double_div_const[rip]
    jb __kefirrt_opt_complex_long_double_div4
__kefirrt_opt_complex_long_double_div2:
    fxch st(1)
    fdiv st(2), st
    fld st(2)
    fmul st, st(3)
    fadd st, st(2)
    fmulp st(1), st
    fdivp st(1), st
    fld st(3)
    fmul st, st(2)
    fadd st, st(3)
    fmul st, st(1)
    fxch st(3)
    fmulp st(2), st
    fxch st(3)
    fsubp st(1), st
    fmulp st(2), st
    fxch st(1)
__kefirrt_opt_complex_long_double_div3:
    fxch st(1)
    fxch st(1)
    fxch st(1)
    add rsp, 64
    ret
__kefirrt_opt_complex_long_double_div4:
    fxch st(1)
    fdiv st, st(2)
    fld st(0)
    fmul st, st(1)
    fadd st, st(2)
    fmulp st(3), st
    fxch st(2)
    fdivp st(1), st
    fld st(2)
    fmul st, st(2)
    fadd st, st(4)
    fmul st, st(1)
    fxch st(4)
    fmulp st(2), st
    fxch st(1)
    fsubp st(2), st
    fmulp st(1), st
    jmp __kefirrt_opt_complex_long_double_div3

.global __kefirrt_fenv_update
.hidden __kefirrt_fenv_update
__kefirrt_fenv_update:
# Test exceptions
	push rax
	stmxcsr [rsp]
	pop rsi
	fnstsw ax
	or eax, esi
	and eax, 0x3f
# Set environment
	fldenv [rdi]
	ldmxcsr [rdi + 28]
# Raise exceptions
	stmxcsr [rsp - 8]
	or [rsp - 8], eax
	ldmxcsr [rsp - 8]
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

    .align 4
__kefirrt_opt_complex_float32_mul_const:
    .long 0x00000000
    .long 0x80000000
    .long 0x00000000
    .long 0x80000000

    .align 8
__kefirrt_opt_complex_float32_div_const:
__kefirrt_opt_complex_float64_mul_const:
    .long 0x00000000
    .long 0x00000000
    .long 0x00000000
    .long 0x80000000

    .align 16
__kefirrt_opt_complex_long_double_div_const:
    .byte 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x80
    .byte 0xff, 0x3f, 0x00, 0x00
    .byte 0x00, 0x00 ,0x00, 0x00

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

    .align 16
.global __kefir_opt_complex_float32_neg
.hidden __kefir_opt_complex_float32_neg
__kefir_opt_complex_float32_neg:
    .long 0x80000000
    .long 0x80000000
    .long 0x80000000
    .long 0x80000000

    .align 16
.global __kefir_opt_complex_float64_neg
.hidden __kefir_opt_complex_float64_neg
__kefir_opt_complex_float64_neg:
    .quad 0x8000000000000000
    .quad 0x8000000000000000
