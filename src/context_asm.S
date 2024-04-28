/*
    This file is part of RIBS2.0 (Robust Infrastructure for Backend Systems).
    RIBS is an infrastructure for building great SaaS applications (but not
    limited to).

    Copyright (C) 2012,2013 Adap.tv, Inc.

    RIBS is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, version 2.1 of the License.

    RIBS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with RIBS.  If not, see <http://www.gnu.org/licenses/>.
*/
.extern current_ctx
.extern __ribs_context_cleanup
.globl ribs_swapcurcontext
.globl __ribs_context_jump

ribs_swapcurcontext:

#ifdef __x86_64__
        movq    current_ctx, %rsi
        /* Save the preserved registers. */
        movq    %rsp, 0(%rsi)
        movq    %rbx, 8(%rsi)
        movq    %rbp, 16(%rsi)
        movq    %r12, 24(%rsi)
        movq    %r13, 32(%rsi)
        movq    %r14, 40(%rsi)
        movq    %r15, 48(%rsi)

ribs_setcontext:
        movq    %rdi, current_ctx
        /* Load the new stack pointer and the preserved registers.  */
        movq    0(%rdi), %rsp
        movq    8(%rdi), %rbx
        movq    16(%rdi), %rbp
        movq    24(%rdi), %r12
        movq    32(%rdi), %r13
        movq    40(%rdi), %r14
        movq    48(%rdi), %r15
        ret

__ribs_context_jump:
        call    __ribs_context_cleanup
        movq    %rbx, %rdi
        jmp     ribs_setcontext
#endif

#ifdef __i386__
        movl    current_ctx, %eax
        /* Save the preserved registers. */
        movl    %esp, 0(%eax)
        movl    %ebx, 4(%eax)
        movl    %ebp, 8(%eax)
        movl    %esi, 12(%eax)
        movl    %edi, 16(%eax)

        movl    4(%esp), %eax
ribs_setcontext:
        movl    %eax, current_ctx
        /* Load the new stack pointer and the preserved registers.  */
        movl    0(%eax), %esp
        movl    4(%eax), %ebx
        movl    8(%eax), %ebp
        movl    12(%eax), %esi
        movl    16(%eax), %edi
        ret

__ribs_context_jump:
        call    __ribs_context_cleanup
        movl    %ebx, %eax
        jmp     ribs_setcontext
#endif

#ifdef __arm__
#ifdef __thumb2__
.thumb
.thumb_func
.syntax unified

        ldr     r1, =current_ctx
        ldr     r2, [r1]
        str     sp, [r2, #36]
        stmia   r2, {r4-r11, lr}
        str     r0, [r1]
        ldr     sp, [r0, #36]
        ldmia   r0, {r4-r11, pc}

__ribs_context_jump:
        blx     r5
        bl      __ribs_context_cleanup
        ldr     r1, =current_ctx
        str     r4, [r1]
        ldr     sp, [r4, #36]
        ldmia   r4, {r4-r11, pc}

#else /* __arm__, not __thumb2__ */
        ldr     r1, =current_ctx
        ldr     r2, [r1]
        stmia   r2, {r4-r11, sp, lr}
        str     r0, [r1]
        ldmia   r0, {r4-r11, sp, pc}

__ribs_context_jump:
        blx     r5
        bl      __ribs_context_cleanup
        ldr     r1, =current_ctx
        str     r4, [r1]
        ldmia   r4, {r4-r11, sp, pc}
#endif
#endif
.global __ribs_context_start
#if defined(__aarch64__)
        // store the state in the current context
        ldr     x1, =current_ctx
        ldr     x2, [x1]
        stp     x19, x20, [x2], #16
        stp     x21, x22, [x2], #16
        stp     x23, x24, [x2], #16
        stp     x25, x26, [x2], #16
        stp     x27, x28, [x2], #16
        stp     x29, x30, [x2], #16
        mov     x3, sp
        str     x3, [x2], #8

ribs_setcontext:
        // set current_ctx to the new context
        ldr     x1, =current_ctx
        str     x0, [x1]

        // restore the target context
        ldp     x19, x20, [x0], #16
        ldp     x21, x22, [x0], #16
        ldp     x23, x24, [x0], #16
        ldp     x25, x26, [x0], #16
        ldp     x27, x28, [x0], #16
        ldp     x29, x30, [x0], #16
        ldr     x3, [x0], #8
        mov     sp, x3

        ret

__ribs_context_jump:
        bl      __ribs_context_cleanup
        mov     x0, x19
        b       ribs_setcontext

__ribs_context_start:
        // set LP to the trampoline
        ldr     x30, =__ribs_context_jump
        // jump to the user function
        br      x20

#endif
