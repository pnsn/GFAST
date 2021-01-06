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

/*! @brief c wrapper on activeMQCPP library initialization */
extern "C" void activeMQ_initialize(void)
{
  activemq::library::ActiveMQCPP::initializeLibrary();
  return;
}

/*! @brief c wrapper on activeMQCPP library shutdown */
extern "C" void activeMQ_finalize(void)
{
  activemq::library::ActiveMQCPP::shutdownLibrary();
  return;
}

