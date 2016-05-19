#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lapacke.h>
#include <cblas.h>
#include <stdbool.h>
#include <omp.h>
#include <lapacke.h>
#include "gfast.h"

bool __GFAST_CMT__getAvgDisplacement(int npts, bool lremove_disp0,
                                     double dt, double SA_time,
                                     double cmt_window_avg,
                                     double epoch,
                                     double *__restrict__ ubuff,
                                     double *__restrict__ nbuff,
                                     double *__restrict__ ebuff,
                                     double *uAvg, double *nAvg, double *eAvg);

/*!
 * @brief Drives the CMT estimation.
 *
 * @param[in] cmt_props  CMT inversion parameters
 * @param[in] SA_lat     event latitude (degrees) [-90,90]
 * @param[in] SA_lon     event longitude (degrees) [0,360]
 * @param[in] SA_dep     event depth (km)
 * @param[in] cmt_data   cmt offset data to invert
 *
 * @param[inout] cmt     on input contains the depths for the grid search
 *                       on output contains the corresponding variance
 *                       reduction, moment tensors, nodal planes at each
 *                       depth in the CMT grid search, and optimal depth
 *                       index.
 *
 * @result 0 indicates success
 *         1 indicates an error on the cmt structure
 *         2 indicates the offset data structure is invalid
 *         3 indicates there was insufficient data to invert
 *         4 indicates an error encountered during the computation
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_scaling_CMT__driver(
   struct GFAST_props_struct cmt_props,
   double SA_lat, double SA_lon, double SA_dep,
   struct GFAST_offsetData_struct cmt_data,
   struct GFAST_cmtResults_struct *cmt)
{
    const char *fcnm = "GFAST_scaling_CMT__driver\0";
    enum cmt_return_enum
    {   
        CMT_SUCCESS = 0,            /*<! CMT computation was successful */
        CMT_STRUCT_ERROR = 1,       /*!< CMT structure is invalid */
        CMT_OS_DATA_ERROR = 2,      /*!< CMT offset data structure invalid */
        CMT_INSUFFICIENT_DATA = 3,  /*!< Insufficient data to invert */
        CMT_COMPUTE_ERROR = 4       /*!< An internal error was encountered */
    };
    double *utmRecvEasting, *utmRecvNorthing, *staAlt,
           *eOffset, *eEst, *eWts, *nOffset, *nEst, *nWts,
           *uOffset, *uEst, *uWts,
           DC_pct, eres, nres, sum_res2, ures,
           utmSrcEasting, utmSrcNorthing, wte, wtn, wtu, x1, y1, x2, y2;
    int i, idep, ierr, ierr1, k, l1, zone_loc;
    bool *luse, lnorthp;
    //------------------------------------------------------------------------//
    //
    // Verify the input data structure makes sense
    ierr = CMT_SUCCESS;
    luse = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    staAlt = NULL;
    uOffset = NULL;
    nOffset = NULL;
    eOffset = NULL;
    uWts = NULL;
    nWts = NULL;
    eWts = NULL;
    nEst = NULL;
    eEst = NULL;
    uEst = NULL;
    if (cmt_data.nsites < 1)
    {
        ierr = CMT_OS_DATA_ERROR;
        if (cmt_props.verbose > 1)
        {
            log_warnF("%s: No peak displacement data\n", fcnm);
        }
        goto ERROR;
    }
    // Verify the output data structures
    if (cmt->ndeps < 1)
    {
        log_errorF("%s: No depths in CMT gridsearch!\n", fcnm);
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    if (cmt->objfn == NULL || cmt->mts == NULL ||
        cmt->str1 == NULL || cmt->str2 == NULL ||
        cmt->dip1 == NULL || cmt->dip2 == NULL ||
        cmt->rak1 == NULL || cmt->rak2 == NULL ||
        cmt->Mw == NULL || cmt->srcDepths == NULL ||
        cmt->EN == NULL || cmt->NN == NULL || cmt->UN == NULL)
    {
        if (cmt->objfn == NULL)
        {
            log_errorF("%s: Error cmt->objfn is NULL\n", fcnm);
        }
        if (cmt->mts == NULL)
        {
            log_errorF("%s: Error cmt->mts is NULL\n", fcnm);
        }
        if (cmt->str1 == NULL)
        {
            log_errorF("%s: Error cmt->str1 is NULL\n", fcnm);
        }
        if (cmt->str2 == NULL)
        {   
            log_errorF("%s: Error cmt->str2 is NULL\n", fcnm);
        }
        if (cmt->dip1 == NULL)
        {   
            log_errorF("%s: Error cmt->dip1 is NULL\n", fcnm);
        }
        if (cmt->dip2 == NULL)
        {
            log_errorF("%s: Error cmt->dip2 is NULL\n", fcnm);
        }
        if (cmt->rak1 == NULL)
        {
            log_errorF("%s: Error cmt->rak1 is NULL\n", fcnm);
        }
        if (cmt->rak2 == NULL)
        {
            log_errorF("%s: Error cmt->rak2 is NULL\n", fcnm);
        }
        if (cmt->Mw == NULL)
        {
            log_errorF("%s: Error Mw is NULL\n", fcnm);
        }
        if (cmt->srcDepths == NULL)
        {
            log_errorF("%s: Error srcDepths is NULL\n", fcnm);
        }
        if (cmt->EN == NULL)
        {
            log_errorF("%s: Error EN is NULL\n", fcnm);
        }
        if (cmt->NN == NULL)
        {
            log_errorF("%s: Error NN is NULL\n", fcnm);
        }
        if (cmt->UN == NULL)
        {
            log_errorF("%s: Error UN is NULL\n", fcnm);
        }
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    // Avoid a segfault
    if (cmt->nsites != cmt_data.nsites)
    {   
        log_errorF("%s: nsites on cmt and cmt_data differs %d %d\n",
                   fcnm, cmt->nsites, cmt_data.nsites);
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    // Warn in case hypocenter is outside of grid-search
    if (cmt_props.verbose > 1 &&
        (SA_dep < cmt->srcDepths[0] || SA_dep > cmt->srcDepths[cmt->ndeps-1]))
    {
        log_warnF("%s: Warning hypocenter isn't in grid search!\n", fcnm);
    }
    // Initialize result
    cmt->opt_indx =-1;
    #pragma omp simd
    for (i=0; i<cmt->nsites*cmt->ndeps; i++)
    {
        cmt->EN[i] = 0.0;
        cmt->NN[i] = 0.0;
        cmt->UN[i] = 0.0;
    }
    #pragma omp simd
    for (i=0; i<cmt->ndeps; i++)
    {
        cmt->objfn[i] = 0.0;
        cmt->str1[i] = 0.0;
        cmt->str2[i] = 0.0;
        cmt->dip1[i] = 0.0;
        cmt->dip2[i] = 0.0;
        cmt->rak1[i] = 0.0;
        cmt->rak2[i] = 0.0;
        cmt->Mw[i] = 0.0;
    }
    #pragma omp simd
    for (i=0; i<6*cmt->ndeps; i++)
    {
        cmt->mts[i] = 0.0;
    }
    // Require there is a sufficient amount of data to invert
    luse = GFAST_memory_calloc__bool(cmt_data.nsites);
    l1 = 0;
    for (k=0; k<cmt_data.nsites; k++)
    {
        wtu = cmt_data.wtu[k];
        wtn = cmt_data.wtn[k];
        wte = cmt_data.wte[k];
        if (!cmt_data.lactive[k] || fmax(wtu, fmax(wtn, wte)) <= 0.0)
        {
             continue;
        }
        luse[k] = true;
        l1 = l1 + 1;
    }
    if (l1 < cmt_props.cmt_min_sites)
    {
        if (cmt_props.verbose > 1)
        {
            log_warnF("%s: Insufficient data to invert %d < %d\n",
                      fcnm, l1, cmt_props.cmt_min_sites);
        }
        ierr = CMT_INSUFFICIENT_DATA;
        goto ERROR;
    }
    // Set space
    utmRecvNorthing = GFAST_memory_calloc__double(l1);
    utmRecvEasting  = GFAST_memory_calloc__double(l1);
    staAlt  = GFAST_memory_calloc__double(l1);
    uOffset = GFAST_memory_calloc__double(l1);
    nOffset = GFAST_memory_calloc__double(l1);
    eOffset = GFAST_memory_calloc__double(l1);
    uWts    = GFAST_memory_calloc__double(l1);
    nWts    = GFAST_memory_calloc__double(l1);
    eWts    = GFAST_memory_calloc__double(l1);
    nEst    = GFAST_memory_calloc__double(l1*cmt->ndeps);
    eEst    = GFAST_memory_calloc__double(l1*cmt->ndeps);
    uEst    = GFAST_memory_calloc__double(l1*cmt->ndeps);
    // Get the source location
    zone_loc = cmt_props.utm_zone; // Use input UTM zone
    if (zone_loc ==-12345){zone_loc =-1;} // Figure it out
    GFAST_coordtools__ll2utm(SA_lat, SA_lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    utmSrcNorthing = y1;
    utmSrcEasting = x1;
    // Require there is a sufficient amount of data to invert
    l1 = 0;
    for (k=0; k<cmt_data.nsites; k++)
    {
        if (!luse[k]){continue;}
        // Get the recevier UTM
        GFAST_coordtools__ll2utm(cmt_data.sta_lat[k],
                                 cmt_data.sta_lon[k],
                                 &y2, &x2,
                                 &lnorthp, &zone_loc);
        // Copy the pertinent data
        uOffset[l1] = cmt_data.ubuff[k];
        nOffset[l1] = cmt_data.nbuff[k];
        eOffset[l1] = cmt_data.ebuff[k];
        uWts[l1] = cmt_data.wtu[k];
        nWts[l1] = cmt_data.wtn[k];
        eWts[l1] = cmt_data.wte[k];
        utmRecvNorthing[l1] = y2;
        utmRecvEasting[l1] = x2;
        staAlt[l1] = cmt_data.sta_alt[k];
        l1 = l1 + 1;
    }
    // Invert!
    if (cmt_props.verbose > 2)
    { 
        log_debugF("%s: Inverting for CMT with %d sites\n", fcnm, l1);
    }
    ierr = GFAST_CMT__depthGridSearch(l1, cmt->ndeps,
                                      cmt_props.verbose,
                                      cmt_props.ldeviatoric_cmt,
                                      utmSrcEasting,
                                      utmSrcNorthing,
                                      cmt->srcDepths,
                                      utmRecvEasting,
                                      utmRecvNorthing,
                                      staAlt,
                                      nOffset,
                                      eOffset,
                                      uOffset,
                                      nWts,
                                      eWts,
                                      uWts,
                                      nEst,
                                      eEst,
                                      uEst,
                                      cmt->mts);
    if (ierr != 0)
    {   
        log_errorF("%s: Error in CMT gridsearch!\n", fcnm);
        ierr = CMT_COMPUTE_ERROR;
        goto ERROR;
    }
    ierr = 0;
#ifdef PARALLEL_CMT
    #pragma omp parallel for \
     private(DC_pct, eres, i, idep, ierr1, k, sum_res2, nres, ures) \
     shared(cmt, eOffset, eEst, fcnm, l1, luse, nOffset, nEst, uOffset, uEst) \
     reduction(+:ierr), default(none) 
#endif
    for (idep=0; idep<cmt->ndeps; idep++)
    {
        sum_res2 = 0.0;
        #pragma omp simd reduction(+:sum_res2)
        for (i=0; i<l1; i++){
            nres = nOffset[i] - nEst[idep*l1+i];
            eres = eOffset[i] - eEst[idep*l1+i];
            ures = uOffset[i] - uEst[idep*l1+i];
            sum_res2 = sum_res2 + nres*nres + eres*eres + ures*ures;
        }
        sum_res2 = sqrt(sum_res2);
        // Decompose the moment tensor
        ierr1 = GFAST_CMT__decomposeMomentTensor(1, &cmt->mts[6*idep],
                                                 &DC_pct,
                                                 &cmt->Mw[idep],
                                                 &cmt->str1[idep],
                                                 &cmt->str2[idep],
                                                 &cmt->dip1[idep],
                                                 &cmt->dip2[idep],
                                                 &cmt->rak1[idep],
                                                 &cmt->rak2[idep]);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error decomposing mt\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Prefer results with larger double couple percentages
        cmt->objfn[idep] = sum_res2/DC_pct;
        // Save the data
        i = 0;
        for (k=0; k<cmt->nsites; k++)
        {
            cmt->NN[idep*cmt->nsites+k] = 0.0;
            cmt->EN[idep*cmt->nsites+k] = 0.0;
            cmt->UN[idep*cmt->nsites+k] = 0.0;
            if (luse[k])
            {
                cmt->NN[idep*cmt->nsites+k] = nEst[idep*l1+i];
                cmt->EN[idep*cmt->nsites+k] = eEst[idep*l1+i];
                cmt->UN[idep*cmt->nsites+i] = uEst[idep*l1+i];
                i = i + 1;
            }
        }
    } // Loop on depths
    if (ierr != 0)
    {
        log_errorF("%s: Error decomposing moment tensor\n", fcnm);
        ierr = CMT_COMPUTE_ERROR;
    }
    // Get the optimimum index
    cmt->opt_indx = numpy_argmin(cmt->ndeps, cmt->objfn);
ERROR:;
    GFAST_memory_free__bool(&luse);
    GFAST_memory_free__double(&utmRecvNorthing);
    GFAST_memory_free__double(&utmRecvEasting);
    GFAST_memory_free__double(&staAlt);
    GFAST_memory_free__double(&uOffset);
    GFAST_memory_free__double(&nOffset);
    GFAST_memory_free__double(&eOffset);
    GFAST_memory_free__double(&nEst);
    GFAST_memory_free__double(&eEst);
    GFAST_memory_free__double(&uEst);
    GFAST_memory_free__double(&nWts);
    GFAST_memory_free__double(&eWts);
    GFAST_memory_free__double(&uWts);
    return ierr;
}
/*!
 * @brief Drives the CMT estimation.
 *
 * @param[in] props      properties for CMT grid search inversion
 * @param[in] SA         shakeAlert structure with hypocentral information
 * @param[in] gps_data   GPS data to invert
 *
 * @param[inout] cmt     on input contains the depths for the grid search
 *                       on output contains the corresponding variance
 *                       reduction, moment tensors, nodal planes at each
 *                       depth in the CMT grid search, and optimal depth
 *                       index.
 *
 * @result 0 indicates success
 *
 * @author Brendan Crowell (PNSN) and Ben Baker (ISTI)
 *
 */
int GFAST_CMT__driver2(struct GFAST_props_struct props,
                       struct GFAST_shakeAlert_struct SA,
                       struct GFAST_data_struct gps_data,
                       struct GFAST_cmtResults_struct *cmt)
{
    const char *fcnm = "GFAST_CMT__driver2\0";
    enum cmt_return_enum
    {
        CMT_SUCCESS = 0,
        CMT_STRUCT_ERROR = 1,
        CMT_GPS_DATA_ERROR = 2,
        CMT_INSUFFICIENT_DATA = 3,
        CMT_COMPUTE_ERROR = 4
    };
    double *utmRecvEasting, *utmRecvNorthing, *staAlt, *x2, *y2,
           *eAvgDisp, *eEst, *eWts, *nAvgDisp, *nEst, *nWts,
           *uAvgDisp, *uEst, *uWts,
           currentTime, DC_pct, distance, eAvg, effectiveHypoDist,
           eres, nAvg, nres, sum_res2, uAvg, ures, utmSrcEasting,
           utmSrcNorthing, x1, y1;
    int i, idep, ierr, ierr1, k, l1, nwork, zone_loc;
    bool lnorthp, luse;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = CMT_SUCCESS;
    x2 = NULL;
    y2 = NULL;
    utmRecvEasting = NULL;
    utmRecvNorthing = NULL;
    staAlt = NULL;
    uAvgDisp = NULL;
    nAvgDisp = NULL;
    eAvgDisp = NULL;
    uWts = NULL;
    nWts = NULL;
    eWts = NULL;
    nEst = NULL;
    eEst = NULL;
    uEst = NULL;
    // Reality check
    if (gps_data.stream_length < 1)
    {
        if (props.verbose > 1)
        {
            ierr = CMT_GPS_DATA_ERROR;
            log_warnF("%s: No GPS streams\n", fcnm);
        }
        goto ERROR;
    }
    if (cmt->ndeps < 1){
     
        log_errorF("%s: No depths in CMT gridsearch!\n", fcnm);
        ierr = CMT_STRUCT_ERROR;
        goto ERROR;
    }
    // Count the data and get the workspace 
    l1 = 0;
    nwork = 0;
    for (k=0; k<gps_data.stream_length; k++)
    {
        if (gps_data.data[k].lskip_cmt){continue;}
        nwork = fmax(gps_data.data[k].npts, nwork);
        l1 = l1 + 1;
    }
    if (l1 < props.cmt_min_sites)
    {
        if (props.verbose > 1)
        {
            if (l1 < 1)
            {
                log_warnF("%s: All sites masked in CMT estimation\n", fcnm);
            }
            else
            {
                log_warnF("%s: Too many masked sites to compute CMT\n", fcnm);
            }
        }
        ierr = CMT_GPS_DATA_ERROR;
        goto ERROR;
    }
    if (nwork < 1)
    {
        if (props.verbose > 1)
        {
            log_warnF("%s: There is no data\n", fcnm);
        }
        ierr = CMT_GPS_DATA_ERROR;
        goto ERROR;
    }
    // Initialize
    l1 = 0;
    x2 = GFAST_memory_calloc__double(gps_data.stream_length);
    y2 = GFAST_memory_calloc__double(gps_data.stream_length);
    utmRecvNorthing = GFAST_memory_calloc__double(gps_data.stream_length);
    utmRecvEasting  = GFAST_memory_calloc__double(gps_data.stream_length);
    staAlt = GFAST_memory_calloc__double(gps_data.stream_length);
    uAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    nAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    eAvgDisp = GFAST_memory_calloc__double(gps_data.stream_length);
    uWts     = GFAST_memory_calloc__double(gps_data.stream_length);
    nWts     = GFAST_memory_calloc__double(gps_data.stream_length);
    eWts     = GFAST_memory_calloc__double(gps_data.stream_length);
    // Get the source location
    zone_loc = props.utm_zone; // Use input UTM zone
    if (zone_loc ==-12345){zone_loc =-1;} // Figure it out
    GFAST_coordtools__ll2utm(SA.lat, SA.lon,
                             &y1, &x1,
                             &lnorthp, &zone_loc);
    utmSrcNorthing = y1; 
    utmSrcEasting = x1;
    // Loop on the receivers, get distances, and data
    for (k=0; k<gps_data.stream_length; k++){
        if (gps_data.data[k].lskip_cmt){continue;} // Not in inversion
        // Get the recevier UTM
        GFAST_coordtools__ll2utm(gps_data.data[k].sta_lat,
                                 gps_data.data[k].sta_lon,
                                 &y2[k], &x2[k],
                                 &lnorthp, &zone_loc);
        // Get the distance - remember source is + down and receiver is + up
        distance = sqrt( pow(x1 - x2[k], 2)
                       + pow(y1 - y2[k], 2)
                       + pow(SA.dep*1000.0 + gps_data.data[k].sta_alt, 2));
        distance = distance*1.e-3; // convert to km
        // In a perfect world is there any chance of having data?
        currentTime = gps_data.data[k].epoch
                    + (gps_data.data[k].npts - 1)*gps_data.data[k].dt;
        effectiveHypoDist = (currentTime - SA.time)*props.cmt_window_vel;
        if (distance < effectiveHypoDist){
            luse = __GFAST_CMT__getAvgDisplacement(gps_data.data[k].npts,
                                                   props.lremove_disp0,
                                                   gps_data.data[k].dt,
                                                   SA.time,
                                                   props.cmt_window_avg,
                                                   gps_data.data[k].epoch,
                                                   gps_data.data[k].ubuff,
                                                   gps_data.data[k].nbuff,
                                                   gps_data.data[k].ebuff,
                                                   &uAvg, &nAvg, &eAvg);
            if (luse){
                uAvgDisp[l1] = uAvg;
                nAvgDisp[l1] = nAvg;
                eAvgDisp[l1] = eAvg;
                uWts[l1] = 1.0;
                nWts[l1] = 1.0;
                eWts[l1] = 1.0;
                utmRecvNorthing[l1] = y2[k]; 
                utmRecvEasting[l1] = x2[k]; 
                staAlt[l1] = gps_data.data[k].sta_alt;
                l1 = l1 + 1;
            }
        }
    } // Loop on data streams
    if (l1 < props.cmt_min_sites){
        if (props.verbose > 1){
            log_warnF("%s: Insufficient data to invert\n", fcnm);
        }
        ierr = CMT_INSUFFICIENT_DATA;
        goto ERROR;
    }
    nEst = GFAST_memory_calloc__double(l1*cmt->ndeps);
    eEst = GFAST_memory_calloc__double(l1*cmt->ndeps);
    uEst = GFAST_memory_calloc__double(l1*cmt->ndeps);
    // Warn in case hypocenter is outside of grid-search
    if (SA.dep < cmt->srcDepths[0] || SA.dep > cmt->srcDepths[cmt->ndeps-1]){
        log_warnF("%s: Warning hypocenter isn't in grid search!\n", fcnm);
    }   
    // Invert!
    if (props.verbose > 2){ 
        log_debugF("%s: Inverting for CMT with %d sites\n", fcnm, l1);
    }
    ierr = GFAST_CMT__depthGridSearch(l1, cmt->ndeps,
                                      props.verbose,
                                      props.ldeviatoric_cmt,
                                      utmSrcEasting,
                                      utmSrcNorthing,
                                      cmt->srcDepths,
                                      utmRecvEasting,
                                      utmRecvNorthing,
                                      staAlt,
                                      nAvgDisp,
                                      eAvgDisp,
                                      uAvgDisp,
                                      nWts,
                                      eWts,
                                      uWts,
                                      nEst,
                                      eEst,
                                      uEst,
                                      cmt->mts);
    if (ierr != 0)
    {
        log_errorF("%s: Error in CMT gridsearch!\n", fcnm);
        goto ERROR;
    }
    // Compute the derived objective function
    ierr = 0;
#ifdef __PARALLEL_CMT
    #pragma omp parallel for \
     private(DC_pct, eres, i, idep, ierr1, sum_res2, nres, ures) \
     shared(cmt, eAvgDisp, eEst, fcnm, l1, nAvgDisp, nEst, uAvgDisp, uEst) \
     reduction(+:ierr), default(none) 
#endif
    for (idep=0; idep<cmt->ndeps; idep++){
        sum_res2 = 0.0;
        #pragma omp simd reduction(+:sum_res2)
        for (i=0; i<l1; i++){
            nres = nAvgDisp[i] - nEst[idep*l1+i];
            eres = eAvgDisp[i] - eEst[idep*l1+i];
            ures = uAvgDisp[i] - uEst[idep*l1+i];
            sum_res2 = sum_res2 + nres*nres + eres*eres + ures*ures;
        }
        sum_res2 = sqrt(sum_res2);
        // Decompose the moment tensor
        ierr1 = GFAST_CMT__decomposeMomentTensor(1, &cmt->mts[6*idep],
                                                 &DC_pct,
                                                 &cmt->Mw[idep],
                                                 &cmt->str1[idep],
                                                 &cmt->str2[idep],
                                                 &cmt->dip1[idep],
                                                 &cmt->dip2[idep],
                                                 &cmt->rak1[idep],
                                                 &cmt->rak2[idep]);
        if (ierr1 != 0)
        {
            log_errorF("%s: Error decomposing mt\n", fcnm);
            ierr = ierr + 1;
            continue;
        }
        // Prefer results with larger double couple percentages
        cmt->objfn[idep] = sum_res2/DC_pct;
    }
    // Get the optimimum index
    cmt->opt_indx = numpy_argmin(cmt->ndeps, cmt->objfn);
ERROR:;
    GFAST_memory_free__double(&x2);
    GFAST_memory_free__double(&y2);
    GFAST_memory_free__double(&utmRecvNorthing);
    GFAST_memory_free__double(&utmRecvEasting);
    GFAST_memory_free__double(&staAlt);
    GFAST_memory_free__double(&uAvgDisp);
    GFAST_memory_free__double(&nAvgDisp);
    GFAST_memory_free__double(&eAvgDisp);
    GFAST_memory_free__double(&nEst);
    GFAST_memory_free__double(&eEst);
    GFAST_memory_free__double(&uEst);
    GFAST_memory_free__double(&nWts);
    GFAST_memory_free__double(&eWts);
    GFAST_memory_free__double(&uWts);
    return ierr;
}
//============================================================================//
/*!
 * @brief Computes the maximum site distance (offset) from the
 *        3 channel GPS site data
 *
 * @param[in] npts             number of points in time series
 * @param[in] lremove_disp0    if true then remove the (u,n,e) position at 
 *                             origin time SA_time.
 * @param[in] dt               sampling period (s) of GPS buffers
 * @param[in] SA_time          epochal UTC origin time (s)
 * @param[in] cmt_window_avg   time from end of acquisition which the data
 *                             is to be averaged (s)
 * @param[in] epoch            epochal UTC start time (s) of GPS traces
 * @param[in] ubuff            vertical position [npts]
 * @param[in] nbuff            north position [npts] 
 * @param[in] ebuff            east position [npts]
 *
 * @param[out] uAvg            average displacement on the vertical channel
 * @param[out] nAvg            average displacement on the north channel
 * @param[out] eAvg            average displacement on the east channel
 *
 * @result if true then the data is usable in the inversion otherwise
 *         this site should be skipped.
 *
 * @author Ben Baker (ISTI)
 */
bool __GFAST_CMT__getAvgDisplacement(int npts, bool lremove_disp0,
                                     double dt, double SA_time,
                                     double cmt_window_avg,
                                     double epoch,
                                     double *__restrict__ ubuff,
                                     double *__restrict__ nbuff,
                                     double *__restrict__ ebuff,
                                     double *uAvg, double *nAvg, double *eAvg)
{
    double diffT, de, dn, du, e0, n0, u0, eAvgNan, nAvgNan, uAvgNan;
    int i, iavg, iavg1, indx0;
    bool luse;
    //------------------------------------------------------------------------//
    //
    // Initialize result 
    *uAvg = NAN;
    *nAvg = NAN;
    *eAvg = NAN;
    // Set the initial position
    u0 = 0.0;
    n0 = 0.0;
    e0 = 0.0;
    diffT = SA_time - epoch;
    indx0 = fmax(0, (int) (diffT/dt + 0.5));
    indx0 = fmin(npts-1, indx0);
    if (lremove_disp0){
        diffT = SA_time - epoch;
        u0 = ubuff[indx0];
        n0 = nbuff[indx0];
        e0 = ebuff[indx0];
        // Prevent a problem
        if (isnan(u0) || isnan(n0) || isnan(e0)){
            return NAN;
        }
    }
    // Set the start index
    indx0 = npts - (int) (cmt_window_avg/dt + 0.5);
    indx0 = fmax(0, indx0);
    diffT = epoch;
    uAvgNan = 0.0;
    nAvgNan = 0.0;
    eAvgNan = 0.0; 
    iavg = 0; 
    // Compute the average over the window
    #pragma omp simd reduction(+:iavg, eAvgNan, nAvgNan, uAvgNan)
    for (i=indx0; i<npts; i++){
        luse = false;
        du = 0.0;
        dn = 0.0;
        de = 0.0; 
        iavg1 = 0;
        if (!isnan(ubuff[i]) && !isnan(nbuff[i]) && !isnan(ebuff[i]) &&
            ubuff[i] >-999.0 && nbuff[i] >-999.0 && ebuff[i] >-999.0)
        {
            luse = true;
        }
        if (luse){du = ubuff[i] - u0;}
        if (luse){dn = nbuff[i] - n0;}
        if (luse){de = ebuff[i] - e0;}
        if (luse){iavg1 = 1;}
        uAvgNan = uAvgNan + du;
        nAvgNan = nAvgNan + dn;
        eAvgNan = eAvgNan + de;
        iavg = iavg + iavg1;
    } // Loop on data points
    // Average it if possible
    if (iavg > 0){
        *uAvg = uAvgNan/(double) iavg;
        *nAvg = nAvgNan/(double) iavg;
        *eAvg = eAvgNan/(double) iavg;
        luse = true;
    }else{
        luse = false;
    }
    return luse;
}
