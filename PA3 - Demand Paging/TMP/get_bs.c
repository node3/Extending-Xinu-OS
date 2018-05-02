#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {
	//kprintf("\nget_bs: begin (bs_id: %d, npages: %d)", bs_id, npages);
        if ((npages <= 0) || (npages > 128)) {
                return SYSERR;
        }

        STATWORD ps;
        disable(ps);

        /* requests a new mapping of npages with ID map_id */
        if (bs_id >= 0 && bs_id < NBSM) {
                if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED) {
                        bsm_tab[bs_id].bs_status = BSM_MAPPED;
                        bsm_tab[bs_id].bs_pid[currpid] = BSM_MAPPED;
                        bsm_tab[bs_id].bs_npages = npages;
                        bsm_tab[bs_id].bs_private = FALSE;
			//kprintf("\nget_bs: good 1 exit");
			restore(ps);
                        return npages;
                }
                else {
                        if (bsm_tab[bs_id].bs_private == FALSE) {
                                bsm_tab[bs_id].bs_pid[currpid] = BSM_MAPPED;
				//kprintf("\nget_bs: good 2 exit");
				restore(ps);
                                return bsm_tab[bs_id].bs_npages;
                        }
                        else {
				//kprintf("\nget_bs: fail : private bs cannot be alloted");
				restore(ps);
                                return SYSERR;
                        }
                }
        }
        else {
		//kprintf("\nget_bs: fail : bs id out of range");
		restore(ps);
                return SYSERR;
        }
}
