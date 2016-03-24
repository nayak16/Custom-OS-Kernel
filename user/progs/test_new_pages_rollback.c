/** @file test_new_pages_rollback.c
 *  @brief attempts to allocate more frames than avaliable
 */

#include <stdlib.h>
#include <syscall.h>
#include <simics.h>

int main(){
    /* allocate more than half of the total physical memory */
    new_pages((void *)0x2000000, 524288);
    lprintf("Allocated a lot of pages...");
    if (fork() == 0){
        lprintf("Somehow, forked passed haha...");
    }
    lprintf("Attempting to allocate another page");
    if (new_pages((void *) 0x1FFF000, 1) < 0){
        lprintf("Not enough pages to allocate one page");
        return -1;
    }
    lprintf("All tests passed!");
    return 0;
}
