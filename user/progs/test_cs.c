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
    int j = 5;
    while(1){
        j++;
        printf("j = %d\n", j);
    }

}
