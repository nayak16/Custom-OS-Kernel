#include <simics.h>
#include <syscall.h>

int main(){

    sim_ck1();
    int tid = gettid();
    lprintf("tid %d", tid);
    while(1);
    return 0;
}
