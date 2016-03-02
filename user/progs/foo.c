#include <simics.h>
#include <syscall.h>

int main(){

    sim_ck1();
    MAGIC_BREAK;
    int tid = gettid();
    if (tid == -1) return -1;

    while(1);
    return 0;
}
