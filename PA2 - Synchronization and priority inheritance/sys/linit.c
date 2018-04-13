#include <lock.h>

int nextlock;
struct  lentry  locks[NLOCK];

void linit(){
	nextlock = NLOCK-1;

	struct  lentry  *lptr;
	int i;
	for (i=0 ; i<NLOCK ; i++) {
		(lptr = &locks[i])->lstate = LFREE;
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
		lptr->lacq = NOT_ACQUIRED;
		lptr->lcnt = 0;
	}
}
