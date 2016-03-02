/* The 15-410 kernel project
 *
 *     loader.h
 *
 * Structure definitions, #defines, and function prototypes
 * for the user process loader.
 */

#ifndef _LOADER_H
#define _LOADER_H

#include <elf_410.h>
#include <pcb.h>
#include <frame_manager.h>
#include <x86/page.h>

#define USER_STACK_TOP 0xffffffff

#define USER_STACK_SIZE PAGE_SIZE

/* --- Prototypes --- */

int getbytes( const char *filename, int offset, int size, char *buf );

int load_elf_sections(simple_elf_t *elf, pcb_t *pcb, frame_manager_t *fm);

int load_user_stack(frame_manager_t *fm, pcb_t *pcb);

#endif /* _LOADER_H */
