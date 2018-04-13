/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <lock.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	
	// Handle locks before killing
	// Identify and release all the locks that are held
	//kprintf("###########KILL%d\n", pid);
	int i;
	//int args[NLOCK];
	for(i=NLOCK-1;i>=0;i--) {
		if (proctab[pid].plocks[i].status == L_CREATED) {
			if (proctab[pid].plocks[i].waiting == FALSE) {
				//printf("\n\nkill release %d\n\n", i);
				releaseall(1, i);
			}
			else{
				//printf("\n\n%d dequeue %d\n\n", currpid, pid);
				dequeue(pid);
				set_lprio(i);
			}
		}
	}

	// Continuation
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
