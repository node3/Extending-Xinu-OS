/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>

extern int page_replace_policy;
int sc_tab[NFRAMES];
int sc_head;
int sc_tail;

/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
        if (policy == LFU)
                page_replace_policy = LFU;
        else if (policy = SC)
                page_replace_policy = SC;
        else
                return SYSERR;
        return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}

// my functions
SYSCALL init_policy() {
	int i;
	for (i=0;i<NFRAMES;i++) 
		sc_tab[i] = NONE;
	sc_head = NONE;
	sc_tail = NONE;
}

SYSCALL replacement_policy_insert(int frm_id) {
	int policy = grpolicy();
	if (policy == SC) {
		return sc_enqueue(frm_id);
	}
	else if (policy == LFU) {
                return lfu_enqueue(frm_id);
        }
	else {
		//kprintf("replacement_policy_insert: Unknown policy");
		return SYSERR;
	}

}

SYSCALL replacement_policy_remove(int frm_id) {
	int policy = grpolicy();
	if (policy == SC) {
		return sc_dequeue(frm_id);
	}
	else if (policy == LFU) {
                return lfu_dequeue(frm_id);
        }
        else {
                //kprintf("replacement_policy_remove: Unknown policy");
                return SYSERR;
        }
}

SYSCALL lfu_enqueue(int i){
	STATWORD ps;
        disable(ps);

	frm_tab[i].fr_refcnt++;
        if (sc_tab[i] != NONE) {
                //kprintf("\nlfu_enqueue: Frame %d is already occupied", i);
		restore(ps);
                return SYSERR;
        }
        if (sc_head == NONE) {
                //kprintf("\nlfu_enqueue: Frame %d added. It is the only frame.", i);
                sc_head = sc_tail = i;
                sc_tab[sc_tail] = NONE;
		restore(ps);
                return OK;
        }
        else {
                //kprintf("\nlfu_enqueue: Frame %d added after %d", i, sc_tail);
                sc_tab[sc_tail] = i;
                sc_tail = i;
                sc_tab[i] = NONE;
		restore(ps);
                return OK;
        }

	restore(ps);
	return SYSERR;
}

SYSCALL lfu_dequeue(int i){
        STATWORD ps;
        disable(ps);

        if (sc_head == -1)
        {
                //kprintf("\nlfu_dequeue: No frame available to dequeue");
	        restore(ps);
                return SYSERR;
        }
        else {
                if (i == NONE) {
                        if (sc_tail == sc_head) {
                                int x = sc_head;
                                sc_tab[x] = NONE;
                                //kprintf("\nlfu_dequeue: Frame %d dequeued. No more frames to dequeue.", sc_head);
                                sc_head = sc_tail = NONE;
				frm_tab[x].fr_refcnt = 0;
			        restore(ps);
                                return x;
                        }
                        else {
                                int x = sc_head;
				int y = NONE;
				int p, q;
				for (p = NONE, q = sc_head; p != sc_tail; p = q, q = sc_tab[q]) {
					if ((frm_tab[q].fr_refcnt < frm_tab[x].fr_refcnt) ||
				           ((frm_tab[q].fr_refcnt == frm_tab[x].fr_refcnt) && (frm_tab[q].fr_vpno >= frm_tab[x].fr_vpno))) {
                                                x = q;
						y = p;
					}
				}
				p = y;
				q = x;

				if ((sc_head == q) && (sc_tail == q)) { // removing the only element
					sc_head = sc_tail = NONE;
					//kprintf("\nlfu_dequeue: Frame %d dequeued. No more elements", q);
				}
				else if ( p == NONE ) { // removing first element in the queue
					sc_head = sc_tab[sc_head];
					//kprintf("\nlfu_dequeue: Frame %d dequeued from the start.", q);
				}
				else if (q == sc_tail) { // removing the last element
					sc_tail = p;
					sc_tab[sc_tail] = NONE;
					//kprintf("\nlfu_dequeue: Frame %d dequeued from the end.", q);
				}
				else { // removing the element from the middle
					sc_tab[p] = sc_tab[q];
					//kprintf("\nlfu_dequeue: Frame %d dequeued from the middle.", q);
				}
				frm_tab[q].fr_refcnt = 0;
				sc_tab[q] = NONE;
			        restore(ps);
				return q;
                        }
                }
                else {
                        int p,q;
                        for (p = NONE, q = sc_head; p != sc_tail; p = q, q = sc_tab[q]) {
                                if (q == i) {
                                        if ((sc_head == q) && (sc_tail == q)) { // removing the only element
                                                sc_head = sc_tail = NONE;
                                                //kprintf("\nlfu_dequeue: Frame %d dequeued. No more elements", q);
                                        }
                                        else if ( p == NONE ) { // removing first element in the queue
                                                sc_head = sc_tab[sc_head];
                                                //kprintf("\nlfu_dequeue: Frame %d dequeued from the start.", q);
                                        }
                                        else if (q == sc_tail) { // removing the last element
                                                sc_tail = p;
                                                sc_tab[sc_tail] = NONE;
                                                //kprintf("\nlfu_dequeue: Frame %d dequeued from the end.", q);
                                        }
                                        else { // removing the element from the middle
                                                sc_tab[p] = sc_tab[q];
                                                //kprintf("\nlfu_dequeue: Frame %d dequeued from the middle.", q);
                                        }
					frm_tab[q].fr_refcnt = 0;
                                        sc_tab[q] = NONE;
				        restore(ps);
                                        return q;
                                }
                        }
                        //kprintf("\nlfu_dequeue: Frame %d not found", q);
		        restore(ps);
                        return SYSERR;
                }
        }
}

