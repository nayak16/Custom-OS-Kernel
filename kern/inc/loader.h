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

#define USER_STACK_TOP 0xfffffff0

#define USER_STACK_BOTTOM 0xffffe000

#define USER_STACK_SIZE (PAGE_SIZE*2)

/* --- Prototypes --- */

int getbytes( const char *filename, int offset, int size, char *buf );

int load_elf_sections(simple_elf_t *elf, pcb_t *pcb);
int load_elf_exists(const char *filename);
int load_user_stack(pcb_t *pcb);

#endif /* _LOADER_H */
