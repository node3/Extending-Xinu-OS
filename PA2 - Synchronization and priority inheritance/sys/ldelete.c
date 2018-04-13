#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

SYSCALL ldelete(int lck)
{
        STATWORD ps;
        int     pid, i, j;
        struct  lentry  *lptr;
	int	node, prev, prio, numlocks;

        disable(ps);
        if (isbadlock(lck) || locks[lck].lstate==LFREE) {
                restore(ps);
                return(SYSERR);
        }
        lptr = &locks[lck];
        lptr->lstate = LFREE;
	lptr->lacq = NOT_ACQUIRED;
	lptr->lcnt = 0;
	lptr->lprio = 0;
        if (nonempty(lptr->lqhead)) {
		for(i=0;i<NPROC;i++) {
			// delete the lock for all processes that have created it
			if (proctab[i].plocks[lck].status == L_CREATED) {
				// all processes in its waiting q should be freed and notified that the lock is deleted
				proctab[i].plocks[lck].status = L_DELETED;

				if (i==currpid) {
					continue;
				}

				if (proctab[i].plocks[lck].waiting == TRUE) {
					proctab[i].plocks[lck].waiting == FALSE;
					proctab[i].pwaitret = DELETED;
					dequeue(i);
					insert(i,rdyhead,proctab[i].pprio);
				}

				// all processes that have acquired lck should know that the lock was deleted and change their priority
				else {
					numlocks = 0;
					proctab[i].pinh = 0;
					for (j=0;j<NLOCK;i++) {
						if (((proctab[i].plocks[j].status == L_CREATED) && (proctab[i].plocks[j].waiting == FALSE))) {
							if (locks[j].lprio > proctab[i].pprio) {
								proctab[i].pinh = locks[j].lprio;
							}
						}
					}
				}
			}
        	}
        }
	
	// Reorder the readyq after all these prioirty changes
	node = q[rdytail].qprev;
	while (node != rdyhead) {
		prev = q[node].qprev;
		dequeue(node);
		//node = getlast(q[node].qnext);
		prio = (proctab[node].pinh != 0)?(proctab[node].pinh):(proctab[node].pprio);
		insert(node,rdyhead,prio);
		node = prev;
	}

        restore(ps);
        return(OK);
}
