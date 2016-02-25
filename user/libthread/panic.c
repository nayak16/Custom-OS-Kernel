/* 
 * Copyright (c) 1996-1995 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <simics.h>

/*
 * This function is called by the assert() macro defined in assert.h;
 * it's also a nice simple general-purpose panic function.
 */
void panic(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);

	printf("\n");

	volatile static int side_effect = 0;
	while (1) {
		// exact authorship uncertain, popularized by Heinlein
		printf("When in danger or in doubt, run in circles, scream and shout.\n");
		lprintf("When in danger or in doubt, run in circles, scream and shout.");
		++side_effect;
	}
}
