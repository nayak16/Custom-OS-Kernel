/** @file pcb_t.h
 *
 */

#ifndef _PCB_T_H_
#define _PCB_T_H_

/* page directory include */
#include <page_directory.h>
/* frame manager include */
#include <frame_manager.h>

typedef struct pcb{
    int pid;
    page_directory_t pd;
} pcb_t;

int pcb_init(pcb_t *pcb);
int pcb_set_running(pcb_t *pcb);
int pcb_destroy(pcb_t *pcb);
int pcb_load(pcb_t *pcb, fm_t fm, const char *filename);

#endif /* _PCB_T_H_ */
