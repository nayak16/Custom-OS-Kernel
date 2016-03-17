.globl flush_tlb

flush_tlb:
    invlpg 4(%esp)
    ret
