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

.global __kefirrt_opt_save_registers
.hidden __kefirrt_opt_save_registers
__kefirrt_opt_save_registers:
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
__kefirrt_opt_float32_to_uint_constant:
    .long   1593835520

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
    .align 8
__kefirrt_opt_float64_to_uint_constant:
    .long   0
    .long   1138753536

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
    fld DWORD PTR __kefirrt_opt_long_double_to_uint_constant
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
__kefirrt_opt_long_double_to_uint_constant:
    .long   1593835520

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

