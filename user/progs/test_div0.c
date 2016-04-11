#include <simics.h>


int main(){
    int x = 1;
    int y = 0;
    int z = x/y;
    lprintf("Should not get here");
    return z;
}
