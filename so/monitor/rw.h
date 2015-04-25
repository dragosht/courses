#ifndef RW_H__
#define RW_H__

#include "monitor.h"

Monitor* CreateRWMonitor();
int GetNrConds();
void StartCit(Monitor* m);
void StopCit(Monitor* m);
void StartScrit(Monitor* m);
void StopScrit(Monitor* m);

#endif

