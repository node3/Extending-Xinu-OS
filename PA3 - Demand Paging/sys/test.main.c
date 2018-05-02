/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

void halt();

#define TPASSED    1
#define TFAILED    0

#define MYVADDR1   0x40000000
#define MYVPNO1    0x40000
#define MYVADDR2   0x80000000
#define MYVPNO2    0x80000
#define MYBS1      1
#define MAX_BSTORE 16

#ifndef NBPG
#define NBPG       4096
#endif

#define assert(x,error) if(!(x)){ \
      kprintf(error);\
      return;\
      }
      
void test1()
{
  kprintf("\nTest 1: Testing xmmap.\n");
  char *addr1 = (char*)0x40000000; 
  int i = ((unsigned long)addr1) >> 12; 
  
  get_bs(MYBS1, 100);
  if (xmmap(i, MYBS1, 100) == SYSERR) {
    kprintf("xmmap call failed\n");
    kprintf("\tFAILED!\n");
    return;
  }

  for (i = 0; i < 26; i++) {
    *addr1 = 'A'+i;   //trigger page fault for every iteration
    addr1 += NBPG;    //increment by one page each time
  }

  addr1 = (char*)0x40000000;
  for (i = 0; i < 26; i++) {
    if (*addr1 != 'A'+i) {
      kprintf("\tFAILED!\n");
      return;
    }
    addr1 += NBPG;    //increment by one page each time
  }

  xmunmap(0x40000000>>12);
  release_bs(MYBS1);
  kprintf("\tPASSED!\n");
  return;
}
/*----------------------------------------------------------------*/
void proc_test2(int i,int j,int* ret,int s) {
  char *addr;
  int bsize;
  int r;
  bsize = get_bs(i, j);
  if (bsize != 50)
    *ret = TFAILED;
  r = xmmap(MYVPNO1, i, j);
  kprintf("\nValue of s is %d",s);
  if (j<=50 && r == SYSERR){
    *ret = TFAILED;
  }
  if (j> 50 && r != SYSERR){
    *ret = TFAILED;
  }
  sleep(s);
  if (r != SYSERR)
    xmunmap(MYVPNO1);
  release_bs(i);
  return;
}
void test2() {
  int pids[16];
  int mypid;
  int i,j;

  int ret = TPASSED;
  kprintf("\nTest 2: Testing backing store operations\n");

  int bsize = get_bs(1, 100);
  if (bsize != 100)
    ret = TFAILED;
  release_bs(1);
  bsize = get_bs(1, 130);
  if (bsize != SYSERR)
    ret = TFAILED;
  bsize = get_bs(1, 0);
  if (bsize != SYSERR)
    ret = TFAILED;

  mypid = create(proc_test2, 2000, 20, "proc_test2", 4, 1,
                 50, &ret, 4);
  resume(mypid);
  sleep(2);
  for(i=1;i<=5;i++){
    pids[i] = create(proc_test2, 2000, 20, "proc_test2", 4, 1,
                     i*20, &ret, 0);
    resume(pids[i]);
  }
  sleep(3);
  kill(mypid);
  for(i=1;i<=5;i++){
    kill(pids[i]);
  }
  if (ret != TPASSED)
    kprintf("\tFAILED!\n");
  else
    kprintf("\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/
void proc1_test3(int i,int* ret) {
  char *addr;
  int bsize;
  int r;

  get_bs(i, 100);
  
  if (xmmap(MYVPNO1, i, 100) == SYSERR) {
    *ret = TFAILED;
    return 0;
  }
  sleep(4);
  xmunmap(MYVPNO1);
  release_bs(i);
  return;
}
void proc2_test3() {
  /*do nothing*/
  sleep(1);
  return;
}
void test3() {
  int pids[16];
  int mypid;
  int i,j;

  int ret = TPASSED;
  kprintf("\nTest 3: Private heap is exclusive\n");

  for(i=0;i<=15;i++){
    pids[i] = create(proc1_test3, 2000, 20, "proc1_test3", 2, i,&ret);
    if (pids[i] == SYSERR){
      ret = TFAILED;
    }else{
      resume(pids[i]);
    }
  }
  sleep(1);
  mypid = vcreate(proc2_test3, 2000, 100, 20, "proc2_test3", 0, NULL);
  if (mypid != SYSERR)
    ret = TFAILED;

  for(i=0;i<=15;i++){
    kill(pids[i]);
  }
  if (ret != TPASSED)
    kprintf("\tFAILED!\n");
  else
    kprintf("\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/

void proc1_test4(int* ret) {
  unsigned char *addr;
  int i;

  get_bs(MYBS1, 100);

  if (xmmap(MYVPNO1, MYBS1, 100) == SYSERR) {
    kprintf("xmmap call failed\n");
    *ret = TFAILED;
    sleep(3);
    return;
  }

  addr = (char*) MYVADDR1;
  for (i = 0; i < 26; i++) {
    kprintf("\nProc1: Writing %c", 'A' + i);
    *(addr + i * NBPG) = 'A' + i;
  }
  sleep(6);

  /*Shoud see what proc 2 updated*/
  for (i = 0; i < 26; i++) {
    kprintf("\nProc1: Reading %c", 'a' + i);
    /*expected output is abcde.....*/
    if (*(addr + i * NBPG) != 'a'+i){
      *ret = TFAILED;
    kprintf("\nProc1: Failed. Read %c", *(addr + i * NBPG));
      break;    
    }
  }

  xmunmap(MYVPNO1);
  release_bs(MYBS1);
  return;
}
void proc2_test4(int *ret) {
  unsigned char *addr;
  unsigned int i;

  get_bs(MYBS1, 100);

  if (xmmap(MYVPNO2, MYBS1, 100) == SYSERR) {
    kprintf("xmmap call failed\n");
    *ret = TFAILED;
    sleep(3);
    return;
  }

  addr = (char*) MYVADDR2;

  /*Shoud see what proc 1 updated*/
  for (i = 0; i < 26; i++) {
    kprintf("\nProc2: Reading %c", 'A' + i);
    /*expected output is ABCDEF.....*/
    if (*(addr + i * NBPG) != 'A'+i){
      *ret = TFAILED;
      break;
    }
  }

  /*Update the content, proc1 should see it*/
  for (i = 0; i < 26; i++) {
    *(addr + i * NBPG) = 'a' + i;
    kprintf("\nProc2: Writing %c", 'a' + i);
  }

  xmunmap(MYVPNO2);
  release_bs(MYBS1);
  return;
}
void test4() {
  int pid1;
  int pid2;
  int ret = TPASSED;
  kprintf("\nTest 4: Shared backing store\n");

  pid1 = create(proc1_test4, 2000, 20, "proc1_test4", 1, &ret);
  pid2 = create(proc2_test4, 2000, 20, "proc2_test4", 1, &ret);

  resume(pid1);
  sleep(3);
  resume(pid2);

  sleep(10);
  kill(pid1);
  kill(pid2);
  if (ret != TPASSED)
    kprintf("\tFAILED!\n");
  else
    kprintf("\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/
void proc1_test5(int* ret) {
  int *x;
  int *y;
  int *z;

  //kprintf("ready to allocate heap space\n");
  x = vgetmem(1024);
  if ((x == SYSERR) || (x < 0x1000000)
      || (x > 0x1000000 + 128 * NBPG - 1024)) {
    *ret = TFAILED;
  }
  if (x == SYSERR)
    return;

  *x = 100;
  *(x + 1) = 200;

  if ((*x != 100) || (*(x+1) != 200)) {
    *ret = TFAILED;
  }
  vfreemem(x, 1024);

  x = vgetmem(129*NBPG); //try to acquire a space that is bigger than size of one backing store
  if (x != SYSERR) {
    *ret = TFAILED;
  }

  x = vgetmem(50*NBPG);
  y = vgetmem(50*NBPG);
  z = vgetmem(50*NBPG);
  if ((x == SYSERR) || (y == SYSERR) || (z != SYSERR)){
    *ret = TFAILED;
    if (x != SYSERR) vfreemem(x, 50*NBPG);
    if (y != SYSERR) vfreemem(y, 50*NBPG);
    if (z != SYSERR) vfreemem(z, 50*NBPG);
    return;
  }
  vfreemem(y, 50*NBPG);
  z = vgetmem(50*NBPG);
  if (z == SYSERR){
    *ret = TFAILED;
  }
  if (x != SYSERR) vfreemem(x, 50*NBPG);
  if (z != SYSERR) vfreemem(z, 50*NBPG);
  return;
}

void test5() {
  int pid1;
  int ret = TPASSED;

  kprintf("\nTest 5: vgetmem/vfreemem\n");
  pid1 = vcreate(proc1_test5, 2000, 100, 20, "proc1_test5", 1, &ret);

  //kprintf("pid %d has private heap\n", pid1);
  resume(pid1);
  sleep(3);
  kill(pid1);
  if (ret != TPASSED)
    kprintf("\tFAILED!\n");
  else
    kprintf("\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/
void proc1_test6(int *ret) {

  char *vaddr, *addr0, *addr_lastframe, *addr_last;
  int i, j;
  int tempaddr;
  int addrs[1200];

  int maxpage = (NFRAMES - (5 + 1 + 1 + 1));
  
  int vaddr_beg = 0x40000000;
  int vpno;
  
  for(i = 0; i < MAX_BSTORE; i++){
    tempaddr = vaddr_beg + 100 * NBPG * i; 
    vaddr = (char *) tempaddr;
    vpno = tempaddr >> 12;
    get_bs(i, 100);
    if (xmmap(vpno, i, 100) == SYSERR) {
      *ret = TFAILED;
      kprintf("xmmap call failed\n");
      sleep(3);
      return;
    }

    for (j = 0; j < 100; j++) {
      *(vaddr + j * NBPG) = 'A' + i;
    }
    
    for (j = 0; j < 100; j++) {
      if (*(vaddr + j * NBPG) != 'A'+i){
        *ret = TFAILED;
        break;
      }
    }    
    xmunmap(vpno);
    release_bs(i);    
  }

  return;
}

void test6(){
  int pid1;
  int ret = TPASSED;
  kprintf("\nTest 6: Stress testing\n");

  pid1 = create(proc1_test6, 2000, 50, "proc1_test6",1,&ret);

  resume(pid1);
  sleep(4);
  kill(pid1);
  if (ret != TPASSED)
    kprintf("\tFAILED!\n");
  else
    kprintf("\tPASSED!\n");
}

/*-------------------------------------------------------------------------------------*/
void test_func7()
{
  int PAGE0 = 0x40000;
  int i,j,temp;
  int addrs[1200];
  int cnt = 0; 
  //can go up to  (NFRAMES - 5 frames for null prc - 1pd for main - 1pd + 1pt frames for this proc)
  //frame for pages will be from 1032-2047
  int maxpage = (NFRAMES - (5 + 1 + 1 + 1));
  for (i=0;i<=maxpage/100;i++){
    if(get_bs(i,100) == SYSERR)
    {
      kprintf("get_bs call failed \n");
      return;
    }
    if (xmmap(PAGE0+i*100, i, 100) == SYSERR) {
      kprintf("xmmap call failed\n");
      return;
    }
    for(j=0;j < 100;j++)
    {  
      //store the virtual addresses
      addrs[cnt++] = (PAGE0+(i*100) + j) << 12;
    }     
  }
  /* all of these should generate page fault, no page replacement yet
     acquire all free frames, starting from 1032 to 2047, lower frames are acquired first
     */
  char *zero_addr = (char*) 0x0;
  for(i=0; i < maxpage; i++)
  {  
    *((int *)addrs[i]) = i + 1;
    if (i + 1 != *((int *)(zero_addr + (i + 1032) * NBPG))) {
      kprintf("\tFAILED!\n");
      kprintf("AA 0x%08x: %d\n", (int *)addrs[i], *((int *)addrs[i]));
      kprintf("BB 0x%08x: %d\n", zero_addr + (i + 1032) * NBPG, *((int *)(zero_addr + (i + 1032) * NBPG)));
    }
  }
  
  //trigger page replacement, this should clear all access bits of all pages
  kprintf("\n\t7.1 Expected replaced frame: 1032\n\t");

  *((int *)addrs[maxpage]) = maxpage + 1; 
  temp = *((int *)addrs[maxpage]);
  if (temp != *((int *)(1032 * NBPG))) {
    kprintf("\tFAILED!\n");
    kprintf("AA 0x%08x: %d\n", (int *)addrs[maxpage], *((int *)addrs[maxpage]));
    kprintf("BB 0x%08x: %d\n", 1032 * NBPG, *((int *)(1032 * NBPG)));
  }
  
  
  for(i=1; i <= maxpage; i++)
  {
    if ((i != 600) && (i != 800))
      *((int *)addrs[i])= i+1;
  }
  
  kprintf("\n\t7.2 Expected replaced frame: 1033\n\t");
  *((int *)addrs[maxpage+1]) = maxpage + 2; 
  temp = *((int *)addrs[maxpage+1]);
  if (temp != *((int *)(1033 * NBPG))) {
    kprintf("\tFAILED!\n");
    kprintf("AA 0x%08x: %d\n", (int *)addrs[maxpage], *((int *)addrs[maxpage]));
    kprintf("BB 0x%08x: %d\n", 1033 * NBPG, *((int *)(1033 * NBPG)));
  }
  
  for (i=0;i<=maxpage/100;i++){
      xmunmap(PAGE0+(i*100));
      release_bs(i);    
  }
}
void test7(){
  int pid1;
  int ret = TPASSED;
  kprintf("\nTest 7: Test FIFO page replacement policy\n");
  srpolicy(SC); 
  pid1 = create(test_func7, 2000, 20, "test_func7", 0, NULL);
  resume(pid1);
  sleep(10);
  kill(pid1);
  kprintf("\n\tFinished! Check error and replaced frames\n");
}

/*-------------------------------------------------------------------------------------*/
void test_func8()
{
  int PAGE0 = 0x40000;
  int i,j,temp;
  int addrs[1200];
  int cnt = 0; 
  //can go up to  (NFRAMES - 5 frames for null prc - 1pd for main - 1pd + 1pt frames for this proc)
  //frame for pages will be from 1032-2047
  int maxpage = (NFRAMES - (5 + 1 + 1 + 1));
  for (i=0;i<=maxpage/100;i++){
    if(get_bs(i,100) == SYSERR)
    {
      kprintf("get_bs call failed \n");
      return;
    }
    if (xmmap(PAGE0+i*100, i, 100) == SYSERR) {
      kprintf("xmmap call failed\n");
      return;
    }
    for(j=0;j < 100;j++)
    {  
      //store the virtual addresses
      addrs[cnt++] = (PAGE0+(i*100) + j) << 12;
    }     
  }
  /* all of these should generate page fault, no page replacement yet
     acquire all free frames, starting from 1032 to 2047, lower frames are acquired first
     */
  char *zero_addr = (char*) 0x0;
  for(i=0; i < maxpage-1; i++)
  {  
    *((int *)addrs[i]) = i + 1;
    if (i + 1 != *((int *)(zero_addr + (i + 1032) * NBPG))) {
      kprintf("\tFAILED!\n");
      kprintf("AA 0x%08x: %d\n", (int *)addrs[i], *((int *)addrs[i]));
      kprintf("BB 0x%08x: %d\n", zero_addr + (i + 1032) * NBPG, *((int *)(zero_addr + (i + 1032) * NBPG)));
    }
  }
  
  for(i=0; i < maxpage/2; i++)
  {  
    *((int *)addrs[i]) = i * 2 + 1;
  }
  i = maxpage-1;
  *((int *)addrs[i]) = i + 1;
  
  //trigger page replacement, this should clear all access bits of all pages
  kprintf("\t8.1 Expected replaced frame: %d\n\t",1032+maxpage/2);

  *((int *)addrs[maxpage]) = maxpage * 3 + 1; 
  temp = *((int *)addrs[maxpage]);
  if (temp != *((int *)((1032+maxpage/2) * NBPG))) {
    kprintf("\tFAILED!\n");
    kprintf("AA 0x%08x: %d\n", (int *)addrs[maxpage], *((int *)addrs[maxpage]));
    kprintf("BB 0x%08x: %d\n", (1032+maxpage/2) * NBPG, *((int *)((1032+maxpage/2) * NBPG)));
  }
     
  for (i=0;i<=maxpage/100;i++){
      xmunmap(PAGE0+(i*100));
      release_bs(i);    
  }
}
void test_func8_2()
{
  int PAGE0 = 0x40000;
  int i,j,temp;
  int addrs[1200];
  int cnt = 0; 
  //can go up to  (NFRAMES - 5 frames for null prc - 1pd for main - 1pd + 1pt frames for this proc)
  //frame for pages will be from 1032-2047
  int maxpage = (NFRAMES - (5 + 1 + 1 + 1));
  for (i=0;i<=maxpage/100;i++){
    if(get_bs(i,100) == SYSERR)
    {
      kprintf("get_bs call failed \n");
      return;
    }
    if (xmmap(PAGE0+i*100, i, 100) == SYSERR) {
      kprintf("xmmap call failed\n");
      return;
    }
    for(j=0;j < 100;j++)
    {  
      //store the virtual addresses
      addrs[cnt++] = (PAGE0+(i*100) + j) << 12;
    }     
  }
  /* all of these should generate page fault, no page replacement yet
     acquire all free frames, starting from 1032 to 2047, lower frames are acquired first
     */
  char *zero_addr = (char*) 0x0;
  for(i=0; i < maxpage-1; i++)
  {  
    *((int *)addrs[i]) = i + 1;
    if (i + 1 != *((int *)(zero_addr + (i + 1032) * NBPG))) {
      kprintf("\tFAILED!\n");
      kprintf("AA 0x%08x: %d\n", (int *)addrs[i], *((int *)addrs[i]));
      kprintf("BB 0x%08x: %d\n", zero_addr + (i + 1032) * NBPG, *((int *)(zero_addr + (i + 1032) * NBPG)));
    }
  }
  
  for(i=0; i < maxpage/3; i++)
  {  
    *((int *)addrs[i]) = i * 4 + 1;
  }
  i = maxpage-1;
  *((int *)addrs[i]) = i + 1;
  
  //trigger page replacement, this should clear all access bits of all pages
  kprintf("\t8.2 Expected replaced frame: %d\n\t",1032+maxpage/3);

  *((int *)addrs[maxpage]) = maxpage * 5 + 1; 
  temp = *((int *)addrs[maxpage]);
  if (temp != *((int *)((1032+maxpage/3) * NBPG))) {
    kprintf("\tFAILED!\n");
    kprintf("AA 0x%08x: %d\n", (int *)addrs[maxpage], *((int *)addrs[maxpage]));
    kprintf("BB 0x%08x: %d\n", (1032+maxpage/3) * NBPG, *((int *)((1032+maxpage/3) * NBPG)));
  }
    
  for (i=0;i<=maxpage/100;i++){
      xmunmap(PAGE0+(i*100));
      release_bs(i);    
  }
}

void test8(){
  int pid1;
  int ret = TPASSED;
  kprintf("\nTest 8: Test LRU page replacement policy\n");
  srpolicy(LFU); 
  pid1 = create(test_func8, 2000, 20, "test_func8", 0, NULL);
  resume(pid1);
  sleep(10);
  kill(pid1);
  kprintf("\n\tSecond run (test where killing process is handled correctly):\n");
  pid1 = create(test_func8_2, 2000, 20, "test_func8_2", 0, NULL);
  resume(pid1);
  sleep(10);
  kill(pid1);
  kprintf("\n\tFinished! Check error and replaced frames\n");
}

int main() {
  kprintf("\n\nHello World, Xinu lives\n\n");
 
  test1();
  test2();
  test3();

  test4();
  test5();
  test6();

  //test7();
  //test8();
	shutdown();
  return 0;
}
