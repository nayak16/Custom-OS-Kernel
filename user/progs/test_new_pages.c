/** @file test_cs.c
 *  @author Aatish
 *  @brief Tests to see if context switch works
 *  @public yes
 *  @for p3
 *  @status done
 */

/* Includes */
#include <syscall.h>
#include <simics.h>    /* for lprintf */
#include <report.h>
#include <stdio.h>

/* Main */
int main() {
    if (new_pages((void *)0x1000, PAGE_SIZE) == 0){
        lprintf("allocated page not in user space");
        return -1;
    }
    if (new_pages((void *)0x0, PAGE_SIZE) == 0){
        lprintf("allocated 0th page");
        return -1;
    }
    if (new_pages((void *)0xFFFFF000, PAGE_SIZE) == 0){
        lprintf("allocated preallocated page");
        return -1;
    }
    if (new_pages((void *)0x1000000, PAGE_SIZE) == 0){
        lprintf("allocated preallocated page");
        return -1;
    }
    if (new_pages((void *)0x2000000, -PAGE_SIZE) == 0){
        lprintf("allocated negative page size");
        return -1;
    }
    if (new_pages((void *)0x2000000, 0) == 0){
        lprintf("allocated 0 sized page");
        return -1;
    }
    if (new_pages((void *)0xFFFFE000, PAGE_SIZE) < 0){
        lprintf("failed to allocate a page");
        return -1;
    }
    if (new_pages((void *)0xFFFFE000, PAGE_SIZE) == 0){
        lprintf("allocated the same page twice");
        return -1;
    }
    if (new_pages((void *)0xFFFF0000, 3*PAGE_SIZE) < 0){
        lprintf("failed to allocate multiple pages");
        return -1;
    }

    /* now check to see if pages are actually mapped */
    int *foo = (int *)0xFFFFE000;
    int i;
    for (i = 0; i < PAGE_SIZE/(sizeof(int)); i++){
        foo[i] = i;
    }

    lprintf("All tests passed!");
    return 0;
}
