
#include <syscall.h>
#include <simics.h>

int main(){
    int tid = fork();
    int i = -1;
    if (tid == 0){
        lprintf("Alpha sleeping");
        i = sleep(100);
        lprintf("Alpha awoken");
    } else {
        tid = fork();
        if (tid == 0){
            lprintf("Beta sleeping");
            i = sleep(50);
            lprintf("Beta awoken");
            i = sleep(30);
            lprintf("Beta awoken second time");
        } else {
            lprintf("Gamma sleeping");
            i = sleep(5);
            lprintf("Gamma awoken");
            i = sleep(70);
            lprintf("Gamma awoken second time");
        }
    }
    lprintf("i:%d",i);
    return 0;
}
