#include <stdio.h>

static unsigned long *esp, *ebp;

void printtos(){
    kprintf("\n\nvoid printtos()");
    asm("movl %esp,esp");
    asm("movl %ebp,ebp");
    kprintf("\nBefore [0x%08x]: 0x%08x", ebp+2, *(ebp+2));
    kprintf("\nAfter [0x%08x]: 0x%08x", ebp, *ebp);
    kprintf("\nAddress of ESP : 0x%08x", esp);
    int i;
    int j = 123;
    for(i = 1;i<5;i++){
        if(esp <= ebp){
            kprintf("\n\telement[%08x]: 0x%08x", esp+i, *(esp+i));
        }
        else{
            break;
        }
    }
}
