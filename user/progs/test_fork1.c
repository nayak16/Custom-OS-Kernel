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
        //id = fork();
    } else {
        printf("Parent made child: %d", id);
    }
    while (1) {
        printf("My tid: %d\n", id);
        int i;
        for (i = 0; i < 2000000; i++){
            continue;
        }
    }
}
