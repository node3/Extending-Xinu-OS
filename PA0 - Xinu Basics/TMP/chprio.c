/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	int start_time = ctr1000;
	allpids[currpid] = 1;

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);
        proctab[currpid].syscount[CHPRIO]++;
        proctab[currpid].systime[CHPRIO] += ctr1000 - start_time;
	return(newprio);
}
