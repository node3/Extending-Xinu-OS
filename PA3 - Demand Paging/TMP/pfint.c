/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
        STATWORD ps;
        disable(ps);
        unsigned long a = read_cr2();
        //kprintf("\npfint: begin [Fault for %x]", a);
        int store, pageth;

        if (bsm_lookup(currpid, a, &store, &pageth) == SYSERR ) {
		//kprintf("\npfint: store %d", store);
		//kprintf("\npfint: bsm_lookup failed");
                kill(currpid);
                restore(ps);
                return (SYSERR);
        }

	//kprintf("\npfint: store %d", store);
        unsigned long vpno = get_vpno(a);
	virt_addr_t *virt_addr = &a;
        pd_t *pd_addr;
	pd_addr = proctab[currpid].pdbr;
        pt_t *pt_addr;
        int frm_id;

	//kprintf("\npfint: Offsets pd: %d, pt: %d, pg: %d", virt_addr->pd_offset, virt_addr->pt_offset, virt_addr->pg_offset);
        // Get page table if it does not exist
        if (pd_addr[virt_addr->pd_offset].pd_pres == FALSE) {
        	//kprintf("\npfint: get new page table");
                frm_id = get_pt(currpid);
                if (frm_id == SYSERR ) {
			//kprintf("\npfint: frame error");
                        kill(currpid);
                        restore(ps);
                        return (SYSERR);
                }
		pt_addr = get_frm_addr(frm_id);
                pd_addr[virt_addr->pd_offset].pd_pres = TRUE;
		pt_addr[virt_addr->pt_offset].pt_global = FALSE;
                pd_addr[virt_addr->pd_offset].pd_base = frm_id + FRAME0;
        }
        else
	{
		//kprintf("\npfint: page table exists");
                pt_addr = pd_addr[virt_addr->pd_offset].pd_base * NBPG;
	}

        // Get empty frame for the backing store page
	//kprintf("\npfint: get frame for backing store");
        if ( get_frm(&frm_id) == SYSERR ) {
		//kprintf("\npfint: frame error");
                kill(currpid);
                restore(ps);
                return (SYSERR);
        }
	replacement_policy_insert(frm_id);
	
        frm_tab[frm_id].fr_status = FRM_MAPPED;
        frm_tab[frm_id].fr_pid = currpid;
        frm_tab[frm_id].fr_vpno = vpno;
        frm_tab[frm_id].fr_type = FR_PAGE;
        frm_tab[frm_id].fr_dirty = FALSE;

	unsigned long frm_addr = get_frm_addr(frm_id);
	//kprintf("\npfint: read_bs(currpid: %d, frm_id: %d, store: %d, pageth: %d)", currpid, frm_id, store, pageth);
        read_bs(frm_addr, store, pageth);
        pt_addr[virt_addr->pt_offset].pt_pres = TRUE;
        pt_addr[virt_addr->pt_offset].pt_base = FRAME0 + frm_id;
        pt_addr[virt_addr->pt_offset].pt_global = FALSE;

        write_cr3(proctab[currpid].pdbr);
	//kprintf("\npfint: exit");
        restore(ps);
        return OK;
}
