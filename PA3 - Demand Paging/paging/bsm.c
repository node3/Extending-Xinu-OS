/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bsm_tab[NBSM];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	int i;
        for (i=0 ; i<NBSM ; i++)
                bsm_tab[i].bs_status = BSM_UNMAPPED;
        return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
        STATWORD ps;
        disable(ps);
        int i;
        *avail = NONE;
        for (i=0 ; i<NBSM ; i++) {
                if (bsm_tab[i].bs_status == BSM_UNMAPPED) {
                        *avail = i;
                        break;
                }
        }
	restore(ps);
	return OK;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD ps;
        disable(ps);
        if ((i >= 0) && (i < NBSM)) {
                bsm_tab[i].bs_status = BSM_UNMAPPED;
        }
	restore(ps);
        return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int* store, int* pageth)
{
        STATWORD ps;
        disable(ps);

        int i;
	//kprintf("\nbsm_lookup: begin (PID: %d, VADDR: %x)", pid, vaddr);
	vaddr = vaddr >> 12;
        for (i = 0; i < NBSM; i++) {
                if (    (bsm_tab[i].bs_status == BSM_MAPPED) && 
			(bsm_tab[i].bs_pid[pid] == BSM_MAPPED) && 
			(bsm_tab[i].bs_vpno[pid] <= vaddr) &&
			((bsm_tab[i].bs_vpno[pid] + bsm_tab[i].bs_npages) > vaddr))  {
				//kprintf("\nbsm_lookup: VADDR/4096: %d, bs_vpno: %d, bs_npages: %d", vaddr, bsm_tab[i].bs_vpno[pid], bsm_tab[i].bs_npages[pid]);
				//kprintf("\nbsm_lookup: store %d", i);
                        	*store = i;
	                        *pageth = vaddr - bsm_tab[i].bs_vpno[pid];
				//kprintf("\nbsm_lookup: success");
				return OK;
                }
        }
	//kprintf("\nbsm_lookup: Store: %d, Pageth: %d", *store, *pageth);
	//kprintf("\nbsm_lookup: failed");
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
}


