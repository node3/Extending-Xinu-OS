#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
#include <custom.h>

extern unsigned long ctr1000;
extern int currpid;

// Inserts currpid to lock's queue and reschedules
void insert_2_lockq_and_resched(int ldes1, struct lentry *lptr, int priority, STATWORD ps){
	proctab[currpid].plocks[ldes1].blocking_time = ctr1000;
	proctab[currpid].pstate = PRSUSP;
	proctab[currpid].plocks[ldes1].waiting = TRUE;
	insert(currpid, lptr->lqhead, priority);
	set_lprio(ldes1);
	restore(ps);
	resched();
}

SYSCALL lock(int ldes1, int type, int priority)
{
        STATWORD ps;
        struct  lentry  *lptr;
	int node;

	// kprintf("***********%d, %d, %d, %d\n", ldes1, type, priority, lptr->lacq);
        disable(ps);
	lptr= &locks[ldes1];

	// invalid criteria to access the lock
        if ((isbadlock(ldes1) || (lptr->lstate)==LFREE) || (proctab[currpid].plocks[ldes1].status != L_CREATED)) {
                restore(ps);
                return(SYSERR);
        }
	
	proctab[currpid].plocks[ldes1].type = type;
	proctab[currpid].plocks[ldes1].priority = priority;
	proctab[currpid].plocks[ldes1].status = L_CREATED;
	
	// if first one to acquire
	if ((lptr->lacq) == NOT_ACQUIRED) {
		if (type == READ)
		{
			lptr->lcnt = 1;
			lptr->lacq = ACQUIRED_BY_READER;
		}
		else
			lptr->lacq = ACQUIRED_BY_WRITER;
		proctab[currpid].plocks[ldes1].waiting = FALSE;
	}
	else {
		// Acquired by reader
		if (lptr->lacq == ACQUIRED_BY_READER) {
			if (type == READ) {
				node = q[lptr->lqtail].qprev;
				// if the q is not empty and the last node (writer) in the q has highest priority
				if ((node != (lptr->lqhead)) && (q[node].qkey > priority)) {
					revamp(proctab[currpid].pprio, ldes1);
					insert_2_lockq_and_resched(ldes1, lptr, priority, ps);
				}
				else{
					//Let the new reader acquire this lock
					lptr->lcnt++;
					proctab[currpid].plocks[ldes1].waiting = FALSE;
				}
			}
			else{
				revamp(proctab[currpid].pprio, ldes1);
				insert_2_lockq_and_resched(ldes1, lptr, priority, ps);
			}
		}

		// Acquired by writer
		else{
			revamp(proctab[currpid].pprio, ldes1);
			insert_2_lockq_and_resched(ldes1, lptr, priority, ps);
		}
	}
	restore(ps);
	return(OK);
}

