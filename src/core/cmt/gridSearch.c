#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "gfast_core.h"
#include "iscl/log/log.h"

/*!
 * @brief Performs the CMT depth grid search.
 *
 * @param[in] l1               number of sites
 * @param[in] ndeps            number of depths
 * @param[in] nlats            number of latitudes (northings)
 * @param[in] nlons            number of longitudes (eastings)
 * @param[in] verbose          controls the verbosity (0 is quiet)
 * @param[in] deviatoric       if true then constrain the inversion s.t. 
 *                             resulting moment tensor is purely deviatoric.
 *                             otherwise, invert for all six moment tensor
 *                             terms.
 * @param[in] utmSrcEastings   source easting UTM position (m).  the
 *                             ilat,ilon'th position in the grid is given by
 *                             ilon*nlats + ilat [nlats*nlons]
 * @param[in] utmSrcNorthings  source northing UTM position (m). the
 *                             ilat,ilon'th position in the grid is given by
 *                             ilon*nlats + ilat [nlats*nlons]
 * @param[in] srcDepths        source depths (km) in grid-search [ndeps]
 * @param[in] utmRecvEasting   receiver easting UTM positions (m) [l1]
 * @param[in] utmRecvNorthing  receiver northing UMT positions (m) [l1]
 * @param[in] staAlt           station elevations above sea level (m) [l1]
 * @param[in] nObsOffset       observed offset in north component (m)
 *                             for i'th site [l1]
 * @param[in] eObsOffset       observed offset in east component (m)
 *                             for i'th site [l1]
 * @param[in] uObsOffset       observed offset in vertical comonent (m)
 *                             for i'th site [l1]
 * @param[in] nWts             weight corresponding to i'th north offset
 *                             observation [l1]
 * @param[in] eWts             weight corresponding to i'th east offset
 *                             observation [l1]
 * @param[in] uWts             weight corresponding to i'th vertical offset
 *                             observation [l1]
 *
 * @param[out] nEst            estimate offset in the north component (m)
 *                             for the i'th site at all depths, lats, and lons.
 *                             [l1*ndeps*nlats*nlons]
 *                             the i'th site at the idep'th depth is given by
 *                             idep*l1 + i
 * @param[out] eEst            estimate offset in the east component (m)
 *                             for the i'th site at all depths, lats, and lons.
 *                             [l1*ndeps*nlats*nlons]
 *                             the i'th site at the idep'th depth is given by
 *                             ilon*nlats*ndeps*l1 + ilat*ndeps*l1 + idep*l1 + i
 * @param[out] uEst            estimate offset in the vertical component (m)
 *                             for the i'th site at all depths
 *                             [l1*ndeps*nlats*nlons]
 *                             the i'th site at the idep'th depth is given by
 *                             ilon*nlats*ndeps*l1 + ilat*ndeps*l1 + idep*l1 + i
 * @param[out] mts             the moment tensor terms (Nm) inverted for in
 *                             an NED system at each depth.  the id'th depth
 *                             is begins at index 6*id.  the moment tensors at
 *                             at each depth are packed:  
 *                             \f$ \{m_{xx}, m_{yy}, m_{zz},
 *                                   m_{xy}, m_{xz}, m_{yz} \} \f$. 
 *                             [6*l1*ndeps*nlats*nlons]
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 * @bug Only deviatoric constraint programmed.
 */
