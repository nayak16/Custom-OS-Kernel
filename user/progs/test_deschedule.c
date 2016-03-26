#include <simics.h>
#include <syscall.h>

int main(){
    int i;
    int reject = 0;
    int tid = 0;
    for (i = 0; i < 10; i++){
        if (tid == 0) lprintf("Parent: %d", i);
        else lprintf("Child: %d", i);
        if (i == 2 && tid == 0) {

            if (tid == 0) deschedule(&reject);
        }
    }
    return 0;
}
