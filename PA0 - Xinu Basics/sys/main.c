/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

int prX;
void halt();

/*------------------------------------------------------------------------
 *  *  *  main  --  user main program
 *   *   *------------------------------------------------------------------------
 *    *    */
prch(c)
char c;
{
	int i;
	sleep(5);	
}
int main()
{
	kprintf("0x%08x",zfunction(0xaabbccdd));
	printsegaddress();
	printtos();
	printprocstks(-1);
	syscallsummary_start();
	resume(prX = create(prch,2000,20,"proc X",1,'A'));
	sleep(10);
	syscallsummary_stop();
	printsyscallsummary();
	return 0;
}

