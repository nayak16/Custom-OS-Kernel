.globl flush_tlb

flush_tlb:
    movl 4(%esp), %eax
    invlpg (%eax)
    ret

.globl set_cur_esp

set_cur_esp:
    /* Save rtn address */
    movl (%esp), %eax
    /* Move arg into esp */
    movl 4(%esp), %esp
    pushl %eax
    ret
