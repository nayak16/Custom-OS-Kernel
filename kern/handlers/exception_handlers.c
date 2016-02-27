/** @file exception_handlers.c
 *  @brief implements various exception handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <idt_handlers.h>
#include <x86/cr.h>

#include <simics.h>

int page_fault_handler(void){
    lprintf("cr2: %d", get_cr2());
    MAGIC_BREAK;
    return 0;
}
