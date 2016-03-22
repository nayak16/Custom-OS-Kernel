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

    if (remove_pages((void *)0xFFFFE000) < 0){
        lprintf("failed to remove a single page");
        return -1;
    }
    lprintf("passed 1");
    if (remove_pages((void *)0xFFFF000) == 0){
        lprintf("removed a user space, non new_pages page");
        return -1;
    }
    lprintf("passed 2");
    if (remove_pages((void *)0xFFFF1000) == 0){
        lprintf("removed the middle of a user allocated page");
        return -1;
    }
    lprintf("passed 3");
    if (remove_pages((void *)0xFFFF2000) == 0){
        lprintf("removed the end of a user allocated page");
        return -1;
    }
    lprintf("passed 4");
    if (remove_pages((void *)0xFFFF0000) < 0){
        lprintf("failed to remove_pages");
    }

    lprintf("All tests passed!");
    return 0;
}
