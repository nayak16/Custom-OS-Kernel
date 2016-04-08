/** @file user/progs/test_exec1.c
 *  @author mpa
 *  @brief Tests basic functionality of exec()
 *  @public yes
 *  @for p3
 *  @covers exec
 *  @status done
 */

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>
#include "410_tests.h"

DEF_TEST_NAME("exec_basic:");

int main(int argc, char** argv)
{

    int tid = fork();
    if (tid == 0) {
        char *args[] = {(char*) 0xdeadd00d, (char*)0xdeadbeef, 0};
        int ret = exec("actual_wait", args);
        lprintf("Exec returned: %d", ret);
        exit(ret);
    }
    int status;
    wait(&status);
    exit(0);
}
