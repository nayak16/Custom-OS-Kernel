/** @file test_deschedule.c
 *  @author Christopher Wei (cjwei)
 *  @covers gettid fork deschedule make_runnable
 *  @brief forks a new process and deschedules until the new process wakes the
 *  parent up
 *  @public yes
 *  @for p3
 *  @status done
 */

#include <simics.h>
#include <syscall.h>

int main(){
    int reject = 0;
    int parent_tid = gettid();
    int child_tid = fork();
    if (child_tid != 0) {
        lprintf("Parent sleeping!");
        deschedule(&reject);
        lprintf("Parent woke up!!");
    }
    int j;
    for(j = 0 ; j < 10000 ; j++);
    if (child_tid == 0 && make_runnable(parent_tid) < 0) {
        lprintf("Parent not asleep!");
    }
    return 0;
}
