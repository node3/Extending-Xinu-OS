/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD    *vgetmem(nbytes)
        unsigned nbytes;
{
	//kprintf("\nvgetmem: begin");
        STATWORD ps;
        disable(ps);
        struct  mblock  *p, *q, *leftover;
        struct mblock *memlist;
        int vgetmem_failed = TRUE;

        memlist = proctab[currpid].vmemlist;

	//kprintf("\nvgetmem: memlist: %ld, mnext: %ld", memlist, memlist->mnext);
        if (nbytes == 0 || memlist->mnext == (struct mblock *) NULL) {
		//kprintf("\nvgetmem: memlist failure 1");
                restore(ps);
                return((WORD *)SYSERR);
        }
        nbytes = (unsigned int) roundmb(nbytes);
        for (q=memlist,p=memlist->mnext ; p != (struct mblock *) NULL ; q=p,p=p->mnext)
                if ( p->mlen == nbytes) {
                        q->mnext = p->mnext;
			//kprintf("\nvgetmem: sucess end");
                	restore(ps);
			return (WORD *)p;
                } else if ( p->mlen > nbytes ) {
                        leftover = (struct mblock *)( (unsigned)p + nbytes );
                        q->mnext = leftover;
                        leftover->mnext = p->mnext;
                        leftover->mlen = p->mlen - nbytes;
			//kprintf("\nvgetmem: success end");
                	restore(ps);
			return (WORD *)p;
                }
	//kprintf("\nvgetmem: fail end");
	restore(ps);
	return (WORD *)SYSERR;
}
