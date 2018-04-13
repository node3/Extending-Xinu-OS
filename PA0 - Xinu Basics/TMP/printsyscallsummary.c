#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

extern int currpid;
int allpids[NPROC];

void validate_and_print(char* syscall, int total_time, int total_count){
	int avg_time;
	if(total_count == 0)
		return;
	else
		avg_time = total_time/total_count;
		kprintf("\n\tSyscall: sys_%s, count: %d, average execution time: %d (ms)", syscall, avg_time, total_count);
}

void syscallsummary_start(){
	int i=0;
	for(;i<NPROC;i++){
		int j = 0;
		for(;j<27;j++){
			proctab[i].systime[j] = 0;
			proctab[i].syscount[j] = 0;
		}
	}
}

void syscallsummary_stop(){
}

void printsyscallsummary(){
	kprintf("\n\nvoid printsyscallsummary()");
	int i;
	for(i=0;i<NPROC;i++){
		if(allpids[i]==0){
			continue;
		}
		
		kprintf("\nProcess [pid:%d]", i);
		validate_and_print("freemem", proctab[i].systime[FREEMEM], proctab[i].syscount[FREEMEM]);
		validate_and_print("chprio", proctab[i].systime[CHPRIO], proctab[i].syscount[CHPRIO]);
		validate_and_print("getpid", proctab[i].systime[GETPID], proctab[i].syscount[GETPID]);
		validate_and_print("getprio", proctab[i].systime[GETPRIO], proctab[i].syscount[GETPRIO]);
		validate_and_print("gettime", proctab[i].systime[GETTIME], proctab[i].syscount[GETTIME]);
		validate_and_print("kill", proctab[i].systime[KILL], proctab[i].syscount[KILL]);
		validate_and_print("receive", proctab[i].systime[KILL], proctab[i].syscount[KILL]);
		validate_and_print("recvclr", proctab[i].systime[RECVCLR], proctab[i].syscount[RECVCLR]);
		validate_and_print("recvtim", proctab[i].systime[RECVTIM], proctab[i].syscount[RECVTIM]);
		validate_and_print("resume", proctab[i].systime[RESUME], proctab[i].syscount[RESUME]);
		validate_and_print("scount", proctab[i].systime[SCOUNT], proctab[i].syscount[SCOUNT]);
		validate_and_print("sdelete", proctab[i].systime[SDELETE], proctab[i].syscount[SDELETE]);
		validate_and_print("send", proctab[i].systime[SEND], proctab[i].syscount[SEND]);
		validate_and_print("setdev", proctab[i].systime[SETDEV], proctab[i].syscount[SETDEV]);
		validate_and_print("setnok", proctab[i].systime[SETNOK], proctab[i].syscount[SETNOK]);
		validate_and_print("screate", proctab[i].systime[SCREATE], proctab[i].syscount[SCREATE]);
		validate_and_print("signal", proctab[i].systime[SIGNAL], proctab[i].syscount[SIGNAL]);
		validate_and_print("signaln", proctab[i].systime[SIGNALN], proctab[i].syscount[SIGNALN]);
		validate_and_print("sleep", proctab[i].systime[SLEEP], proctab[i].syscount[SLEEP]);
		validate_and_print("sleep10", proctab[i].systime[SLEEP10], proctab[i].syscount[SLEEP10]);
		validate_and_print("sleep100", proctab[i].systime[SLEEP100], proctab[i].syscount[SLEEP100]);
		validate_and_print("sleep1000", proctab[i].systime[SLEEP1000], proctab[i].syscount[SLEEP1000]);
		validate_and_print("sreset", proctab[i].systime[SRESET], proctab[i].syscount[SRESET]);
		validate_and_print("stacktrace", proctab[i].systime[STACKTRACE], proctab[i].syscount[STACKTRACE]);
		validate_and_print("suspend", proctab[i].systime[SUSPEND], proctab[i].syscount[SUSPEND]);
		validate_and_print("unsleep", proctab[i].systime[UNSLEEP], proctab[i].syscount[UNSLEEP]);
		validate_and_print("wait", proctab[i].systime[WAIT], proctab[i].syscount[WAIT]);
	}
}
