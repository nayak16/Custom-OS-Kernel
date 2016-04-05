/** @file user/progs/test_life_cycle.c
 *  @brief tests fork, wait
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 */

#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <simics.h>

#include <thread.h>

#define SIZE 4096
#define CHILD_RETURN 42
#define PARENT_RETURN 43

/** @brief Makes a child and store index in array
 *  @param i The index
 *  @return 0
 */
void* foo(void* i) {

    lprintf("Hello from child! %d", gettid());
    int tid = fork();
    lprintf("Good I shouldn't be allowed to fork %d", tid);
    return 0;
}


/** @brief Tests thr_create
 *  @return 0
 */
int main()
{
    int i = 1;
    thr_init(SIZE);

    int tid = thr_create(foo, (void*) i);

    lprintf("Created child thread: %d, Now sleeping!", tid);
    sleep(200);
    return 0;

}
