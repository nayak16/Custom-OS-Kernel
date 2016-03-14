/**
 *  @file asm_helpers.S
 *  @brief Assembly helper functions for kernel
 *  @author Aatish Nayak (aatishn) and Christopher Wei (cjwei)
 *  @bug No known bugs
 */

.globl xchng

xchng:
    push %ebx
    movl 8(%esp), %ebx  // address to lock
    movl 12(%esp), %eax // value to be stored into lock
    xchg %eax, (%ebx)
    pop %ebx
    ret


