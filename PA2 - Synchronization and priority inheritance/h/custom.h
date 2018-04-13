extern struct  qent q[];

void set_lprio(int lck) {
	struct lentry *lptr = &locks[lck];
	int i,j, pinh;
        int node = q[lptr->lqtail].qprev;
	lptr->lprio = 0;
	while	(node != lptr->lqhead)	{
		if (lptr->lprio < proctab[node].pprio)
	                lptr->lprio = proctab[node].pprio;
		node = q[node].qprev;
	}
	for(i=NPROC-1;i>=0;i--){
		if ((proctab[i].plocks[lck].status == L_CREATED) && (proctab[i].plocks[lck].waiting == FALSE)) {
			pinh = 0;
			for(j=NLOCK-1;j>=0;j--) {
				if ((proctab[i].plocks[j].status == L_CREATED) && (proctab[i].plocks[j].waiting == FALSE))
					if (locks[j].lprio > proctab[i].pprio)
						pinh = locks[j].lprio;
			}
			proctab[i].pinh = pinh;
		}
	}
}

void initialize_flagtab(int *flagtab) {
	int i;
	for(i=0;i<NPROC;i++)
		flagtab[i] = 0;
	flagtab[currpid] = 1;
}

void bump_the_priority(int priority, int lck, int *flagtab) {
	int i, j;
	for(i=0;i<NPROC;i++) {
		// Avoid processing the same process (deadlock avoidance)
		if (flagtab[i] == 1)
			return;
		// Bump the priority
		else{
			flagtab[i] = 1;
			// the dependency should have acquired the lock
			if ((proctab[i].plocks[lck].status == L_CREATED) && (proctab[i].plocks[lck].waiting == FALSE) && proctab[i].pprio < priority) {
				proctab[i].pinh = priority;
				for (j=0;j<NLOCK;j++){
					if ((proctab[i].plocks[j].status == L_CREATED)  && (proctab[i].plocks[j].waiting == TRUE)) {
						bump_the_priority(priority, j, flagtab);
					}
				}
			}
		}
	}
}

void rearrange_readyq() {
	int node, prev, prio;
	node = q[rdytail].qprev;
	while(node!=rdyhead){
		prev = q[node].qprev;
		node = getlast(q[node].qnext);
		prio = (proctab[node].pinh==0)?proctab[node].pprio:proctab[node].pinh;
		insert(node,rdyhead,prio);
		node = prev;
	}
}

void revamp(int priority, int lck) {
	int flagtab[NPROC];
	initialize_flagtab(flagtab); //check this
	bump_the_priority(priority, lck, flagtab);
	rearrange_readyq();
}

void strcopy(char var[], char string[]){
	int i;
	for(i=0;string[i]!='\0';i++)
		var[i] = string[i];
	var[i] = string[i];
}

void pplocks(int pid){

	char state[10];
	if(locks[pid].lstate == LFREE)
		strcopy(state,"LFREE");
	else if(locks[pid].lstate == LUSED)
		strcopy(state, "LUSED");
	else
		strcopy(state, "UNKNOWN");

	char lacq[20];
	if(locks[pid].lacq == ACQUIRED_BY_WRITER)
		strcopy(lacq, "ACQUIRED_BY_WRITER");
	else if(locks[pid].lacq == ACQUIRED_BY_READER)
		strcopy(lacq, "ACQUIRED_BY_READER");
	else if(locks[pid].lacq == NOT_ACQUIRED)
		strcopy(lacq, "NOT_ACQUIRED");
	else
		strcopy(lacq, "UNKNOWN");

        printf("\n"
                "\n\tLOCK ID:   %d"
                "\n\tlstate:    %s"
                "\n\tlacq:      %s"
                "\n\tlcnt:      %d"
                "\n\tlprio:     %d"
//                "\n\tlqhead:    %d"
//                "\n\tlqtail:    %d"
		"\n\tQueue: TAIL ",
//                pid, state, lacq, locks[pid].lcnt, locks[pid].lprio, locks[pid].lqhead, locks[pid].lqtail);
                pid, state, lacq, locks[pid].lcnt, locks[pid].lprio);

	int node;
	node = q[locks[pid].lqtail].qprev;
	while(node!= locks[pid].lqhead) {
		printf("<-%d", node);
		node = q[node].qprev;
	}
	printf("<-HEAD\n");
}

void ppprocs(int pid){
	char state[20];
	if (proctab[pid].pstate == PRCURR)
		strcopy(state, "PRCURR");
	else if (proctab[pid].pstate == PRFREE)
		strcopy(state, "PRFREE");
	else if (proctab[pid].pstate == PRREADY)
		strcopy(state, "PRREADY");
        else if (proctab[pid].pstate == PRRECV)
		strcopy(state, "PRRECV");
        else if (proctab[pid].pstate == PRSLEEP)
		strcopy(state, "PRSLEEP");
        else if (proctab[pid].pstate == PRSUSP)
		strcopy(state, "PRSUSP");
        else if (proctab[pid].pstate == PRWAIT)
		strcopy(state, "PRWAIT");
	else
		strcopy(state, "UNKNOWN");

        printf("\n"
                "\n\tPROCESS ID:        %d"
                "\n\tpstate:            %s"
                "\n\tpprio:             %d"
                "\n\tpinh:              %d"
                ,pid, state, proctab[pid].pprio, proctab[pid].pinh);
        int i;
        for(i=0;i<NLOCK;i++){
                if (proctab[pid].plocks[i].status != L_UNDEFINED) {
			char status[20];
			if(proctab[pid].plocks[i].status == L_CREATED)
				strcopy(status, "L_CREATED");
			else if(proctab[pid].plocks[i].status == L_UNDEFINED)
				strcopy(status, "L_UNDEFINED");
			else if(proctab[pid].plocks[i].status == L_DELETED)
				strcopy(status, "L_DELETED");
			else
				strcopy(status, "UNKNOWN");

			char type[20];
			if(proctab[pid].plocks[i].type == READ)
				strcopy(type, "READ");
			else if(proctab[pid].plocks[i].type == WRITE)
				strcopy(type,  "WRITE");
			else 
				strcopy(type, "UNKNOWN");

			char waiting[20];
                        if (proctab[pid].plocks[i].waiting == FALSE)
				strcopy(waiting, "FALSE");
			else if (proctab[pid].plocks[i].waiting == TRUE)
				strcopy(waiting, "TRUE");
                        else
				strcopy(waiting, "UNKNOWN");


                        printf("\n"
                                "\n\t\tLOCK ID:         %d"
                                "\n\t\tstatus:          %s"
                                "\n\t\ttype:            %s"
                                "\n\t\twaiting:         %s"
                                "\n\t\tpriority:        %d"
                                "\n\t\tblocking time:   %ld"
                                ,i, status, type, waiting,
                                //,i, status, waiting,
                                proctab[pid].plocks[i].priority,
                                proctab[pid].plocks[i].blocking_time);
                        pplocks(i);
                }
        }
}
