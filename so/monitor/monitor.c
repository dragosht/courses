#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "monitor.h"

/* Make the _tester happy */
#include "CallbackMonitor.h"

struct generic_monitor_s {
    /*
     * TSD key indicating wether the current thread is in the monitor.
     */
    pthread_key_t       gm_inside;

    pthread_cond_t      *gm_conds;
    int                 *gm_conds_count;
    int                 gm_conds_size;

    pthread_mutex_t     gm_lock;

    pthread_cond_t      gm_wait;
    pthread_cond_t      gm_signal;
    pthread_cond_t      gm_entry;

    /*
     * Needed because only one thread is scheduled on leave().
     */

    int                 gm_wait_count;
    int                 gm_signal_count;
    int                 gm_entry_count;

    int                 gm_busy;
    char gm_policy;
};


typedef struct generic_monitor_s generic_monitor_t;


void gm_schedule(generic_monitor_t* gmon)
{
    assert(gmon != NULL);

    /* Check the wait, signal entry queues in this order */
    if (gmon->gm_wait_count > 0) {
        pthread_cond_signal(&gmon->gm_wait);
    }
    else if (gmon->gm_signal_count > 0) {
        pthread_cond_signal(&gmon->gm_signal);
    }
    else if (gmon->gm_entry_count > 0) {
        pthread_cond_signal(&gmon->gm_entry);
    }
}


Monitor* Create(int conditions, char policy)
{
    generic_monitor_t* gmon = NULL;
    int i;


    gmon = malloc(sizeof(generic_monitor_t));
    assert(gmon != NULL);

    pthread_key_create(&gmon->gm_inside, NULL);

    gmon->gm_conds_size = conditions;
    gmon->gm_conds = malloc(conditions * sizeof(pthread_cond_t));
    gmon->gm_conds_count = malloc(conditions * sizeof(int));
    for (i = 0; i < conditions; i++) {
        pthread_cond_init(&gmon->gm_conds[i], NULL);
        gmon->gm_conds_count[i] = 0;
    }

    pthread_mutex_init(&gmon->gm_lock, NULL);

    pthread_cond_init(&gmon->gm_wait, NULL);
    pthread_cond_init(&gmon->gm_signal, NULL);
    pthread_cond_init(&gmon->gm_entry, NULL);

    gmon->gm_wait_count = 0;
    gmon->gm_signal_count = 0;
    gmon->gm_entry_count = 0;

    gmon->gm_busy = FALSE;
    gmon->gm_policy = policy;

    return gmon;
}



int Destroy(Monitor *m)
{
    generic_monitor_t* gmon = m;
    int i;

    assert(gmon != NULL);

    if (gmon->gm_busy) {
        return -1;
    }

    pthread_mutex_unlock(&gmon->gm_lock);

    for (i = 0; i < gmon->gm_conds_size; i++) {
        pthread_cond_destroy(&gmon->gm_conds[i]);
    }
    free(gmon->gm_conds);
    free(gmon->gm_conds_count);

    pthread_mutex_destroy(&gmon->gm_lock);

    pthread_cond_destroy(&gmon->gm_wait);
    pthread_cond_destroy(&gmon->gm_signal);
    pthread_cond_destroy(&gmon->gm_entry);

    return 0;
}


int Enter(Monitor *m)
{
    generic_monitor_t* gmon = m;
    assert(gmon != NULL);

    if (pthread_getspecific(gmon->gm_inside) == GM_MONITOR_IN) {
        return -1;
    }

    pthread_mutex_lock(&gmon->gm_lock);

    pthread_setspecific(gmon->gm_inside, GM_MONITOR_IN);

    IncEnter();
    gmon->gm_entry_count++;

    while (gmon->gm_busy) {
        pthread_cond_wait(&gmon->gm_entry, &gmon->gm_lock);
    }

    DecEnter();
    gmon->gm_entry_count--;
    gmon->gm_busy = TRUE;

    pthread_mutex_unlock(&gmon->gm_lock);

    return 0;
}

int Leave(Monitor *m)
{
    generic_monitor_t* gmon = m;
    assert(gmon != NULL);

    if (pthread_getspecific(gmon->gm_inside) == GM_MONITOR_OUT) {
        return -1;
    }

    pthread_mutex_lock(&gmon->gm_lock);
    pthread_setspecific(gmon->gm_inside, GM_MONITOR_OUT);
    gm_schedule(m);
    gmon->gm_busy = FALSE;
    pthread_mutex_unlock(&gmon->gm_lock);

    return 0;
}

int Wait(Monitor *m, int cond)
{
    generic_monitor_t* gmon = m;
    assert(gmon != NULL);

    if (cond >= gmon->gm_conds_size || cond < 0) {
        return -1;
    }

    pthread_mutex_lock(&gmon->gm_lock);

    gmon->gm_busy = FALSE;
    gm_schedule(gmon);

    IncCond(cond);
    gmon->gm_conds_count[cond]++;
    pthread_cond_wait(&gmon->gm_conds[cond], &gmon->gm_lock);
    gmon->gm_conds_count[cond]--;
    DecCond(cond);

    gmon->gm_wait_count++;
    IncWait();

    while (gmon->gm_busy) {
        pthread_cond_wait(&gmon->gm_wait, &gmon->gm_lock);
    }

    gmon->gm_busy = TRUE;
    gmon->gm_wait_count--;
    DecWait();

    pthread_mutex_unlock(&gmon->gm_lock);

    return 0;
}

int Signal(Monitor *m, int cond)
{
    generic_monitor_t* gmon = m;
    assert(gmon != NULL);

    if (cond >= gmon->gm_conds_size || cond < 0) {
        return -1;
    }

    pthread_mutex_lock(&gmon->gm_lock);
        /* Check that there's at least one thread wainting for signal */
    if (gmon->gm_conds_count[cond] > 0) {
        pthread_cond_signal(&gmon->gm_conds[cond]);

        if (gmon->gm_policy == SIGNAL_AND_WAIT) {
            gmon->gm_busy = FALSE;
            gmon->gm_signal_count++;
            IncSignal();

            pthread_cond_wait(&gmon->gm_signal, &gmon->gm_lock);

            gmon->gm_busy = TRUE;
            gmon->gm_signal_count--;
            DecSignal();
        }
    }

    pthread_mutex_unlock(&gmon->gm_lock);

    return 0;
}

int Broadcast(Monitor *m, int cond)
{
    generic_monitor_t* gmon = m;
    assert(gmon != NULL);

    if (cond >= gmon->gm_conds_size || cond < 0) {
        return -1;
    }

    pthread_mutex_lock(&gmon->gm_lock);

    if (gmon->gm_conds_count[cond] > 0) {
        pthread_cond_broadcast(&gmon->gm_conds[cond]);

        if (gmon->gm_policy == SIGNAL_AND_WAIT)
        {
            gmon->gm_busy = FALSE;
            gmon->gm_signal_count++;
            IncSignal();

            pthread_cond_wait(&gmon->gm_signal, &gmon->gm_lock);

            gmon->gm_busy = TRUE;
            gmon->gm_signal_count--;
            DecSignal();
        }
    }

    pthread_mutex_unlock(&gmon->gm_lock);

    return 0;
}

