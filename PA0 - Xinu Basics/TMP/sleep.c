/* sleep.c - sleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * sleep  --  delay the calling process n seconds
 *------------------------------------------------------------------------
 */
extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];

SYSCALL	sleep(int n)
{
	int start_time = ctr1000;
	allpids[currpid] = 1;
	STATWORD ps;    
	if (n<0 || clkruns==0)
		return(SYSERR);
	if (n == 0) {
	        disable(ps);
		resched();
		restore(ps);
		return(OK);
	}
	while (n >= 1000) {
		sleep10(10000);
		n -= 1000;
	}
	if (n > 0)
		sleep10(10*n);

	proctab[currpid].syscount[SLEEP]++;
	proctab[currpid].systime[SLEEP] += ctr1000 - start_time;
	return(OK);
}
