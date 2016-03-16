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


/* Main */
int main() {
    //int my_tid = gettid();
    int j = 5;
    while(1){
        j++;
    }

}
