#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
        /* requests a new mapping of npages with ID map_id */
	STATWORD ps;
        disable(ps);

        if ((bs_id >= 0 && bs_id < 16) && (bsm_tab[bs_id].bs_status == BSM_MAPPED)) {
			//kprintf("\nrelease_bs: releasing bs %d for pid %d", bs_id, currpid);
                        bsm_tab[bs_id].bs_pid[currpid] = BSM_UNMAPPED;
                        bsm_tab[bs_id].bs_private = FALSE;

			int i;
			int bs_status = BSM_UNMAPPED;
			for (i=NPROC-1; i >= 0; i--) {
				if (bsm_tab[bs_id].bs_pid[i] == BSM_MAPPED) {
					bs_status = BSM_MAPPED;
					break;
				}
			}
                        bsm_tab[bs_id].bs_status = bs_status;
			if (bs_status != BSM_MAPPED) {
	                        bsm_tab[bs_id].bs_npages = 128;
			}
			restore(ps);
                        return OK;
        }
        else {
		restore(ps);
                return SYSERR;
        }
}
