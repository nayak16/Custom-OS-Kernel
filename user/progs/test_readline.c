/** @file test_readline.c
 *  @author Aatish
 *  @brief Tests to see if readline is working alright
 *  @public yes
 *  @for p3
 *  @status done
 */

/* Includes */
#include <syscall.h>
#include <simics.h>    /* for lprintf */
#include <report.h>
#include <stdio.h>


int main() {
    while (1){
        lprintf("Reading in a line");
        char buf[128];
        int i = readline(128, buf);
        print(i, buf);
        lprintf("Read in %d characters", i);
    }
    return 0;
}
