/** @file test_console_io.c
 *  @author Christopher Wei (cjwei)
 *  @covers set_term_color get_cursor_pos set_cursor_pos print
 *  @brief prints "Ho Ho Ho" in various colors at various positions
 *  @public yes
 *  @for p3
 *  @status done
 */

#include <stdlib.h>
#include <syscall.h>
#include <simics.h>


int main(){
    lprintf("Starting test");
    int fgnd = 0;
    int bgnd = 0;
    while(1){
        fgnd = ((fgnd + 1) % 0x10);
        bgnd = ((bgnd + 1) % 0x8);
        int color = fgnd | (bgnd << 4);
        set_term_color(color);
        int row, col;
        get_cursor_pos(&row, &col);
        print(2, "Ho");
        set_cursor_pos(((row+1) % 20), ((col+2) % 78));
    }
}
