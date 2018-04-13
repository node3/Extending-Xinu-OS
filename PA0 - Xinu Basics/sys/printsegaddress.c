#include <stdio.h>
extern int etext, edata, end;
void printsegaddress(){
    kprintf("\n\nvoid printsegaddress()");
    kprintf("\nCurrent: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x", &etext, etext, &edata, edata, &end, end);
    kprintf("\nPreceding: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x", &etext-1, *(&etext-1), &edata-1, *(&edata-1), &end-1, *(&end-1));
    kprintf("\nAfter: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x", &etext+1, *(&etext+1), &edata+1, *(&edata+1), &end+1, *(&end+1));
}
