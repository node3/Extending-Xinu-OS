#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

SYSCALL lcreate()
{
        STATWORD ps;
        int     lck;

        disable(ps);
        if ((lck=newlock())==SYSERR ) {
                restore(ps);
                return(SYSERR);
        }
        restore(ps);
        return(lck);
}

LOCAL int newlock()
{
        int     lck;
        int     i;

        for (i=0 ; i<NLOCK ; i++) {
                lck=nextlock--;
                if (nextlock < 0)
                        nextlock = NLOCK-1;
                if (locks[lck].lstate==LFREE) {
                        locks[lck].lstate = LUSED;
                        return(lck);
                }
        }
        return(SYSERR);
}
