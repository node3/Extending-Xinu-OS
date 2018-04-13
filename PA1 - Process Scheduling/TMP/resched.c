/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>

unsigned long currSP;   /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:       Upon entry, currpid gives current process id.
 *              Proctab[currpid].pstate gives correct NEXT state for
 *                      current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int default_sched()
{
        register struct pentry  *optr;  /* pointer to old process entry */
        register struct pentry  *nptr;  /* pointer to new process entry */

        /* no switch needed if current process priority higher than next*/

        if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
                 (lastkey(rdytail)<optr->pprio)) {
                return(OK);
        }

        /* force context switch */

        if (optr->pstate == PRCURR) {
                optr->pstate = PRREADY;
                insert(currpid,rdyhead,optr->pprio);
        }

        /* remove highest priority process at end of ready list */

        nptr = &proctab[ (currpid = getlast(rdytail)) ];
        nptr->pstate = PRCURR;          /* mark it currently running    */
#ifdef  RTCLOCK
        preempt = QUANTUM;              /* reset preemption counter     */
#endif

        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

        /* The OLD process returns here when resumed. */
        return OK;
}

void setschedclass(int sched_class)
{
        SCHED_CLASS = sched_class;
}

int getschedclass()
{
        return SCHED_CLASS;
}

int resched()
{

        if (getschedclass() == RANDOMSCHED)
                return rand_sched();
        else
                return linux_like_sched();
}

int rand_sched()
{
        int     node;
        int     probability;
        int     priority_sum=0;
        register struct pentry  *optr;  /* pointer to old process entry */
        register struct pentry  *nptr;  /* pointer to new process entry */

        /* add the current process to ready queue */
        optr = &proctab[currpid];
        if (optr->pstate == PRCURR)
        {
                optr->pstate = PRREADY;
                insert(currpid,rdyhead,optr->pprio);
        }

        /* get the priority sum */
        node = q[rdyhead].qnext;
        while (node != rdytail)
        {
                priority_sum += q[node].qkey;
                node = q[node].qnext;
        }

        /* get random probability */
        if (priority_sum != 0)
	{
                probability = rand() % priority_sum;
		int pp = probability;
		/* get the process for which probability < node.qkey (considered in the decreasing priority order)  */
		node = q[rdytail].qprev;
		while (probability > q[node].qkey)
		{
			probability -= q[node].qkey;
			node = q[node].qprev;
		}
	}
        else
	{
		if (currpid != 0)
			node = 0;		
	}
        /*int n = q[rdyhead].qnext;
        while (n != rdytail)
        {
                printf("%d->", n);
                n = q[n].qnext;
        }
	*/
        /* extract the process to be scheduled from the ready queue */
        nptr = &proctab[ (currpid = getlast(q[node].qnext)) ];
        //printf(" %d, %d, %s, %d\n", pp, currpid, proctab[currpid].pname, proctab[currpid].pprio);
        nptr->pstate = PRCURR;          /* mark it currently running    */
#ifdef  RTCLOCK
        preempt = QUANTUM;              /* reset preemption counter     */
#endif

        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

        /* The OLD process returns here when resumed. */
        return OK;
}


int linux_like_sched()
{
        int     node, probability;
        register struct pentry  *optr;  /* pointer to old process entry */
        register struct pentry  *nptr;  /* pointer to new process entry */
        optr = &proctab[currpid];

        /* Check if the process was prempted before its counter expired */
        if (preempt <= 0) {
                optr->goodness = 0;
                optr->counter = 0;
        }
        else
                optr->counter = preempt;

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->goodness);
	}

        /* get the process with max goodness from proctab */
        node = q[rdyhead].qnext;
        int max_goodness = 0;
        int max_goodness_pid = 0;
        while (node != rdytail)
        {
                if (max_goodness < proctab[node].goodness)
                {
                        max_goodness = proctab[node].goodness;
                        max_goodness_pid = node;
                }
                node = q[node].qnext;
        }

        /* New epoch starts, initialize goodness and quantum for all processes in ready queue */
        if (max_goodness == 0)
        {
                //printf("\n");
                node = q[rdyhead].qnext;
                max_goodness = 0;
                max_goodness_pid = 0;
		
		int i;
		for(i=NPROC-1; i>0; i--)
		{
			if (proctab[i].pstate != PRFREE)
			{
				proctab[i].quantum = proctab[i].counter/2 + proctab[i].pprio;
				proctab[i].goodness = proctab[i].counter + proctab[i].pprio;
				proctab[i].counter = proctab[i].quantum;
				if (max_goodness < proctab[i].goodness)
				{
					max_goodness = proctab[i].goodness;
					max_goodness_pid = i;
				}
				// try bubbble sort instead???
				if (proctab[i].pstate == PRREADY)
				{
					getlast(q[i].qnext);
					insert(i, rdyhead, proctab[i].goodness);	
				}
			}
		}
        }
        /* Update the goodness for other processes if it is not a new epoch*/
        else
        {
                node = q[rdyhead].qnext;
                while (node != rdytail)
                {
                        if (proctab[node].goodness != 0){
                                proctab[node].goodness = proctab[node].counter + proctab[node].pprio;
                        }
                        node = q[node].qnext;
                }
        }

        /* Schedule process with max goodness */
        nptr = &proctab[ (currpid = getlast(q[max_goodness_pid].qnext)) ];
       	// printf("\nScheduling %d, %s, %d, %d, %d\n", currpid, proctab[currpid].pname, proctab[currpid].pprio, proctab[currpid].goodness, proctab[currpid].counter);
        nptr->pstate = PRCURR;          /* mark it currently running    */
#ifdef  RTCLOCK
#endif
        /* reset preemption counter     */
        if (currpid == 0)
                preempt = QUANTUM;
        else
                preempt = nptr->counter;

        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

        /* The OLD process returns here when resumed. */
        return OK;
}
