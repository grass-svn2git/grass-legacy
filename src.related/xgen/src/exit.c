/**********************************************************************
   exit.c       - exit the program
 *********************************************************************/
/*******************************************************************************
Xgen was developed by Kurt Buehler, while at the Center for Advanced Decision
Support for Water and Environmental Systems (CADSWES), University of Colorado
at Boulder and at the Indiana Water Resources Research Center (IWRRC),
Purdue University for the U.S. Army Construction Engineering Research
Laboratory in support of the Geographical Resources Analysis Support
System (GRASS) software. The example scripts were developed by Ms. Christine
Poulsen of USA-CERL, much thanks goes to her for her work.

Permission to use, copy, modify and distribute without charge this software,
documentation, etc. is granted, provided that this comment is retained,
and that the names of Kurt Buehler, Christine Poulsen, CADSWES, IWRRC,
the University of Colorado at Boulder, Purdue University, or USA-CERL are not
used in advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

The author disclaims all warranties with regard to this software, including
all implied warranties of merchantability and fitness, in no event shall
the author be liable for any special, indirect or consequential damages or
any damages whatsoever resulting from loss of use, data or profits,
whether in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of this
software.
*******************************************************************************/
#include "xgen.h"

/***************************************************************
 *  XgenExit - exit after wiping out any remaining processes
 *             spawned by Xgen.
 *
 *	input 		- an integer status returned on exit()
 *  returns 	- ...never...
 **************************************************************/
void
XgenExit(status)
	int status;
{
	Command *com;
	
	/***************************************************************
	 * kill all pending processes...
	 **************************************************************/
	 for ( com = xgenGD.commandList; com != NULL; com = com->next ) {
		int pgrp;
		char buf[80];

		/* unlink temp files if they exist */
		if ( com->capture && !access(com->tmpfile,0) )
			unlink(com->tmpfile);
		if ( !access(com->errfile,0) )
			unlink(com->errfile);
		/* check for existance */
		if ( kill(com->pid,0) == 0 && errno != ESRCH ) {
			/* get the process group and wipe it out */
			pgrp = getpgrp(com->pid);
			if ( killpg(pgrp,SIGINT) < 0 ) {
				sprintf(buf,"killpg: process group %d",pgrp);
				perror(buf);
			}
	 	}
	 }
	/***************************************************************
	 * and quit....
	 **************************************************************/
	 exit(status);
}
