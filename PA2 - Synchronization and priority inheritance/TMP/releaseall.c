#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
//#include <custom.h>

extern unsigned long ctr1000;

int get_next_node_of_type(int lck1, int type, int lqtail1, int lqhead1){
	int next_pid1;
	next_pid1 = q[lqtail1].qprev;
	//printf("get_next_node_of_type(lck=%d, type=%d, lqtail=%d, lqhead=%d)", lck1 ,type,lqtail1, lqhead1);
	while (next_pid1 != lqhead1) {
		if (proctab[next_pid1].plocks[lck1].type == type){
			return next_pid1;
		}
		next_pid1 = q[next_pid1].qprev;
	}
	return -1;
}

void release_highest_lock_priority_process(int lck, int hpid){
	int prio;
	dequeue(hpid);
	//printf("\nRELEASE: release_highest_lock_priority_process %d\n", hpid);
	prio = (proctab[hpid].pinh != 0)?(proctab[hpid].pinh):(proctab[hpid].pprio);
	proctab[hpid].pstate = PRREADY;
        insert(hpid,rdyhead,prio);
        proctab[hpid].plocks[lck].blocking_time = 0;
	proctab[hpid].plocks[lck].waiting = FALSE;
}

// Release a process for a lock and pid
void release_reader(int newpid, int lck, struct lentry *lptr){
	int pid = getlast(q[newpid].qnext);
	int prio = (proctab[pid].pinh==0)?proctab[pid].pprio:proctab[pid].pinh;
	proctab[newpid].pstate = PRREADY;
	insert(pid,rdyhead,prio);
	proctab[pid].plocks[lck].blocking_time = 0;
	proctab[pid].plocks[lck].waiting = FALSE;
	lptr->lcnt++;
}

//void yy(int msg) {
//	kprintf("\n*******RELEASE: %s*******\n", msg);
//}

SYSCALL releaseall (int numlocks, int args)
{
	STATWORD ps;    
	register struct	lentry	*lptr;
	int return_flag = OK;
	int i, lck, pid, newpid;
	int equal_lock_priority, priority, newpriority, next_writer_pid, next_reader_pid, writer_blocking_time, writer_priority;
	int *lckptr = (int *)(&args) + (numlocks-1);
	disable(ps);
	
	while(numlocks>0) {
		//printf("\n%d, %d", *lckptr, numlocks);
		lck = *lckptr;
		lptr= &locks[*lckptr];
		//lptr = &locks[*args];

		if (isbadlock(lck) || lptr->lstate==LFREE || proctab[currpid].plocks[lck].status != L_CREATED) {
			return_flag = SYSERR;
	//yy("badlock");
		}
		// reduce reader count
		else if ((lptr->lacq == ACQUIRED_BY_READER) && (lptr->lcnt > 1)) {
			lptr->lcnt--;
			proctab[currpid].plocks[lck].status = L_DELETED;
	//yy("reduce reader count");
		}
		// unblock a process
		else{
	//yy("unblock a process");

                        lptr->lcnt=0;
			proctab[currpid].plocks[lck].status = L_DELETED;
			// no processes within the queue
			if (q[lptr->lqtail].qprev == lptr->lqhead){
				lptr->lacq = NOT_ACQUIRED;
	//yy("no process in the queue");
			}
			// check for the next process
			else {
	//yy("check for next process");

				//Get highest priority writer and reader
				next_writer_pid = get_next_node_of_type(lck, WRITE, lptr->lqtail, lptr->lqhead);
				//printf("\nNext writer proc %d\n", next_writer_pid);
				next_reader_pid = get_next_node_of_type(lck, READ, lptr->lqtail, lptr->lqhead);
				//printf("\nNext reader proc %d\n", next_reader_pid);
			
				//No writer process
				if (next_writer_pid == -1) {
	//yy("No writer process");
					newpid = q[lptr->lqtail].qprev;
                                        while (newpid != lptr->lqhead) {
						pid = q[newpid].qprev;
						release_highest_lock_priority_process(lck, newpid);
						newpid = pid;
						lptr->lcnt++;
					}
					lptr->lacq = ACQUIRED_BY_READER;
				}

				//No readers or Writer has higher priority than all readers
				else if ((next_reader_pid == -1) || (q[next_writer_pid].qkey > q[next_reader_pid].qkey))	{
	//yy("No reader or writer has max priority");
					release_highest_lock_priority_process(lck, next_writer_pid);
					lptr->lacq = ACQUIRED_BY_WRITER;
				}

				//Reader has higher priority
				else if (q[next_writer_pid].qkey < q[next_reader_pid].qkey)	{
	//yy("Reader has higher priority");
					//only the readers having less than 0.5 second longer waiting time would be admitted
					unsigned long writer_blocking_time = proctab[next_writer_pid].plocks[lck].blocking_time + 500;
					int writer_priority = proctab[next_writer_pid].plocks[lck].priority;
					//kprintf("*********************???????????????????????%ld\n", writer_blocking_time);

					newpid = q[lptr->lqtail].qprev;

                                        while 	(newpid != lptr->lqhead) { 
						// until there are processes to consider
						// the process should be a reader 
						// the process should have more priority than writer 
						// or the process should have equal priority as writer and it waiting time should be greater
						pid = q[newpid].qprev;
						if 	((proctab[newpid].plocks[lck].type == READ) && 
							((proctab[newpid].plocks[lck].priority > writer_priority) ||
							((proctab[newpid].plocks[lck].priority == writer_priority) && 
							(proctab[newpid].plocks[lck].blocking_time < writer_blocking_time))))
						{
							release_reader(newpid, lck, lptr);
						}
						newpid = pid;
					}
					resched();
					lptr->lacq = ACQUIRED_BY_READER;
				}

				//Same priority of writer and reader
				else {
	//yy("Reader and writer have same priority");
					if (proctab[next_writer_pid].plocks[lck].blocking_time < proctab[next_reader_pid].plocks[lck].blocking_time) {
						release_highest_lock_priority_process(lck, next_writer_pid);
						lptr->lacq = ACQUIRED_BY_WRITER;	
					}
					else{
						//only the readers having less than 0.5 second longer waiting time would be admitted
						unsigned long writer_blocking_time = proctab[next_writer_pid].plocks[lck].blocking_time + 500;
						int writer_priority = proctab[next_writer_pid].plocks[lck].priority;

						newpid = q[lptr->lqtail].qprev;

						while (newpid != lptr->lqhead) { 
								// until there are processes to consider
								// the process should be a reader
								// the process should have equal priority as writer and it waiting time should be greater than or equal to it
							pid = q[newpid].qprev;
							if (	(proctab[newpid].plocks[lck].type == READ) && 
								(proctab[newpid].plocks[lck].priority == writer_priority) && 
								(proctab[newpid].plocks[lck].blocking_time <= writer_blocking_time))
							{
								release_reader(newpid, lck, lptr);
							}
							newpid = pid;
						}
						lptr->lacq = ACQUIRED_BY_READER;
					}
				}
			}
		}
		set_lprio(lck);
		--numlocks;
		--lckptr;
	}
	
	// Bump down prioirty to the highest lprio of all the locks still held by this process
	proctab[i].pinh = 0;
	for (i=0;i<NLOCK;i++) {
		if (((proctab[currpid].plocks[i].status == L_CREATED) && (proctab[currpid].plocks[i].waiting == FALSE))) {
			if (locks[i].lprio > proctab[i].pprio) {
				proctab[i].pinh = locks[i].lprio;
			}
		}
	}

	restore(ps);
	return return_flag;
}
