/** @file 410user/progs/idle.c
 *  @author ?
 *  @brief Idle program.
 *  @public yes
 *  @for p2 p3
 *  @covers
 *  @status done
 */

#include <simics.h>
#include <stdio.h>
#include <syscall.h>

int main()
{
    int id = fork();
    if (id == 0) {
        printf("Hello from child!");
        fork();
        fork();
    } else {
        printf("Parent made child: %d", id);
    }
    int i = 0;
    while (1) {
        printf("My tid: %d\n", gettid());
        for(i = 0 ; i < 2000000 ; i++) {

        }
    }
}
