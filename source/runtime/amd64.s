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
