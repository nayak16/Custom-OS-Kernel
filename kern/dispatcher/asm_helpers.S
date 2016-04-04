
#include <simics.h>


.globl restore_context

restore_context:
    movl 4(%esp), %esp
    pop %ds
    pop %es
    pop %fs
    pop %gs
    popa
    addl $4, %esp /* skip error_code */
    iret
