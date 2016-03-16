/** @file test_cs.c
 *  @author Aatish
 *  @brief Tests to see if context switch works
 *  @public yes
 *  @for p3
 *  @status done
 */

/* Includes */
#include <simics.h>    /* for lprintf */
#include <report.h>


/* Main */
int main() {

    while(1){
        lprintf("Donkey says hello!");
    }

}
