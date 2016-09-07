#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gfast.h"
#include "iscl/log/log.h"
#include "iscl/iscl/iscl.h"
#include "iscl/time/time.h"

int main()
{
    const char *fcnm = "gfast_playback\0";
    char propfilename[] = "gfast.props\0";
    struct GFAST_activeEvents_struct events;
    struct GFAST_cmtResults_struct cmt;
    struct GFAST_data_struct gps_data;
    struct GFAST_ffResults_struct ff; 
    struct GFAST_offsetData_struct cmt_data, ff_data;
    struct GFAST_peakDisplacementData_struct pgd_data;
    struct GFAST_pgdResults_struct pgd;
    struct GFAST_props_struct props;
    struct GFAST_shakeAlert_struct SA; 
    char *amqMessage;
    double t0, t1; 
    const enum opmode_type opmode = OFFLINE;
    int ierr, iev;
    bool lacquire, lnewEvent;
    // Initialize 
    ierr = 0;
    memset(&props,    0, sizeof(struct GFAST_props_struct));
    memset(&gps_data, 0, sizeof(struct GFAST_data_struct));
    memset(&events, 0, sizeof(struct GFAST_activeEvents_struct));
    memset(&pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    memset(&cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    memset(&ff, 0, sizeof(struct GFAST_ffResults_struct));
    memset(&pgd_data, 0, sizeof( struct GFAST_peakDisplacementData_struct));
    memset(&cmt_data, 0, sizeof(struct GFAST_offsetData_struct));
    memset(&ff_data, 0, sizeof(struct GFAST_offsetData_struct));
    ISCL_iscl_init(); // Fire up the computational library
    // Read the properties file
    log_infoF("%s: Reading the properties file...\n", fcnm);
    ierr = GFAST_core_properties_initialize(propfilename, opmode, &props);
    if (ierr != 0) 
    {   
        log_errorF("%s: Error reading the GFAST properties file\n", fcnm);
        goto ERROR;
    }
    if (props.verbose > 2)
    {   
        GFAST_core_properties_print(props);
    }
    // Initialize the buffers
    ierr = GFAST_core_data_initialize(props, &gps_data); 
    if (ierr != 0)
    {
        log_errorF("%s: Error initializing data buffers\n", fcnm);
        goto ERROR;
    }
    // Initialize PGD
    ierr = GFAST_core_scaling_pgd_initialize(props.pgd_props, gps_data,
                                             &pgd, &pgd_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing PGD\n", fcnm);
        goto ERROR;
    }
    // Initialize CMT
    ierr = GFAST_core_cmt_initialize(props.cmt_props, gps_data,
                                     &cmt, &cmt_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing CMT\n", fcnm);
        goto ERROR;
    }
    // Initialize finite fault
    ierr = GFAST_core_ff_initialize(props.ff_props, gps_data,
                                    &ff, &ff_data);
    if (ierr != 0)
    {   
        log_errorF("%s: Error initializing FF\n", fcnm);
        goto ERROR;
    }

ERROR:;
    GFAST_core_cmt_finalize(&props.cmt_props,
                            &cmt_data,
                            &cmt);
    GFAST_core_ff_finalize(&props.ff_props,
                           &ff_data,
                           &ff);
    GFAST_core_scaling_pgd_finalize(&props.pgd_props,
                                    &pgd_data,
                                    &pgd);
    GFAST_core_data_finalize(&gps_data);
    GFAST_core_properties_finalize(&props);
    GFAST_core_data_finalize(&gps_data);
    ISCL_iscl_finalize();
    if (ierr != 0)
    {   
        printf("%s: Terminating with error\n", fcnm);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
