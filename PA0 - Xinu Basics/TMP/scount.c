/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <proc.h>

extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];

/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
SYSCALL scount(int sem)
{
        int start_time = ctr1000;
	allpids[currpid] = 1;
extern	struct	sentry	semaph[];

	if (isbadsem(sem) || semaph[sem].sstate==SFREE)
		return(SYSERR);
	proctab[currpid].syscount[SCOUNT]++;
        proctab[currpid].systime[SCOUNT] += ctr1000 - start_time;
	return(semaph[sem].semcnt);
}
