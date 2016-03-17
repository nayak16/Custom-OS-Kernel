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

#include <x86/cr.h>
#include <kern_internals.h>

void page_fault_c_handler(void){
    lprintf("Page fault occured!");
    print_control_regs();
    page_directory_t *pd_temp = malloc(sizeof(page_directory_t));
    pd_temp->directory = (uint32_t *)(get_cr3() & ~0xFFF);
    print_page_directory(pd_temp, 1023, 1, 1);
    MAGIC_BREAK;
    //panic("page_fault_c_handler: Not yet implemented");
}

void double_fault_c_handler(void){
    print_control_regs();
    lprintf("Double fault occured!");
    MAGIC_BREAK;
}
