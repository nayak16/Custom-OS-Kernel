/**
 * The 15-410 kernel project.
 * @name loader.c
 *
 * Functions for the loading
 * of user programs from binary
 * files should be written in
 * this file. The function
 * elf_load_helper() is provided
 * for your use.
 */
/*@{*/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>

#include <simics.h>

/* --- Local function prototypes --- */


/**
 * Copies data from a file into a buffer.
 *
 * @param filename   the name of the file to copy data from
 * @param offset     the location in the file to begin copying from
 * @param size       the number of bytes to be copied
 * @param buf        the buffer to copy the data into
 *
 * @return returns the number of bytes copied on succes; -1 on failure
 */
int getbytes( const char *filename, int offset, int size, char *buf )
{
    int i;
    lprintf("Searching through exec2bj entries...");
    for (i = 0; i < exec2obj_userapp_count; i++){
        exec2obj_userapp_TOC_entry entry = exec2obj_userapp_TOC[i];

        lprintf(entry.execname);

        if (strcmp(entry.execname, filename) == 0){
            int execlen = entry.execlen;
            /* check if we have enough bytes  */
            if (size + offset > execlen){
                return -2;
            }
            memcpy(buf, entry.execbytes+offset, size);
            return size;
        }
    }
    return -1;
}

/*@}*/
