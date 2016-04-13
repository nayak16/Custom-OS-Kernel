/** @file test_hello.c
 *  @author Aatish Nayak (aatishn)
 *  @brief forks and execs into "test_foo" and context switches between the
 *  parent and child process.
 *  @public yes
 *  @for p3
 *  @status done
 */

#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <simics.h>

int main(){
    int tid = fork();
    lprintf("tid = %d", tid);
    if (tid == 0){
        while (1){
            printf("Hello\n");
            yield(-1);
        }
    } else {
        char *argv[] = {"test_foo", NULL};
        exec("test_foo", argv);
    }
    return 0;
}