SYSCALL sc_enqueue(int i) {
	STATWORD ps;
        disable(ps);

	if (sc_tab[i] != NONE) {
		//kprintf("\nsc_enqueue: Frame %d is already occupied", i);
	        restore(ps);
		return SYSERR;
	}
	if (sc_head == NONE) {
		//kprintf("\nsc_enqueue: Frame %d added. It is the only frame.", i);
		sc_head = sc_tail = i;
		sc_tab[sc_tail] = NONE;
	        restore(ps);
		return OK;
	}
	else {
		//kprintf("\nsc_enqueue: Frame %d added after %d", i, sc_tail);
		sc_tab[sc_tail] = i;
		sc_tail = i;
		sc_tab[i] = NONE;
	        restore(ps);
		return OK;
	}
}

SYSCALL sc_dequeue(int i) {
	STATWORD ps;
        disable(ps);
	
	if (sc_head == -1)
	{
		//kprintf("\nsc_dequeue: No frame available to dequeue");
		restore(ps);
		return SYSERR;
	}
	else {
		if (i == NONE) {
			if (sc_tail == sc_head) {	
				int x = sc_head;
				sc_tab[x] = NONE;
				//kprintf("\nsc_dequeue: Frame %d dequeued. No more frames to dequeue.", sc_head);
				sc_head = sc_tail = NONE;
				restore(ps);
				return x;
			}
			else {
				int x = sc_head;	
				sc_head = sc_tab[sc_head];
				sc_tab[x] = NONE;
				//kprintf("\nsc_dequeue: Frame %d dequeued.", x);
				restore(ps);
				return x;
			}
		}
		else {
			int p,q;
			for (p = NONE, q = sc_head; p != sc_tail; p = q, q = sc_tab[q]) {
				if (q == i) {
					if ((sc_head == q) && (sc_tail == q)) { // removing the only element
						sc_head = sc_tail = NONE;
						//kprintf("\nsc_dequeue: Frame %d dequeued. No more elements", q);
					}
					else if ( p == NONE ) { // removing first element in the queue
						sc_head = sc_tab[sc_head];
						//kprintf("\nsc_dequeue: Frame %d dequeued from the start.", q);
					}
					else if (q == sc_tail) { // removing the last element
						sc_tail = p;
						sc_tab[p] = NONE;
						//kprintf("\nsc_dequeue: Frame %d dequeued from the end.", q);
					}
					else { // removing the element from the middle
						sc_tab[p] = sc_tab[q];
						//kprintf("\nsc_dequeue: Frame %d dequeued from the middle.", q);
					}
					sc_tab[q] = NONE;
					restore(ps);
					return q;
				}
			}
			//kprintf("\nsc_dequeue: Frame %d not found", q);
			restore(ps);
			return SYSERR;
		}
	}
}
