/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL vfreemem(block, size)
        struct  mblock  *block;
        unsigned size;
{
        // Add to the freemem list of the process
        STATWORD ps;
        disable(ps);
	//kprintf("\nvfreemem: begin [block: %d, size: %d]", block, size);
        struct  mblock  *p, *q;
        unsigned top;
        struct  pentry  *pptr = &proctab[currpid];
        unsigned long vend = 4096 * (128 + 4096);
        struct mblock *memlist = proctab[currpid].vmemlist;

        if (size==0 || ((unsigned)block<(unsigned)(4096*4096) || ((unsigned)block)>(4096 * (proctab[currpid].vhpnpages + 4096))))
	{	
		//kprintf("\nvfreemem: fail 1 end");
                restore(ps);
                return(SYSERR);
	}
        size = (unsigned)roundmb(size);
        for( p=memlist->mnext,q= memlist; p != (struct mblock *) NULL && p < block ; q=p,p=p->mnext )
                ;
        if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= memlist) || (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
		//kprintf("\nvfreemem: fail 2 end");
                restore(ps);
                return(SYSERR);
        }
        if ( q!= memlist && top == (unsigned)block )
                        q->mlen += size;
        else {
                block->mlen = size;
                block->mnext = p;
                q->mnext = block;
                q = block;
        }
        if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
                q->mlen += p->mlen;
                q->mnext = p->mnext;
        }

        // remove frame and page table entry
        pd_t *pd = (pd_t*) (pptr->pdbr);
        unsigned long npages = size / NBPG;
	unsigned long vpno = pptr->vhpno;
	unsigned long pd_offset = vpno >> 10;
	unsigned long pt_frm_id = pd[pd_offset].pd_base - FRAME0;
        //kprintf("\nvfreemem: removing page mappings [vpno: %d] [pd_offset: %d] [pt_frm_id: %d]", vpno, pd_offset, pt_frm_id);
	pd[pd_offset].pd_base = FALSE;
	pd[pd_offset].pd_global = FALSE;
	pd[pd_offset].pd_pres = FALSE;
	free_frm(pt_frm_id);
	//if (free_frm(pt_frm_id) == SYSERR)
		//kprintf("\nvfreemem: free_frm(%d) failed", pt_frm_id);

        //kprintf("\nvfreemem: success end");
        restore(ps);
        return(OK);
}
