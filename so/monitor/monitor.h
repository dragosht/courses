#ifndef MONITOR_H__
#define MONITOR_H__

#include "Constante.h"

typedef void Monitor;

Monitor* Create(int conditions, char policy);

int Destroy(Monitor *m);

int Enter(Monitor *m);

int Leave(Monitor *m);

int Wait(Monitor *m, int cond);

int Signal(Monitor *m, int cond);

int Broadcast(Monitor *m, int cond);

#endif

