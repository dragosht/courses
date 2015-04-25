#include "monitor.h"
#include "rw.h"

Monitor* CreateRWMonitor()
{
    Monitor* mon = Create(2, SIGNAL_AND_CONTINUE);
    return mon;
}

int GetNrConds()
{
    return 2;
}

void StartCit(Monitor* m)
{
    
}

void StopCit(Monitor* m)
{
    
}

void StartScrit(Monitor* m)
{
    
}

void StopScrit(Monitor* m)
{
    
}


