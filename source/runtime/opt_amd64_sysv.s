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

.global __kefirrt_save_registers
.hidden __kefirrt_save_registers
__kefirrt_save_registers:
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
