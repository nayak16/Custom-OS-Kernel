/** @file 410user/progs/getpid_test1.c
 *  @author zra
 *  @brief Tests gettid().
 *  @public yes
 *  @for p2 p3
 *  @covers gettid
 *  @status done
 */

/* Includes */
#include <syscall.h>  /* for getpid */
#include <stdlib.h>   /* for exit */
#include <simics.h>    /* for lprintf */
#include "410_tests.h"
#include <report.h>

DEF_TEST_NAME("getpid_test1:");

/* Main */
int main() {
	int pid;

    report_start(START_CMPLT);
	pid = gettid();
	lprintf("%s my pid is: %d", test_name, pid);
	
    if(pid == gettid()) {
        report_end(END_SUCCESS);
    } else {
        report_end(END_FAIL);
    }

	exit(0);
}
