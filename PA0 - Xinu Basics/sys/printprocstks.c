#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

static long esp;
void printprocstks(int priority){
	kprintf("\n\nvoid printprocstks(int priority)");
	int i;
	for(i =0;i<NPROC; i++){
		if(proctab[i].pstate != PRFREE){
			if(proctab[i].pprio > priority ){
				kprintf("\nProcess [%s]", proctab[i].pname);
				kprintf("\n\tpid : %d", i);
				kprintf("\n\tpriority : %d", proctab[i].pprio);
				kprintf("\n\tbase : 0x%08x", proctab[i].pbase);
				kprintf("\n\tlimit : 0x%08x", proctab[i].plimit);
				kprintf("\n\tlen : %d", proctab[i].pstklen);
				if(proctab[i].pstate == PRCURR){
					asm("movl %esp,esp"); 
				}
				else
					esp = proctab[i].pesp;
				kprintf("\n\tpointer : 0x%08x", esp);
			}	
		}
	}
}	
