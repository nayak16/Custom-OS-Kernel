/**
 * @file cyclone.c
 * @brief A test of basic thread behaviors with misbehavior
 *
 * This test spawns a thread and then attempts to join it
 * with each misbehavior mode.
 *
 * **> Public: Yes
 * **> Covers: thr_create,thr_join,thr_exit
 * **> NeedsWork: Yes
 * **> For: P2
 * **> Authors: mberman,jge
 * **> Notes: Looks good
 *
 * @author Michael Berman (mberman)
 * @author Based on thr_join0_test by Joey Echeverria (jge)
 * @bug No known bugs.
 */

#include <thread.h>
#include <stdlib.h>
#include <syscall.h>
#include <simics.h>
#include <stdio.h>

#include "410_tests.h"
DEF_TEST_NAME("cyclone:");

#define STACK_SIZE 4096
#define MAX_MISBEHAVE 64

int thread_exited = 0;
int thr_exit_return = 0;

void* thread1(void* token);

/**
 * @brief Spawns the thread and attempts to join
 *
 * @param argc The number of arguments
 * @param argv The argument array
 * @return 1 on success, < 0 on error.
 */
int main(int argc, char *argv[])
{
	int spawn_tid;
	int status;
	int i;

	REPORT_LOCAL_INIT;

	REPORT_START_CMPLT;

	REPORT_ON_ERR(thr_init(STACK_SIZE));

	for (i = 0; i < MAX_MISBEHAVE; i++) {
	  thread_exited = 0;
	  thr_exit_return = 0;

	  lprintf("%s%strying mode %d",TEST_PFX,test_name,i);
	  //misbehave(i);

	  REPORT_FAILOUT_ON_ERR((spawn_tid = thr_create(thread1, (void*)i)));

	  REPORT_FAILOUT_ON_ERR(thr_join(spawn_tid, (void**)&status));

	  if(thread_exited == 0) {
	    REPORT_MISC("Thread joined before exited");
	    REPORT_END_FAIL;
	    thr_exit((void *)-40);
	  }

	  if(status != i) {
	    REPORT_ERR("wrong token returned as status: ",status);
	    REPORT_END_FAIL;
	    thr_exit((void *)-60);
	  }

	  if(thr_exit_return) {
	    REPORT_MISC("ERR: thr_exit() returned.");
	    REPORT_END_FAIL;
	    thr_exit((void *)-80);
	  }
	}


	REPORT_END_SUCCESS;
	thr_exit((void *)0);

	REPORT_MISC("ERR: thr_exit() returned.");
	REPORT_END_FAIL;
	thr_exit((void *)-80);
	return (-80); // getting increasingly desperate here
}

/**
 * @brief Simply exit
 *
 * @param token An integer token to pass to thr_exit
 * @return The passed in token.
 */
void* thread1(void* token)
{
	thread_exited = 1;
	thr_exit(token);
	thr_exit_return = 1;

	return token;
}
