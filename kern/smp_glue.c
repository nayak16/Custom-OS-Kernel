/*
 *
 *    #####          #######         #######         ######            ###
 *   #     #            #            #     #         #     #           ###
 *   #                  #            #     #         #     #           ###
 *    #####             #            #     #         ######             #
 *         #            #            #     #         #
 *   #     #            #            #     #         #                 ###
 *    #####             #            #######         #                 ###
 *
 *   This file is included for completeness (having a tss_desc_create()
 *   function is necessary to link against libsmp) but YOU ARE NOT EXPECTED
 *   TO ACTUALLY USE IT unless it is the Spring semester of 2012.  JUST 
 *   PRETEND YOU NEVER SAW IT HERE.  If you have any questions about why
 *   it is here, DELETE IT BEFORE ASKING, at which point you will be asking
 *   a question about a file that does not exist, which is unlikely to be
 *   productive.  Fnord.
 *
 */






/*
 *
 *    #####          #######         #######         ######            ###
 *   #     #            #            #     #         #     #           ###
 *   #                  #            #     #         #     #           ###
 *    #####             #            #     #         ######             #
 *         #            #            #     #         #
 *   #     #            #            #     #         #                 ###
 *    #####             #            #######         #                 ###
 *
 *
 *   This file NEEDS WORK or SHOULD BE DELETED (your choice).
 *
 *   You are responsible for making tss_desc_create() work in accordance
 *   with the specification provided in the handout.  Your implementation
 *   can live in this smp_glue.c file or elsewhere, as you choose.
 *
 *   If you wish, you can add an AP entry-point function to this file, also
 *   in accordance with the documentation found in the handout.
 *
 */

#include <smp.h>
#include <stdlib.h>

uint64_t
tss_desc_create(void *tss, size_t tss_size)
{
	uint64_t seg = 0LL;
	(void) tss_size;
	panic(__FILE__);
	return seg;
}
