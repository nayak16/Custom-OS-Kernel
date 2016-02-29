/** @file exception_handlers.c
 *  @brief implements various exception handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <x86/cr.h>

#include <simics.h>
/* panic inc */
#include <stdlib.h>
#include <debug.h>

void page_fault_c_handler(void){
    lprintf("Page fault occured!");
    panic("page_fault_c_handler: Not yet implemented");
}

void double_fault_c_handler(void){
    print_control_regs();
    lprintf("Double fault occured!");
    MAGIC_BREAK;
}
