/**
 *  @file asm_helpers.S
 *  @brief Assembly helper functions for mutex, cond_vars, and threads
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

#include <syscall_int.h>

.globl thread_fork

thread_fork:
    // Push callee saved registers
    push %ebx

    // Save new_esp in reg
    movl 8(%esp), %ebx
    // Save func in reg
    movl 12(%esp), %ecx
    // Save args in reg
    movl 16(%esp), %edx
    // Save handler install in reg

    int $THREAD_FORK_INT

    cmp $0, %eax // if (eax == 0)
    jne parent
    // Move new_esp into esp/ebp reg
    movl %ebx, %esp
    movl %ebx, %ebp
    // Save caller saved register
    push %edx
    // Call handler install
    call install_exception_handler

    // "Call" thread function
    call %ecx
    // CHILD SHOULD NVR GET HERE
    push %eax
    call thr_exit

    parent:
    // Restore callee saved registers
    //pop %edi
    pop %ebx
    ret

.globl get_esp
get_esp:
    movl %esp, %eax // Get current esp
    add $4, %eax    // Account for pushed rtn addr
    ret
