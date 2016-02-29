/** @file exception_handlers.c
 *  @brief implements various exception handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <x86/cr.h>

#include <simics.h>

void page_fault_c_handler(void){
    lprintf("Page fault occured!");
    lprintf("cr2: %u", (unsigned int) get_cr2());
}

void double_fault_c_handler(void){
    lprintf("Double fault occured!");
    lprintf("cr2: %u", (unsigned int) get_cr2());
    MAGIC_BREAK;
}
