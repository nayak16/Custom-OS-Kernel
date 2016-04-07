/** @file autostack.c
 *  @brief Contains implementation for autostack growth handler
 *
 *  For legacy single-threaded programs, this registers an exception handler
 *  that overwrites the default exception handler and looks for page faults.
 *  This allows us to extend the stack space by essentially doubling the
 *  amount of stack allocated with calls to newpage. We chose this method of
 *  increasing stack space as it allows us to quickly resolve page faults
 *  with a minimal calls to new_pages. In the case that the exception is not
 *  a pagefault or not related to stack growth (such as a dereference of NULL)
 *  then our custom exception handler will not be reinstalled and the program
 *  will resume at the faulting address thus causing the default handler to be
 *  called.
 *
 *  Another interesting design choice is to malloc the exception stack, which
 *  is given a stack size of 1 page which is plenty. The reasoning behind this
 *  design decision is that it would be incredibly messy and hard to either
 *  shift the stack down by 1 page to make room for an exception stack, or
 *  fragmenting the parent thread's stack by sticking it directly underneath
 *  the current stack pointeer. The exception stack will be freed in thr_init.
 *  In the off chance thr_init is never called, we will have memory leaks:
 *  which is not okay, but I guess it happens.
 *
 *  @author Christopher Wei (cjwei), Aatish Nayak (aatishn)
 *  @bug No known bugs
 */


#include <syscall.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#include <simics.h>
#include "contracts.h"

#define STACK_GROWTH_THRESHOLD (PAGE_SIZE * 32)

/** @brief External address to the top of the user space stack */
extern void *STACK_TOP;
/** @brief External address to the bottom of the user space stack */
extern void *STACK_BOTTOM;

/** @brief Pointer to the top of the exception stack */
void *exception_stack;

/** @brief A custom page fault handler that increases stack space on
 *         page fault related exceptions
 *  @param args The arguments passed in to page_fault_handler
 *  @param ureg The context saved before fault occured
 *  @return Void
 */
void page_fault_handler(void* args, ureg_t *ureg) {
    if (ureg->cause == SWEXN_CAUSE_PAGEFAULT && (void *)ureg->cr2 != NULL) {

        /* ensure that args.stack_high is never lower than args.stack_low */
        ASSERT((uint32_t)STACK_TOP >= (uint32_t)STACK_BOTTOM);

        /* Check to see if we can resolve address access by extending stack
         * by one page */
        if ((uint32_t)STACK_BOTTOM - STACK_GROWTH_THRESHOLD >= ureg->cr2
                || ureg->cr2 < 0x1000000){
            /* probably not a stack address, then */
            swexn(NULL,NULL, args, ureg);
        }

        lprintf("Autostack grown encountered!");

        void *new_stack_low =
            (void *)((uint32_t)STACK_BOTTOM - PAGE_SIZE);

        /* if unable to allocate more pages restore context without
         * reregistering */
        if (new_pages(new_stack_low, PAGE_SIZE) < 0)
            swexn(NULL,NULL, args, ureg);

        STACK_BOTTOM = new_stack_low;

        /* only re-register the handler if we catch a page fault
         * after registering handler, restore context */
        if (swexn((void*) exception_stack, page_fault_handler,
                  (void*) args, ureg) < 0){
            panic("Couldn't register autostack handler after page fault");
        }
        /* SHOULD NEVER GET HERE */
        panic("Should never get here");
    }
    /* restore context, do not register another handler */
    swexn(NULL,NULL, args, ureg);
}

/** @brief Sets up automatic stack growth handler
 *  @param stack_high The top of the currently allocated stack
 *  @param stack_low The bottom of the currently allocated stack
 *  @return Void
 */
void install_autostack(void *stack_high, void *stack_low) {
    /* update the global variables */
    STACK_TOP = stack_high;
    STACK_BOTTOM = stack_low;
    /* allocate a exception stack in the heap */
    exception_stack = _malloc(PAGE_SIZE);
    /* install custom exception handler */
    if (swexn(exception_stack, page_fault_handler, NULL, NULL) < 0)
        panic("Couldn't register autostack handler");
}
