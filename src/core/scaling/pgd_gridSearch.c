#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include <omp.h>
#include "gfast_core.h"
#include "iscl/log/log.h"
#include "iscl/time/time.h"

/*!
 * @brief PGD scaling full grid search (lat, lon, depth) driver
 *
 * @param[in] l1               number of sites
 * @param[in] ndeps            number of source depths
 * @param[in] verbose          controls verbosity (< 2 is quiet)
 * @param[in] dist_tol         displacement tolerance (cm).  if the displacment
 *                             is less than dist_tol it would be set to dist_tol
 * @param[in] disp_def         displacement default (cm) if d < dist_tol
 * @param[in] utmSrcNorthings  source UTM northing positions (m) [nlat*nlon]
 * @param[in] utmSrcEastings   source UTM easting positions (m) [nlat*nlon]
 * @param[in] srcDepths        source depth in grid search (km) [ndeps]
 * @param[in] utmRecvNorthing  receiver UTM northing position (m) [l1]
 * @param[in] utmRecvEasting   receiver UTM easting position (m) [l1]
 * @param[in] staAlt           station elevation (m) [l1]
 * @param[in] d                site peak ground displacements (cm) [l1]
 * @param[in] wts              data weights on each observation [l1].
 *                             if NULL or if each weight is the same then
 *                             this array will be ignored.
 *
 * @param[out] srdist          source receiver distance between the
 *                             idep'th source and k'th receiver
 *                             [l1*ndeps*nlat*nlon].
 *                             the i'th site at the idep'th depth is given by
 *                             ilon*nlats*ndeps*l1 + ilat*ndeps*l1 + idep*l1 + i
 * @param[out] M               magnitude at each depth [ndeps*nlat*nlon].
 *                             the (ilat,ilon,idep)'th is given by 
 *                             ilon*nlats*ndeps + ilat*ndeps + idep
 * @param[out] VR              variance reduction (percentage) at each
 *                             depth [ndeps*nlat*nlon].
 *                             the (ilat,ilon,idep)'th is given by 
 *                             ilon*nlats*ndeps + ilat*ndeps + idep
 * @param[out] iqr             the interquartile range computed from the
 *                             difference of the 75th percentile of the 
 *                             weighted residuals and the 25th percentile
 *                             of the weighted residuals at each depth [ndeps].
 *                             the (ilat,ilon,idepth)idep'th is given by 
 *                             ilon*nlats*ndeps + ilat*ndeps + idep
 * @param[out] Uest            the PGD estimate peak ground displacements.
*                              the i'th estimate at the idep'th depth is given by
 *                             ilon*nlats*ndeps*l1 + ilat*ndeps*l1 + idep*l1 + i
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int core_scaling_pgd_gridSearch(const int l1, const int ndeps,
                                const int nlats, const int nlons,
                                const int verbose,
                                const double dist_tol,
                                const double disp_def,
                                const double *__restrict__ utmSrcEastings,
                                const double *__restrict__ utmSrcNorthings,
                                const double *__restrict__ srcDepths,
                                const double *__restrict__ utmRecvEasting,
                                const double *__restrict__ utmRecvNorthing,
                                const double *__restrict__ staAlt,
                                const double *__restrict__ d,
                                const double *__restrict__ wts,
                                double *__restrict__ srdist,
                                double *__restrict__ M,
                                double *__restrict__ VR, 
                                double *__restrict__ iqr,
                                double *__restrict__ Uest)
{
    const char *fcnm = "core_scaling_pgd_gridSearch\0";
    int ierr, ierr1, ilat, ilatLon, ilon;
    // Error check
    if (l1 < 1 || ndeps < 1 || nlats < 1 || nlons < 1)
    {
        if (l1 < 1)
        {
            log_errorF("%s: Error invalid number of input stations: %d\n",
                       fcnm, l1);
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
    if (utmSrcEastings == NULL || utmSrcNorthings == NULL ||
        srcDepths == NULL || utmRecvEasting == NULL ||
        utmRecvNorthing == NULL || staAlt == NULL ||
        d == NULL || M == NULL || VR == NULL || Uest == NULL || srdist == NULL)
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
        if (d == NULL){log_errorF("%s: d is NULL\n", fcnm);}
        if (M == NULL){log_errorF("%s: M is NULL\n", fcnm);}
        if (VR == NULL){log_errorF("%s: VR is NULL\n", fcnm);}
        if (Uest == NULL){log_errorF("%s: Uest is NULL\n", fcnm);}
        if (srdist == NULL){log_errorF("%s: srdist is NULL\n", fcnm);}
        return -1;
    }
    // Loop on the longitudes (eastings)
    ierr = 0;
#ifdef PARALLEL_CMT
    #pragma omp parallel for collapse(2) \
     private(ilatLon, ilat, ilon) \
     reduction(+:ierr), shared(srdist, M, VR, iqr, Uest) \
     default(none)
#endif
    for (ilon=0; ilon<nlons; ilon++)
    {
        // Scan through the latitudes (northings)
        for (ilat=0; ilat<nlats; ilat++)
        {
            ilatLon = ilon*nlats + ilat;
            ierr1 = core_scaling_pgd_depthGridSearch(l1, ndeps,
                                                     verbose,
                                                     dist_tol,
                                                     disp_def,
                                                     utmSrcEastings[ilatLon],
                                                     utmSrcNorthings[ilatLon],
                                                     srcDepths,
                                                     utmRecvEasting,
                                                     utmRecvNorthing,
                                                     staAlt,
                                                     d,
                                                     wts,
                                                     &srdist[ilatLon*ndeps*l1],
                                                     &M[ilatLon*ndeps],
                                                     &VR[ilatLon*ndeps],
                                                     &iqr[ilatLon*ndeps],
                                                     &Uest[ilatLon*ndeps*l1]);
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
