/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t store, int npages)
{
	STATWORD ps;
        disable(ps);

	//kprintf("\nxmmap: store: %d, npages: %d mapping to vpno %d for pid %d", store, npages, virtpage, currpid );
        if (    (virtpage >= VIRTUAL_MEMORY_BASE / NBPG) && \
                (bsm_tab[store].bs_status == BSM_MAPPED) && \
                (bsm_tab[store].bs_pid[currpid] == BSM_MAPPED) && \
                (npages > 0) && \
                (npages <= bsm_tab[store].bs_npages)) {
			bsm_tab[store].bs_vpno[currpid] = virtpage;
			//kprintf("\nxmmap: good call");
			restore(ps);
			return OK;
        }
        else {
		/*
		if (virtpage >= VIRTUAL_MEMORY_BASE / NBPG) 
			kprintf("\nxmmap: 1");
		if (bsm_tab[store].bs_status == BSM_MAPPED)
			kprintf("\nxmmap: 2");
		if (bsm_tab[store].bs_pid[currpid] == BSM_MAPPED)
			kprintf("\nxmmap: 3");
		if (npages > 0)
			kprintf("\nxmmap: 4");
		if (npages <= bsm_tab[store].bs_npages)
			kprintf("\nxmmap: 5");
		*/
                //kprintf("\nxmmap: bad call");
		restore(ps);
                return SYSERR;
        }
}

/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
        int i;
	STATWORD ps;
        disable(ps);
	//kprintf("\nxmunmap: removing mapping for vpno: %x for pid: %d", virtpage, currpid);
	pd_t *pd = (pd_t*) (proctab[currpid].pdbr);
	unsigned long pd_offset = virtpage >> 10;
	int pt_frm_id;

        for (i = 0; i < NBSM; i++) {
                if ((bsm_tab[i].bs_status == BSM_MAPPED) && (bsm_tab[i].bs_pid[currpid] == BSM_MAPPED) && (bsm_tab[i].bs_vpno[currpid] == virtpage)) {
			if (pd[pd_offset].pd_pres == TRUE) {
				pt_frm_id = pd[pd_offset].pd_base - FRAME0;
				//kprintf("\nxmunmap: removing page table");
				free_frm(pt_frm_id);
				//if (free_frm(pt_frm_id) == SYSERR)
				//	kprintf("\nxmunmap: free_frm(%d) failed", pt_frm_id);
				pd[pd_offset].pd_base = FALSE;
				pd[pd_offset].pd_global = FALSE;
				pd[pd_offset].pd_pres = FALSE;
			}
                        bsm_tab[i].bs_vpno[currpid] = NONE;
        		//kprintf("\nxmunmap: One virtual address unmapped from BS %d", i);
			restore(ps);
        		return OK;
                }
        }

        //kprintf("\nxmunmap: No BS unmapped");
	restore(ps);
        return SYSERR;
}
