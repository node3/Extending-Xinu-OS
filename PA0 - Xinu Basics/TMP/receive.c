/* receive.c - receive */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];
/*------------------------------------------------------------------------
 *  receive  -  wait for a message and return it
 *------------------------------------------------------------------------
 */
SYSCALL	receive()
{
        int start_time = ctr1000;
	allpids[currpid] = 1;

	STATWORD ps;    
	struct	pentry	*pptr;
	WORD	msg;

	disable(ps);
	pptr = &proctab[currpid];
	if ( !pptr->phasmsg ) {		/* if no message, wait for one	*/
		pptr->pstate = PRRECV;
		resched();
	}
	msg = pptr->pmsg;		/* retrieve message		*/
	pptr->phasmsg = FALSE;
	restore(ps);
        proctab[currpid].syscount[RECEIVE]++;
        proctab[currpid].systime[RECEIVE] += ctr1000 - start_time;
	return(msg);
}
