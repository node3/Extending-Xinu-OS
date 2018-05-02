/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL   newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
        int     *procaddr;              /* procedure address            */
        int     ssize;                  /* stack size in words          */
        int     hsize;                  /* virtual heap size in pages   */
        int     priority;               /* process priority > 0         */
        char    *name;                  /* name (for debugging)         */
        int     nargs;                  /* number of args that follow   */
        long    args;                   /* arguments (treated like an   */
                                        /* array in the code)           */
{
	STATWORD ps;
	//kprintf("\nvcreate: begin");
	// process setup
	struct  pentry  *pptr;
	int pid = create(procaddr,ssize,priority,name,nargs,args);
	pptr = &proctab[pid];
        pptr->vhpnpages = hsize;

	// BSM setup
        get_bsm(&(pptr->store));
        if (pptr->store == SYSERR) {
        	restore(ps);
                return SYSERR;
        }
        bsm_tab[pptr->store].bs_pid[pid] = BSM_MAPPED;
        bsm_tab[pptr->store].bs_status = BSM_MAPPED;
        bsm_tab[pptr->store].bs_vpno[pid] = VIRTUAL_MEMORY_BASE >> 12;
	pptr->vhpno = VIRTUAL_MEMORY_BASE >> 12;
        bsm_tab[pptr->store].bs_npages = hsize;
        bsm_tab[pptr->store].bs_private = TRUE;
	//kprintf("\nvcreate: PID %d, Backing store %d, vpno %d, hsize %d", pid, pptr->store, bsm_tab[pptr->store].bs_vpno[pid], bsm_tab[pptr->store].bs_npages[pid]);

        disable(ps);
        // vmemlist setup
	struct  mblock  *mptr;
	mptr = (struct mblock *) get_bs_addr(pptr->store);
        mptr->mnext = (struct mblock *) NULL;
        mptr->mlen = hsize * NBPG;
        (pptr->vmemlist)->mnext = VIRTUAL_MEMORY_BASE;
	(pptr->vmemlist)->mlen = 0;

	//kprintf("\nvcreate: vmemlist->mnext: %x, vmemlist->mlen: %d, mptr->mnext: %x, mptr->mlen: %d", 
	//	(pptr->vmemlist)->mnext, (pptr->vmemlist)->mlen, mptr->mnext, mptr->mlen);
	//kprintf("\nvcreate: end");
        restore(ps);

        return(pid);
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL   newpid()
{
        int     pid;                    /* process id to return         */
        int     i;

        for (i=0 ; i<NPROC ; i++) {     /* check all NPROC slots        */
                if ( (pid=nextproc--) <= 0)
                        nextproc = NPROC-1;
                if (proctab[pid].pstate == PRFREE)
                        return(pid);
        }
        return(SYSERR);
}
