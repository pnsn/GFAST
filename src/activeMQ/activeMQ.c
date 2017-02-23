#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "gfast_activeMQ.h"

static bool isinit = false;

void activeMQ_start(void)
{
    if (activeMQ_isInit())
    {
        printf("activeMQ_start: Library already initialized\n");
        return;
    }
    activeMQ_initialize();
    activeMQ_setInit();
}

void activeMQ_stop(void)
{
    if (!activeMQ_isInit())
    {
        printf("activeMQ_stop: Library already shut down\n");
        return;
    }
    activeMQ_finalize();
    activeMQ_setUninit();
    return;
}

void activeMQ_setInit(void)
{
    isinit = true;
    return;
} 

void activeMQ_setUninit(void)
{
    isinit = false;
    return;
}

bool activeMQ_isInit(void)
{
    return isinit;
}
