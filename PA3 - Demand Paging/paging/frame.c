/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

fr_map_t frm_tab[NFRAMES];
int gpt[4];
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
	int x;
	for (x=0; x < NFRAMES; x++) {
		frm_tab[x].fr_status = FRM_UNMAPPED;
		frm_tab[x].fr_pid = NONE;
		frm_tab[x].fr_vpno = NONE;
		frm_tab[x].fr_refcnt = 0;
		frm_tab[x].fr_type = NONE;
		frm_tab[x].fr_dirty = NONE;
	}
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
        STATWORD ps;
        disable(ps);

	int x;
	*avail = NONE;
        for (x=0; x < NFRAMES; x++) {
                if (frm_tab[x].fr_status == FRM_UNMAPPED) {
			*avail = x;
			//kprintf("\nget_frm: got an empty frame %d", x);
			restore(ps);
			return OK;
		}
        }
	// todo if *avail == NONE, replace a page to get a new frame
	if (*avail == NONE) {
		//kprintf("\nget_frm: No empty frame available. Attempting to free a frame");
		x = replacement_policy_remove(NONE);
		if (free_frm(x) == SYSERR)
		{
			restore(ps);
			return SYSERR;
		}
		//kprintf("\nget_frm: free_frm freed frame %d", x);
		*avail = x;
		restore(ps);
		return OK;
	}
  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int id)
{
	if ((id >= NFRAMES) || (id < 0))
		return SYSERR;
        STATWORD ps;
        disable(ps);

        if (frm_tab[id].fr_status == FRM_UNMAPPED) {
		//kprintf("\nfree_frm: Could not free FRAME %d", id);
		restore(ps);
                return SYSERR;
        }
        else {
		if (frm_tab[id].fr_type == FR_DIR) {
			//kprintf("\nfree_frm: freeing FR_DIR at FRAME %d", id);
			int x;
			pd_t *pd = get_frm_addr(id);
			for (x = 0; x < PAGESIZE; x++) { 
				if ((pd[x].pd_pres == TRUE) && (pd[x].pd_global != TRUE)) {
					free_frm(pd[x].pd_base - FRAME0);
				}
			}	
		}
		else if (frm_tab[id].fr_type == FR_TBL) {
			//kprintf("\nfree_frm: freeing FR_TBL at FRAME %d", id);
			int x;
			int frm_id;
			pt_t *pt = get_frm_addr(id);
			for (x = 0; x < PAGESIZE; x++) {
				if (pt[x].pt_pres == TRUE) {
					frm_id = pt[x].pt_base - FRAME0;
					free_frm(frm_id);
					replacement_policy_remove(frm_id);
				}
			}
		}
		else if (frm_tab[id].fr_type == FR_PAGE) {
			//kprintf("\nfree_frm: freeing FR_PAGE at FRAME %d", id);
			unsigned char * frm_addr = get_frm_addr(id);
			unsigned long vaddr = frm_tab[id].fr_vpno*NBPG;
			int store, pageth;
			int pid = frm_tab[id].fr_pid;
			int pd_offset = frm_tab[id].fr_vpno >> 10;
                        int pt_offset = (frm_tab[id].fr_vpno << 22) >> 22;
			pd_t *pd = proctab[pid].pdbr;
	                pt_t *pt = pd[pd_offset].pd_base * NBPG;
			if (bsm_lookup(frm_tab[id].fr_pid, vaddr, &store, &pageth) == SYSERR) {
				//kprintf("\nfree_frm: bsm_lookup failed for VADDR %x, PID %d", vaddr, frm_tab[id].fr_pid);
				restore(ps);
				return SYSERR;
			}
			//kprintf("\nfree_frm: write_bs(%x, %d, %d) FRM_ID: %d", frm_addr, store, pageth, id);
			write_bs(frm_addr, store, pageth);
			pt[pt_offset].pt_pres = FALSE;
			pt[pt_offset].pt_write = FALSE;
			pt[pt_offset].pt_user = FALSE;
			pt[pt_offset].pt_pwt = FALSE;
			pt[pt_offset].pt_pcd = FALSE;
			pt[pt_offset].pt_acc = FALSE;
			pt[pt_offset].pt_mbz = FALSE;
			pt[pt_offset].pt_dirty = FALSE;
			pt[pt_offset].pt_global = FALSE;
			pt[pt_offset].pt_avail = FALSE;
			pt[pt_offset].pt_base = FALSE;
		}
		else {
			//kprintf("\nfree_frm: FR_TYPE %d unknown. FRAME %d not freed.", frm_tab[id].fr_type, id);
		}

                frm_tab[id].fr_status = FRM_UNMAPPED;
		frm_tab[id].fr_pid = NONE;
		frm_tab[id].fr_vpno = NONE;
		frm_tab[id].fr_refcnt = 0;
		frm_tab[id].fr_type = NONE;
		frm_tab[id].fr_dirty = FALSE;
		restore(ps);
                return OK;
        }
}

