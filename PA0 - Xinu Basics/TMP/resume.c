/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];
/*----------------------------------------------------------------------n
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */
SYSCALL resume(int pid)
{
        int start_time = ctr1000;
	allpids[currpid] = 1;
	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		restore(ps);
		return(SYSERR);
	}
	prio = pptr->pprio;
	ready(pid, RESCHYES);
	restore(ps);
        proctab[currpid].syscount[RESUME]++;
        proctab[currpid].systime[RESUME] += ctr1000 - start_time;
	return(prio);
}
