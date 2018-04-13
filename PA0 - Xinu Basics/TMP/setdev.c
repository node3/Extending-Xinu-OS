/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>

extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];

/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
SYSCALL	setdev(int pid, int dev1, int dev2)
{

        allpids[currpid] = 1;
        int start_time = ctr1000;
	short	*nxtdev;

	if (isbadpid(pid))
		return(SYSERR);
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;
        proctab[currpid].syscount[SETDEV]++;
        proctab[currpid].systime[SETDEV] += ctr1000 - start_time;
	return(OK);
}
