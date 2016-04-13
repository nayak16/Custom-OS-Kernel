/** @file thr_mgmt_handlers.c
 *  @brief implements thread management syscall handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */

#include <kern_internals.h>
#include <scheduler.h>
#include <thr_helpers.h>
#include <dispatcher.h>

#include <string.h>
#include <ureg.h>
#include <x86/eflags.h>
#include <x86/seg.h>
#include <common_kern.h>
#include <simics.h>


/** @brief Implements the gettid system call
 *  @return The tid of the current thread
 */
int syscall_gettid_c_handler(){
    return thr_gettid();
}

/** @brief Implements the yield system call
 *  @param old_esp The esp of the current running thread
 *  @param tid The tid of the thread to yield to, or -1 if arbitrary
 *  @return 0 on success, negative integer code on failure
 */
int syscall_yield_c_handler(uint32_t old_esp, int tid){
    return thr_yield(old_esp, tid);
}

/** @brief Implements the deschedule system call
 *  @param old_esp The esp of the current running thread
 *  @param reject Pointer to a reject integer if we should ignore the deschedule
 *  @return 0 on success, negative integer code on failure
 */
int syscall_deschedule_c_handler(uint32_t old_esp, int *reject){
   return thr_deschedule(old_esp, reject);
}

/** @brief Implements the make_runnable system call
 *  @param tid The tid to make runnable
 *  @return 0 on success, negative integer code on failure
 */
int syscall_make_runnable_c_handler(int tid){
    return thr_make_runnable(tid);
}

/** @brief Implements the get_ticks system call
 *  @return Number of clock ticks
 */
unsigned int syscall_get_ticks_c_handler(){
    return sched.num_ticks;
}

/** @brief Implements the sleep system call
 *  @param old_esp the esp of the current thread
 *  @param ticks The number of ticks to sleep
 *  @return 0 on success, negative integer code on failure
 */
int syscall_sleep_c_handler(uint32_t old_esp, int ticks){
    if (ticks < 0) return -1;
    if (ticks == 0) return 0;
    return thr_sleep(old_esp, ticks);
}

/** @brief Denotes a safe esp to swexn restore into*/
#define ESP_SAFE 0
/** @brief Denotes an unsafe esp to swexn restore into*/
#define ESP_UNSAFE 1

/** @brief Checks the safety of a esp to swexn restore into
 *  @param esp The esp to check
 *  @return ESP_SAFE if safe, otherwise ESP_UNSAFE
 */
int check_esp_safety(void *esp){
    if ((uint32_t)esp < USER_MEM_START) return ESP_UNSAFE;
    return ESP_SAFE;
}

/** @brief Denotes a safe eip to swexn restore into*/
#define EIP_SAFE 0
/** @brief Denotes a safe eip to swexn restore into*/
#define EIP_UNSAFE 1

/** @brief Checks the safety of a eip to swexn restore into
 *  @param eip The eip to check
 *  @return EIP_SAFE if safe, otherwise EIP_UNSAFE
 */
int check_eip_safety(void *eip){
    /* check for handlers in kernel memory */
    if ((uint32_t)eip < USER_MEM_START) return EIP_UNSAFE;
    return EIP_SAFE;
}

/** @brief Denotes a safe eflags to swexn restore into */
#define EFLAGS_SAFE 0
/** @brief Denotes an unsafe eflags to swexn restore into due to
 *  reserved flags */
#define EFLAGS_RESV 1
/** @brief Denotes an unsafe eflags to swexn restore into due to
 *  disabled interrupts */
#define EFLAGS_IF 2
/** @brief Denotes an unsafe eflags to swexn restore into due to
 *  IO privilege levels */
#define EFLAGS_IOPL 3
/** @brief Checks the safetey of eflags to swexn restore into
 *  @param eflags The eflags to check
 *  @return EFLAGS_SAFE on success otherwise a status code denoting the reason
 *  for unsafeness
 */
