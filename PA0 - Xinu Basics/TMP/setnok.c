/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];

/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
SYSCALL	setnok(int nok, int pid)
{
        allpids[currpid] = 1;
        int start_time = ctr1000;
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);
        proctab[currpid].syscount[SETNOK]++;
        proctab[currpid].systime[SETNOK] += ctr1000 - start_time;
	return(OK);
}
