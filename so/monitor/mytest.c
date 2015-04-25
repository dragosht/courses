#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <pthread.h>

#include "monitor.h"


/* Stub these up */
void IncEnter();
void DecEnter();
void IncSignal();
void DecSignal();
void IncWait();
void DecWait();
void IncCond(int nrCond);
void DecCond(int nrCond);



#define NUM_THREADS 5

Monitor *m;

void* thra(void* params)
{
    Enter(m);
    //printf("first: %d\n", Enter(m));
    sleep(1);
    Wait(m, 0);
    sleep(1);
    Leave(m);

    //printf("last: %d\n", Leave(m));

    return NULL;
}


void* thrb(void* params)
{
    sleep(1);
    Enter(m);
    sleep(1);
    Signal(m, 0);
    sleep(2);
    Leave(m);

    return NULL;
}


int main(int argc, char *argv[])
{
    pthread_t a, b;

    m = Create(1, SIGNAL_AND_WAIT);

    pthread_create(&a, NULL, &thra, NULL);
    pthread_create(&b, NULL, &thrb, NULL);

    pthread_join(a, NULL);
    pthread_join(b, NULL);

    return 0;
}

void IncEnter()
{
    /* That's probably pretty lame */
    printf("Thread %d: IncEnter()\n", (int)pthread_self());
}

void DecEnter()
{
    printf("Thread %d: DecEnter()\n", (int)pthread_self());
}

void IncSignal()
{
    printf("Thread %d: IncSignal()\n", (int)pthread_self());
}

void DecSignal()
{
    printf("Thread %d: DecSignal()\n", (int)pthread_self());
}

void IncWait()
{
    printf("Thread %d: IncWait()\n", (int)pthread_self());
}

void DecWait()
{
    printf("Thread %d: DecWait()\n", (int)pthread_self());
}

void IncCond(int nrCond)
{
    printf("Thread %d: IncCond(%d)\n", (int)pthread_self(), nrCond);
}

void DecCond(int nrCond)
{
    printf("Thread %d: DecCond(%d)\n", (int)pthread_self(), nrCond);
}


