/**
 * @file CoreActiveMqUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the core/activeMQ directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"

#include "gfast.h"

TEST(CoreActiveMq, testSetTcpURIRequest) {
    char *brokerURI;
    int msReconnect, maxAttempts;
    const char *destinationURL = "tcp://localhost:62616\0";
    msReconnect = 500;
    maxAttempts = 10;
    
    brokerURI = activeMQ_setTcpURIRequest(destinationURL,
        msReconnect, maxAttempts);

    EXPECT_STREQ(
        "failover:(tcp://localhost:62616)?initialReconnectDelay=500?startupMaxReconnectAttempts=10",
        brokerURI);
}
