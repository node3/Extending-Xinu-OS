/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>

extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];
/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
SYSCALL getpid()
{
	int start_time = ctr1000;
	allpids[currpid] = 1;

	proctab[currpid].syscount[GETPID]++;
        proctab[currpid].systime[GETPID] += ctr1000 - start_time;
	return(currpid);
}