int core_cmt_gridSearch(const int l1,
                        const int ndeps, const int nlats, const int nlons,
                        const int verbose,
                        const bool deviatoric,
                        const double *__restrict__ utmSrcEastings,
                        const double *__restrict__ utmSrcNorthings,
                        const double *__restrict__ srcDepths,
                        const double *__restrict__ utmRecvEasting,
                        const double *__restrict__ utmRecvNorthing,
                        const double *__restrict__ staAlt,
                        const double *__restrict__ nObsOffset,
                        const double *__restrict__ eObsOffset,
                        const double *__restrict__ uObsOffset,
                        const double *__restrict__ nWts,
                        const double *__restrict__ eWts,
                        const double *__restrict__ uWts,
                        double *__restrict__ nEst,
                        double *__restrict__ eEst,
                        double *__restrict__ uEst,
                        double *__restrict__ mts)
{
    const char *fcnm = "core_cmt_gridSearch\0";
    int ierr, ierr1, ilat, ilon, ilatLon;
    //------------------------------------------------------------------------//
    //
    // Check for NULL arrays 
    if (utmSrcEastings == NULL || utmSrcNorthings == NULL ||
        srcDepths == NULL || utmRecvEasting == NULL ||
        utmRecvNorthing == NULL || staAlt == NULL ||
        nObsOffset == NULL || eObsOffset == NULL || uObsOffset == NULL ||
        nEst == NULL || eEst == NULL || uEst == NULL ||
        mts == NULL)
    {
        if (utmSrcEastings == NULL)
        {
            log_errorF("%s: utmSrcEastings is NULL!\n", fcnm);
        }
        if (utmSrcNorthings == NULL)
        {
            log_errorF("%s: utmSrcNorthings is NULL!\n", fcnm);
        } 
        if (srcDepths == NULL){log_errorF("%s: srcDepths is NULL!\n", fcnm);}
        if (utmRecvEasting == NULL)
        {
            log_errorF("%s: utmRecvEasting is NULL!\n", fcnm);
        }
        if (utmRecvNorthing == NULL)
        {
            log_errorF("%s: utmRecvNorthing is NULL!\n", fcnm);
        }
        if (staAlt == NULL){log_errorF("%s: staAlt is NULL\n", fcnm);}
        if (nObsOffset == NULL){log_errorF("%s: nObsOffset is NULL\n", fcnm);}
        if (eObsOffset == NULL){log_errorF("%s: eObsOffset is NULL\n", fcnm);}
        if (uObsOffset == NULL){log_errorF("%s: uObsOffset is NULL\n", fcnm);}
        if (nEst == NULL){log_errorF("%s: nEst is NULL\n", fcnm);}
        if (eEst == NULL){log_errorF("%s: eEst is NULL\n", fcnm);}
        if (uEst == NULL){log_errorF("%s: uEst is NULL\n", fcnm);}
        if (mts == NULL){log_errorF("%s: mts is NULL\n", fcnm);}
        return -1;
    }
    // Verify the sizes
    if (l1 < 1 || ndeps < 1 || nlats < 1 || nlons < 1)
    {
        if (l1 < 1)
        {
            log_errorF("%s: Error no observations %d\n", fcnm, l1);
        }
        if (ndeps < 1)
        {
            log_errorF("%s: Error invalid number of source depths: %d\n",
                       fcnm, ndeps);
        }
        if (nlons < 1)
        {
            log_errorF("%s: Error invalid number of lons (easting): %d\n",
                       fcnm, nlons);
        }
        if (nlats < 1)
        {
            log_errorF("%s: Error invalid number of lats (northings): %d\n",
                       fcnm, nlats);
        } 
        return -1;
    }
    // Prevent problems with deviatoric
    if (!deviatoric)
    {
        log_errorF("%s: Cannot perform general MT gridsearch!\n", fcnm);
        return -1;
    }
    // Loop on the longitudes (eastings)
    ierr = 0;
#ifdef PARALLEL_CMT
    #pragma omp parallel for collapse(2) \
     private(ilatLon, ilat, ilon) \
     reduction(+:ierr), shared(nEst, eEst, uEst, mts) \
     default(none)
#endif
    for (ilon=0; ilon<nlons; ilon++)
    {
        // Scan through the latitudes (northings)
        for (ilat=0; ilat<nlats; ilat++)
        {
            ilatLon = ilon*nlats + ilat;
            ierr1 = core_cmt_depthGridSearch(l1, ndeps,
                                             verbose,
                                             deviatoric,
                                             utmSrcEastings[ilatLon],
                                             utmSrcNorthings[ilatLon],
                                             srcDepths,
                                             utmRecvEasting,
                                             utmRecvNorthing,
                                             staAlt,
                                             nObsOffset,
                                             eObsOffset,
                                             uObsOffset,
                                             nWts,
                                             eWts,
                                             uWts,
                                             &nEst[ilatLon*ndeps*l1],
                                             &eEst[ilatLon*ndeps*l1],
                                             &uEst[ilatLon*ndeps*l1],
                                             &mts[6*ilatLon*ndeps]);
            if (ierr1 != 0)
            {
                log_errorF("%s: Error calling depthGridSearch %d %d\n",
                           fcnm, ilat, ilon);
                ierr = ierr + 1;
            }
        } // loop on latitudes 
    } // loop on longitudes
    // Check if i encountered an error
    if (ierr != 0)
    {
        log_errorF("%s: Error in gridsearch\n", fcnm);
        return -2;
    }
    return 0; 
}
