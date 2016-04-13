/** @file test_foo.c
 *  @author Aatish Nayak (aatishn)
 *  @brief Tests context switch
 *  @public yes
 *  @for p3
 *  @status done
 */

#include <simics.h>
#include <syscall.h>
#include <stdio.h>

int main(){
    int i = fork();
    while(1){
        printf("foo %d\n", i);
        yield(-1);
    }
    return 0;
}