int check_eflags_safety(uint32_t eflags){
    if ((eflags & EFL_RESV1) == 0 ||
        (eflags & EFL_RESV2) != 0 ||
        (eflags & EFL_RESV3) != 0) return EFLAGS_RESV;
    if ((eflags & EFL_IF) == 0) return EFLAGS_IF;
    if ((eflags & EFL_IOPL_RING3) != 0) return EFLAGS_IOPL;
    if ((eflags & EFL_IOPL_RING2) != 0) return EFLAGS_IOPL;
    if ((eflags & EFL_IOPL_RING1) != 0) return EFLAGS_IOPL;
    return EFLAGS_SAFE;
}

/** @brief Denotes a safe ureg */
#define UREG_SAFE 0
/** @brief Denotes an unsafe ureg due to eflags */
#define UREG_EFLAGS 1
/** @brief Denotes an unsafe ureg due to segment selectors */
#define UREG_SEGMENT 2
/** @brief Denotes an unsafe ureg due to eip */
#define UREG_EIP 3
/** @brief Denotes an unsafe ureg due to esp */
#define UREG_ESP 4

/** @brief Checks safety of ureg to swexn restore into
 *  @param ureg The ureg to check
 *  @return UREG_SAFE if ureg is safe to restore into, otherwise some integer
 *  code denoting the reason for unsafeness
 */
int check_ureg_safety(ureg_t *ureg){
    if (ureg->ds != SEGSEL_USER_DS) return UREG_SEGMENT;
    if (ureg->ss != SEGSEL_USER_DS) return UREG_SEGMENT;
    if (ureg->cs != SEGSEL_USER_CS) return UREG_SEGMENT;
    if (check_eip_safety((void *)ureg->eip) != EIP_SAFE) return UREG_EIP;
    if (check_esp_safety((void *)ureg->esp) != ESP_SAFE) return UREG_ESP;
    if (check_eflags_safety(ureg->eflags) != EFLAGS_SAFE) return UREG_EFLAGS;
    return UREG_SAFE;
}


/** @brief Implements the swexn system call
 *  @param esp3 The exception stack to use
 *  @param eip The handler to call
 *  @param arg The arguments to the handler
 *  @param newureg The new uregs to restore into
 *  @param stack The stack address directly after saving context prior to this
 *  system call
 *  @return 0 on success, negative integer code on failure
 */
int syscall_swexn_c_handler(void *esp3,
        void (*eip)(void *arg, ureg_t *ureg),
        void *arg, ureg_t *newureg, uint32_t *stack){
    /* get current tcb */
    tcb_t *cur_tcb;
    if (scheduler_get_current_tcb(&sched, &cur_tcb) < 0) return -1;

    void *old_esp3;
    void (*old_eip)(void *arg, ureg_t *ureg);
    void *old_arg;
    /* deregister any custom handlers if any */
    if (tcb_deregister_swexn_handler(cur_tcb, &old_esp3, &old_eip, &old_arg) < 0){
        return -2;
    }
    /* check if we are either installing or uninstalling a handler */
    if (esp3 != NULL && eip != NULL){
        /* check for invalid esp or eip values */
        if (check_esp_safety(esp3) != ESP_SAFE || check_eip_safety(eip) != EIP_SAFE){
            /* undo deregister and return negative code */
            tcb_register_swexn_handler(cur_tcb, old_esp3, old_eip, old_arg);
            return -3;
        }
        if (tcb_register_swexn_handler(cur_tcb, esp3, eip, arg) < 0){
            /* undo deregister and return negative code */
            tcb_register_swexn_handler(cur_tcb, old_esp3, old_eip, old_arg);
            return -4;
        }
    }
    if (newureg != NULL){
        // check if newureg has any values that would crash the kernel
        if (check_ureg_safety(newureg) == UREG_SAFE){
            restore_context((uint32_t)&(newureg->ds));
        } else {
            /* undo changes and return negative integer code */
            tcb_register_swexn_handler(cur_tcb, old_esp3, old_eip, old_arg);
            return -5;
        }
    }
    return 0;
}
