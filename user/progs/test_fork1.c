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
    int id = 1;
    if (id == 0) {
        printf("Hello from child!");
        //id = fork();
    } else {
        printf("Parent made child: %d", id);
    }
    while (1) {
        printf("My id is %d\n", id);
    }
}
