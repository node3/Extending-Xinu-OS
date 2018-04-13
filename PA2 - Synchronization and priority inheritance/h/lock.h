#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCK
#define NLOCK            50
#endif

#define isbadlock(l)     (l<0 || l>=NLOCK)

// Use these for a lstate
#define LFREE	1
#define LUSED	2

// Use these for type
#define READ	0
#define WRITE	1

// Use these for lacq
#define ACQUIRED_BY_WRITER	2
#define ACQUIRED_BY_READER	1
#define NOT_ACQUIRED	0

void linit();

struct  lentry  {
        char    lstate;	 // LFREE or LUSED
	int	lacq;	 // ACQUIRED_BY_WRITER, ACQUIRED_BY_READER or NOT_ACQUIRED
        int     lcnt;    // number of reader processes executing in the critical section
	int 	lprio;
        int     lqhead;
        int     lqtail;
};

extern  struct  lentry  locks[];
extern  int     nextlock;

#endif

