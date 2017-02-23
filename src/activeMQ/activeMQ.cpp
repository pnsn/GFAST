#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include "gfast_activeMQ.h"
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#pragma clang diagnostic ignored "-Wc++11-long-long"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wunused-exception-parameter"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wextra-semi"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Weverything"
#endif
#include <activemq/library/ActiveMQCPP.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

using namespace std;

class SAActiveMQ
{
    public:

        SAActiveMQ(void)
        {
            initialize();
            activeMQ_setInit(); 
        }

        ~SAActiveMQ(void)
        {
            finalize();
            activeMQ_setUninit();
        }

        void initialize(void)
        {
            activemq::library::ActiveMQCPP::initializeLibrary();
            activeMQ_setInit();
            return;
        }

        bool isInitialized()
        {
            return activeMQ_isInit();
        }
 
        void finalize(void)
        {
            activemq::library::ActiveMQCPP::shutdownLibrary();
            activeMQ_setUninit();
            return;
        }
};

static SAActiveMQ saAMQ;

extern "C" void activeMQ_initialize(void)
{
    activemq::library::ActiveMQCPP::initializeLibrary();
/*
    if (!activeMQ_isInit())
    {
        printf("activeMQ_initialize: Library already initialized\n");
        return;
    }
    activemq::library::ActiveMQCPP::initializeLibrary();
    return;
*/
}

extern "C" bool activeMQ_isInitialized(void)
{
    return activeMQ_isInit();
}

extern "C" void activeMQ_finalize(void)
{
    //activemq::library::ActiveMQCPP::shutdownLibrary();
/*
    if (!activeMQ_isInit()) // TODO this is wrong - but program hangs otherwise
    {
        activemq::library::ActiveMQCPP::shutdownLibrary();
        activeMQ_setUninit();
    }
    else
    {
        printf("activeMQ_finalize: Library already finalized\n");
        return;
    }
*/
    return;
}

