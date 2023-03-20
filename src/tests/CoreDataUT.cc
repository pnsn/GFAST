/**
 * @file CoreDataUT.cc
 * @author Carl Ulberg, University of Washington (ulbergc@uw.edu)
 * @brief This tests files in the core/data directory.
 */

#include "gtest/gtest.h"
#include "ut_log_init.h"
#include "ut_main.h"

#include "gfast.h"

// test finalize, initialize, readMetaDataFile, readSiteMaskFile

TEST(CoreData, testReadMetaDataFileBasic) {
    const char *metaDataFile;
    char **metaDataNetworks;
    int n_networks, ierr;
    struct GFAST_data_struct gps_data;

    // initialize
    metaDataFile = "data/merged_chanfile_coord.dat\0";
    metaDataNetworks = NULL;
    n_networks = 0;
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));

    ierr = core_data_readMetaDataFile(metaDataFile,
                                      metaDataNetworks,
                                      n_networks,
                                      &gps_data);
    EXPECT_EQ(0, ierr) << "Error reading sites file";
    EXPECT_EQ(492, gps_data.stream_length);

    // Gains should be equal on the channels.
    // This also indirectly verifies that the data array is filled.
    for (int i = 0; i < gps_data.stream_length; i++) {
        EXPECT_EQ(gps_data.data[i].gain[0], gps_data.data[i].gain[1]);
        EXPECT_EQ(gps_data.data[i].gain[0], gps_data.data[i].gain[2]);
    }

    core_data_finalize(&gps_data);
}

TEST(CoreData, testReadMetaDataFileWithMetaDataNetworks) {
    const char *metaDataFile;
    char **metaDataNetworks;
    int n_networks, ierr;
    struct GFAST_data_struct gps_data;

    // initialize
    metaDataFile = "data/merged_chanfile_coord.dat\0";
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));

    // metaDataNetworks = NULL;
    n_networks = 1;
    metaDataNetworks = (char **)malloc(sizeof(char *) * 1); 
    metaDataNetworks[0] = (char *)malloc(sizeof(char) * 3);
    strcpy(metaDataNetworks[0], "CI\0");

    ierr = core_data_readMetaDataFile(metaDataFile,
                                      metaDataNetworks,
                                      n_networks,
                                      &gps_data);
    EXPECT_EQ(0, ierr) << "Error reading sites file";
    EXPECT_EQ(461, gps_data.stream_length);

    // Gains should be equal on the channels.
    // This also indirectly verifies that the data array is filled.
    for (int i = 0; i < gps_data.stream_length; i++) {
        EXPECT_EQ(gps_data.data[i].gain[0], gps_data.data[i].gain[1]);
        EXPECT_EQ(gps_data.data[i].gain[0], gps_data.data[i].gain[2]);
    }

    core_data_finalize(&gps_data);
    free(metaDataNetworks[0]);
    free(metaDataNetworks);
}

TEST(CoreData, testInitialize) {
    char propfilename[1024];
    int ierr, i;
    struct GFAST_props_struct props;
    struct GFAST_data_struct gps_data;
    const enum opmode_type opmode = REAL_TIME_EEW;

    // initialize
    strncpy(propfilename,
            "data/gfast.props",
            1024-1);
    ierr = 0;
    memset(&props, 0, sizeof(struct GFAST_props_struct));
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));

    // First read the properties
    ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);
    EXPECT_EQ(0, ierr);

    // Now actually test the function in question
    ierr = core_data_initialize(props, &gps_data);
    EXPECT_EQ(0, ierr);

    for ( i = 0; i < gps_data.stream_length; i++ ) {
        EXPECT_GT(gps_data.data[i].maxpts, 0);
        EXPECT_NE(gps_data.data[i].ubuff, nullptr);
        EXPECT_NE(gps_data.data[i].nbuff, nullptr);
        EXPECT_NE(gps_data.data[i].ebuff, nullptr);
        EXPECT_NE(gps_data.data[i].tbuff, nullptr);
    }

    // finalize
    GFAST_core_data_finalize(&gps_data);
    GFAST_core_properties_finalize(&props);
}

