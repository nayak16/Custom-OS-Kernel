/** @file tcb_pool.h
 *  @brief Defines interface for a thread control block pool
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)

 *  @bug No known bugs
 */



#include <ll.h>
#include <ht.h>
#include <tcb_pool.h>
#include <tcb.h>


int tcb_pool_init(tcb_pool_t *tp);
int tcb_pool_add_tcb(tcb_pool_t *tp, tcb_t *tcb);
int tcb_pool_get_next_tcb(tcb_pool_t *tp);
int tcb_pool_find_tcb(tcb_pool_t *tp, int id, tcb_t **tcbp);
int tcb_pool_remove_tcb(tcb_pool_t *tp, int id);


