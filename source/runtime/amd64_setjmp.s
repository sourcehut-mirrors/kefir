# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2020-2022 Jevgenijs Protopopovs
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

.global __kefirrt_setjmp
.global __kefirrt_longjmp

.type __kefirrt_setjmp, @function
__kefirrt_setjmp:
# Save registers
    mov [rdi], rbx
    mov [rdi + 8], rbp
    mov [rdi + 16], r12
    mov [rdi + 24], r13
    mov [rdi + 32], r14
    mov [rdi + 40], r15
# Save stack pointer
    lea rdx, [rsp + 8]
    mov [rdi + 48], rdx
# Save return address
    mov rdx, [rsp]
    mov [rdi + 56], rdx
# Save stack
    lea rsi, [rsp + 8]  # Stack contents
    mov rcx, r14        # Number of qwords on stack (max. 31)
    sub rcx, rsi
    shr rcx, 3
    and rcx, 31
    mov [rdi + 64], rcx # Save number of qwords
    lea rdi, [rdi + 72] # Buffer with stack contents
    rep movsq
# Return
    xor rax, rax
    ret

.type __kefirrt_longjmp, @function
__kefirrt_longjmp:
# Return (val != 0 ? val : 1)
    xor rax, rax
	cmp esi, 1
    adc eax, esi
# Restore registers
    mov rbx, [rdi]
    mov rbp, [rdi + 8]
    mov r12, [rdi + 16]
    mov r13, [rdi + 24]
    mov r14, [rdi + 32]
    mov r15, [rdi + 40]
    mov rsp, [rdi + 48]
# Restore stack
    mov r8, [rdi + 56]  # Return address
    mov rcx, [rdi + 64] # Number of qwords on stack
    lea rsi, [rdi + 72] # Buffer with stack contents
    mov rdx, rcx        # Stack pointer
    shl rdx, 3
    mov rsp, r14
    sub rsp, rdx
    lea rdi, [rsp]      # Stack contents
    rep movsq
# Return to setjmp site
    jmp r8
