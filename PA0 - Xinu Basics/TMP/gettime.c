/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <proc.h>

extern int getutim(unsigned long *);
extern long ctr1000;
extern int currpid;
extern int allpids[NPROC];
/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{
    /* long	now; */

	/* FIXME -- no getutim */
        int start_time = ctr1000;
	allpids[currpid] = 1;


        proctab[currpid].syscount[GETTIME]++;
        proctab[currpid].systime[GETTIME] += ctr1000 - start_time;
    return OK;
}
