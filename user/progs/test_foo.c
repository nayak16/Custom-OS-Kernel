#include <simics.h>
#include <syscall.h>

int main(){
    int i;
    for (i = 0; i < 10; i++){
        continue;
    }
    halt();
    lprintf("returned from halt");
    return 0;
}
