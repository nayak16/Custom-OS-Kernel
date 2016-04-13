/* @file test_div0
 * @author Christopher Wei (cjwei)
 * @brief Tests divide by zero handler and swexn (div by 0 doesnt generate
 * an error code is this is really testing that logic)
 * @public yes
 * @for p3
 * @status done
 */

#include <simics.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include "410_tests.h"

DEF_TEST_NAME("test_div0:");

#define STAQ_SIZE (4096)
char exn_staq[STAQ_SIZE];
#define EXN_STAQ_TOP ((void *)(&exn_staq[STAQ_SIZE-7]))

void success(){
    REPORT_MISC("Success!");
    exit(0);
}


void handler(void *arg, ureg_t *uregs){
    /* catch div by 0 error */
    REPORT_MISC("Hello from a handler");
    if (uregs->cause != SWEXN_CAUSE_DIVIDE){
        REPORT_MISC("not a divide by zero error!");
    }
    if (uregs == NULL){
        REPORT_MISC("bad uregs");
    }
    /* jump to "success" upon return */
    uregs->eip = (int)success;
    swexn(EXN_STAQ_TOP, handler, NULL, uregs);
}


int main(){
    int x = 1;
    int y = 0;

    /* install handler */
    int ret = swexn(EXN_STAQ_TOP, handler, NULL, NULL);
    if (ret < 0){
        REPORT_MISC("Uh oh! Swexn failed!");
    }
    /* generate div by 0 error */
    int z = x/y;
    REPORT_MISC("Oops! Should not have gotten here");
    return z;
}
