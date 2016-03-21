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

  char *name = "knife";
  char *args[] = {name, "a", 0};

  MAGIC_BREAK;
  exec(name,args);

  exit(-1);
}
