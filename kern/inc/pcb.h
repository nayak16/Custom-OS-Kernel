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
    uint32_t stack_top;

    int argc;
    char **argv;

    unsigned long entry_point;
    page_directory_t pd;

    uint32_t num_threads;
} pcb_t;

int pcb_init(pcb_t *pcb);
int pcb_set_running(pcb_t *pcb);
int pcb_destroy(pcb_t *pcb);
int pcb_load_prog(pcb_t *pcb, const char *filename, int argc, char** argv);
int pcb_copy(pcb_t *dest_pcb, pcb_t *source_pcb);

#endif /* _PCB_T_H_ */
