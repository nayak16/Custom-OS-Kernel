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
#include <thr_helpers.h>

#include <x86/cr.h>
#include <kern_internals.h>

void page_fault_c_handler(void){
    lprintf("Segmentation Fault");
    thr_set_status(-1);
    thr_vanish();
}

void double_fault_c_handler(void){
    print_control_regs();
    lprintf("Double fault occured!");
    MAGIC_BREAK;
}