// My functions
int get_pd(int pid) {
        STATWORD ps;
        disable(ps);

	int x;
	int frm_id;
	pd_t *pd;
	get_frm(&frm_id);
	
	if (frm_id == NONE) {
		//kprintf("\nget_pd: Could not get a free frame");
		restore(ps);
		return SYSERR;
	}
	
	frm_tab[frm_id].fr_status = FRM_MAPPED;
	frm_tab[frm_id].fr_pid = pid;
	frm_tab[frm_id].fr_vpno = NONE;
	frm_tab[frm_id].fr_refcnt = 1;
	frm_tab[frm_id].fr_type = FR_DIR;
	frm_tab[frm_id].fr_dirty = FALSE;	

	proctab[pid].pdbr = get_frm_addr(frm_id);
	for (x = 0; x < 4 ; x++) {
		pd = get_frm_addr(frm_id) + (x * sizeof(pd_t));
		//kprintf("\n%ld", pd);
		pd->pd_pres = TRUE;
		pd->pd_write = TRUE;
		pd->pd_user = FALSE;
		pd->pd_pwt = FALSE;
		pd->pd_pcd = FALSE;
		pd->pd_acc = FALSE;
		pd->pd_mbz = FALSE;
		pd->pd_fmb = FALSE;
		pd->pd_global = TRUE;
		pd->pd_avail = FALSE;
		pd->pd_base = gpt[x];
	}
	for (; x < PAGESIZE ; x++) {
                pd = get_frm_addr(frm_id) + (x * sizeof(pd_t));
                //kprintf("\n%ld", pd);
                pd->pd_pres = FALSE;
                pd->pd_write = FALSE;
                pd->pd_user = FALSE;
                pd->pd_pwt = FALSE;
                pd->pd_pcd = FALSE;
                pd->pd_acc = FALSE;
                pd->pd_mbz = FALSE;
                pd->pd_fmb = FALSE;
                pd->pd_global = FALSE;
                pd->pd_avail = FALSE;
                pd->pd_base = FALSE;
        }
	//kprintf("\nget_pd: PID %d, FRAME %d", pid, frm_id);
	restore(ps);	
	return frm_id;
}

int get_pt(int pid) {
        STATWORD ps;
        disable(ps);

        int x;
        int frm_id;
        pt_t *pt;
        get_frm(&frm_id);

        if (frm_id == NONE) {
                //kprintf("\nget_pt: Could not get a free frame");
		restore(ps);	
                return SYSERR;
        }

        frm_tab[frm_id].fr_status = FRM_MAPPED;
        frm_tab[frm_id].fr_pid = pid;
        frm_tab[frm_id].fr_vpno = NONE;
        frm_tab[frm_id].fr_refcnt = 1;
        frm_tab[frm_id].fr_type = FR_TBL;
        frm_tab[frm_id].fr_dirty = FALSE;

        for (x = 0; x < PAGESIZE ; x++) {
                pt = get_frm_addr(frm_id) + (x * sizeof(pt_t));
                //kprintf("\n%ld", pd);
                pt->pt_pres = FALSE;
                pt->pt_write = FALSE;
                pt->pt_user = FALSE;
                pt->pt_pwt = FALSE;
                pt->pt_pcd = FALSE;
                pt->pt_acc = FALSE;
                pt->pt_mbz = FALSE;
                pt->pt_dirty = FALSE;
                pt->pt_global = FALSE;
                pt->pt_avail = FALSE;
                pt->pt_base = FALSE;
                //pt->pt_base = (frm_id * 1024) + x;
        }
	//kprintf("\nget_pt: PID %d, FRAME %d", pid, frm_id);
	restore(ps);
	return frm_id;
}

int init_gpt() {
        STATWORD ps;
        disable(ps);

	int x, y, frm_id;
	pt_t *pt;
	for (x = 0; x < 4; x++) {
		frm_id = get_pt(NULLPROC);
		if (frm_id == NONE) 
		{
			restore(ps);
			return SYSERR;
		}
		gpt[x] = FRAME0 + frm_id;

		for (y = 0; y < PAGESIZE; y++) {
			pt = get_frm_addr(frm_id) + (y * sizeof(pt_t));
			pt->pt_pres = TRUE;
			pt->pt_global = TRUE;
			pt->pt_base = (frm_id * PAGESIZE) + y;
		}
		//kprintf("\ninit_gpt: initiliased global PT %d at Frame %d", x, frm_id);
	}
	restore(ps);
	return OK;
}

SYSCALL flush_frames(int pid) {
        STATWORD ps;
        disable(ps);
        if (currpid != NULLPROC) {
		//kprintf("\n--------------------------RESCHED---------------------------------");
		int x, y;
		int pd_offset, pt_offset;
		pd_t *pd = proctab[pid].pdbr;
		pt_t *pt;
		for (x = sc_head; x != NONE; x = y) {
			y = sc_tab[x];
			if (frm_tab[x].fr_pid == pid) {
				//kprintf("\nflush_frames: writing frame %d, vpno %x back to backing store", x, frm_tab[x].fr_vpno);
				pd_offset = frm_tab[x].fr_vpno >> 10;
				pt_offset = (frm_tab[x].fr_vpno << 22) >> 22;
				free_frm(x);
				pt = pd[pd_offset].pd_base * NBPG;
				pt[pt_offset].pt_pres = FALSE;
				pt[pt_offset].pt_write = FALSE;
				pt[pt_offset].pt_user = FALSE;
				pt[pt_offset].pt_pwt = FALSE;
				pt[pt_offset].pt_pcd = FALSE;
				pt[pt_offset].pt_acc = FALSE;
				pt[pt_offset].pt_mbz = FALSE;
				pt[pt_offset].pt_dirty = FALSE;
				pt[pt_offset].pt_global = FALSE;
				pt[pt_offset].pt_avail = FALSE;
				pt[pt_offset].pt_base = FALSE;
				replacement_policy_remove(x);
			}
		}
        }
	restore(ps);
	return OK;
}
