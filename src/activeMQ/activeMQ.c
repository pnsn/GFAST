#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "gfast_activeMQ.h"

/*! static variable to keep track of whether activeMQCPP library has been initialized */
static bool isinit = false;

/*! @brief Initialize activeMQCPP library */
void activeMQ_start(void)
{
    if (isinit)
    {
        printf("activeMQ_start: Library already initialized\n");
        return;
    }
    activeMQ_initialize();
    isinit = true;
}

/*! @brief Shut down activeMQCPP library */
void activeMQ_stop(void)
{
    if (!isinit)
    {
        printf("activeMQ_stop: Library already shut down\n");
        return;
    }
    activeMQ_finalize();
    isinit = false;
    return;
}

/*! @brief function to access activeMQCPP library state */
bool activeMQ_isInit(void)
{
    return isinit;
}
