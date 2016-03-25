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

    lprintf("argc: %d", argc);
    int i;
    for (i = 0 ; i < argc ; i++) {
        lprintf("arg%d: %s, argv: %p", i+1, argv[i], &(argv[i]));
    }
  char *args[] = {"test_exec1", "RECURSIVE", 0};
    MAGIC_BREAK;
  exec("test_exec1",args);

  exit(-1);
}
