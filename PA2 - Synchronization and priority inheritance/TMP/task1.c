#include <conf.h>                                                                                                                                                                            
#include <kernel.h>                                                                                                                                                                          
#include <proc.h>                                                                                                                                                                            
#include <sem.h>                                                                                                                                                                             
#include <lock.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

void reader(char *msg, int lck, int priority)
{
    int a;

    printf("  %s: to acquire lock\n", msg);
    a = lock(lck, READ, priority);
    if (a == SYSERR) {
        printf("  %s: LOCK THREW SYSERR\n", msg);
        return;
    }
    if (a == L_DELETED) {
        printf ("  %s: LOCK IS DELETED\n", msg);
        return;
    }
    printf("  %s: acquired lock, sleep 2s\n", msg);
    sleep(2);
    printf("  %s: to release lock\n", msg);
    releaseall(1, lck);
}

void writer(char *msg, int lck, int priority)
{
    int a;
    printf("  %s: to acquire lock\n", msg);
    a = lock(lck, WRITE, priority);
    if (a == SYSERR) {
        printf("  %s: lock returned SYSERR\n", msg);
        return;
    }
    if (a == L_DELETED) {
        printf("  %s: lock was DELETED\n", msg);
        return;
    }
    printf("  %s: acquired lock, sleep 3s\n", msg);
    sleep(3);
    printf("  %s: to release lock\n", msg);
    releaseall(1, lck);
}

void fsem(char *msg, int sem) 
{
    int a;
    printf("  %s: to acquire sem\n", msg);
    a = wait(sem);
    if (a == SYSERR) {
        printf("  %s: XINU semaphore returned SYSERR\n", msg);
        return;
    }
    else {
        printf("  %s: acquired sem, sleep 2s\n", msg);
        sleep(2);
        printf("  %s: to release sem\n", msg);
        signal(sem);
    }
}

int main()
{
    int lw1, lw2, lr1; 
    int sw1, sw2, sr1; 
    int lck, sem;

    printf("Priority inversion solution using reader/writer lock\n");

    lck  = lcreate();
    lw1 = create(writer, 2000, 20, "writer", 3, "writer 1", lck, 10);
    lr1 = create(reader, 2000, 40, "reader", 3, "reader 1", lck, 20);
    lw2 = create(writer, 2000, 10, "writer", 3, "writer 2", lck, 30);

    printf("Priority before resuming Writer 1 : (writer 1: %d), (reader 1: %d)\n", getprio(lw1), getprio(lr1));
    printf("Writer 1 starts, then sleeps 1 second\n");
    resume(lw1);
    sleep(1);

    printf("Priority before resuming Reader 1 : (writer 1: %d), (reader 1: %d)\n", getprio(lw1), getprio(lr1));
    printf("Reader 1 starts, then sleeps 1 second\n");
    resume(lr1);
    sleep(1);

    printf("Priority before resuming Writer 2 : (writer 1: %d), (reader 1: %d)\n", getprio(lw1), getprio(lr1));
    printf("Writer 2 starts, then sleeps 1 second\n");
    resume(lw2);
    sleep(1);

    sleep(10);
    printf ("----------------------------------------------------\n");

    printf("Priority inversion problm in XINU semaphore\n");

    sem  = screate(1); // Counting semaphore as mutex lock
    sw1 = create(fsem, 2000, 20, "writer", 3, "writer 3", sem, 10);
    sr1 = create(fsem, 2000, 40, "reader", 3, "reader 2", sem, 20);
    sw2 = create(fsem, 2000, 10, "writer", 3, "writer 4", sem, 30);

    printf("Priority before resuming Writer 3 : (writer 3: %d), (reader 2: %d)\n", getprio(sw1), getprio(sr1));
    printf("Writer 3 starts, then sleeps 1 second\n");
    resume(sw1);
    sleep(1);

    printf("Priority before resuming Reader 2 : (writer 3: %d), (reader 2: %d)\n", getprio(sw1), getprio(sr1));
    printf("Reader 2 starts, then sleeps 1 second\n");
    resume(sr1);
    sleep(1);

    printf("Priority before resuming Writer 4 : (writer 3: %d), (reader 2: %d)\n", getprio(sw1), getprio(sr1));
    printf("Writer 4 starts, then sleeps 1 second\n");
    resume(sw2);
    sleep(1);

    shutdown();
    return 0;
}