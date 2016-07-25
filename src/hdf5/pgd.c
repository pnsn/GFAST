#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include "gfast.h"
/*!
 * @brief Writes the PGD data and results to the given PGD group
 *
 * @author Ben Baker, ISTI
 *
 */ 
int GFAST_HDF5_PGD__write(hid_t pgd_group,
                          hid_t pgd_type,
                          struct GFAST_peakDisplacementData_struct pgd_data,
                          struct GFAST_pgdResults_struct pgd)
{
    const char *fcnm = "GFAST_HDF5_PGD__write\0";
    struct h5_peakDisplacementData_struct h5_pgdData;
    struct h5_pgdResults_struct h5_pgd;
    int ierr;
    //------------------------------------------------------------------------//
    //
    // Error handling
    ierr = 0;
    if (pgd_data.nsites < 1)
    {
        log_errorF("%s: Error no data to write\n", fcnm);
        return -1;
    }
    if (pgd.nsites < 1 || pgd.ndeps < 1)
    {
        log_errorF("%s: Error no results to write\n", fcnm);
        return -1;
    }   
    memset(&h5_pgdData, 0, sizeof(struct h5_peakDisplacementData_struct));
    memset(&h5_pgd, 0, sizeof(struct h5_pgdResults_struct));
    // Copy the data
    ierr = GFAST_HDF5__copyType__peakDisplacementData(COPY_DATA_TO_H5,
                                                      &pgd_data,
                                                      &h5_pgdData);
    if (ierr != 0)
    {
        log_errorF("%s: Error copying data\n", fcnm);
        return ierr;
    }
    // Copy the results 
    ierr = GFAST_HDF5__copyType__pgdResults(COPY_DATA_TO_H5,
                                            &pgd,
                                            &h5_pgd);
    if (ierr != 0)
    {
        log_errorF("%s: Error copying results\n", fcnm);
        goto ERROR;
    }
    // Write the data

    // Write the results

    // Free the h5_data and h5_results
ERROR:;

    return ierr;
}

/*!
 * @brief Reads the PGD data and results from the given PGD group
 *
 * @author Ben Baker, ISTI
 * 
 */
int GFAST_HDF5_PGD__read( )
{

    return 0;
}

//============================================================================//

