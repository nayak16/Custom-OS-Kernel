.globl flush_tlb

flush_tlb:
    movl 4(%esp), %eax
    invlpg (%eax)
    ret
