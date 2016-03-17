.globl flush_tlb

flush_tlb:
    //invlpg 4(%esp)
    movl %cr3, %eax
    movl %eax, %cr3
    ret
