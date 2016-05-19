#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"

/*!
 * @brief Frees memory associated with peak ground displacement
 *
 * @param[inout] pgd_data     peak ground displacement data structure with
 *                            memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freePGDData(
   struct GFAST_peakDisplacementData_struct *pgd_data)
{
    int i;
    if (pgd_data->stnm != NULL)
    {   
        for (i=0; i<pgd_data->nsites; i++)
        {
            free(pgd_data->stnm[i]);
        }
        free(pgd_data->stnm);
    }
    GFAST_memory_free__double(&pgd_data->pd);
    GFAST_memory_free__double(&pgd_data->wt);
    GFAST_memory_free__double(&pgd_data->sta_lat);
    GFAST_memory_free__double(&pgd_data->sta_lon);
    GFAST_memory_free__double(&pgd_data->sta_alt);
    GFAST_memory_free__bool(&pgd_data->lmask);
    GFAST_memory_free__bool(&pgd_data->lactive);
    memset(pgd_data, 0, sizeof(struct GFAST_peakDisplacementData_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with the offset data 
 *
 * @param[inout] offset_data     offset data structure with memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeOffsetData(struct GFAST_offsetData_struct *offset_data)
{
    int i;
    if (offset_data->stnm != NULL)
    {
        for (i=0; i<offset_data->nsites; i++)
        {
            free(offset_data->stnm[i]);
        }
        free(offset_data->stnm);
    }
    GFAST_memory_free__double(&offset_data->ubuff);
    GFAST_memory_free__double(&offset_data->nbuff);
    GFAST_memory_free__double(&offset_data->ebuff);
    GFAST_memory_free__double(&offset_data->wtu);
    GFAST_memory_free__double(&offset_data->wtn);
    GFAST_memory_free__double(&offset_data->wte);
    GFAST_memory_free__double(&offset_data->sta_lat);
    GFAST_memory_free__double(&offset_data->sta_lon);
    GFAST_memory_free__double(&offset_data->sta_alt);
    GFAST_memory_free__bool(&offset_data->lmask);
    GFAST_memory_free__bool(&offset_data->lactive);
    memset(offset_data, 0, sizeof(struct GFAST_offsetData_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with waveform data on a site 
 *
 * @param[inout] data     collocated data structure with memory to be freed 
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeWaveformData(struct GFAST_waveformData_struct *data)
{
    if (data == NULL){return;}
    GFAST_memory_free__double(&data->ubuff);
    GFAST_memory_free__double(&data->nbuff);
    GFAST_memory_free__double(&data->ebuff);
    GFAST_memory_free__double(&data->tbuff);
    memset(data, 0, sizeof(struct GFAST_waveformData_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with GPS data structure
 *
 * @param[inout] gps_data    GPS data structure with memory to be freed
 * 
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeData(struct GFAST_data_struct *gps_data)
{
    int k;
    if (gps_data == NULL){return;}
    if (gps_data->stream_length > 0 && gps_data->data != NULL)
    {
        for (k=0; k<gps_data->stream_length; k++)
        {
            GFAST_memory_freeWaveformData(&gps_data->data[k]);
        }
        GFAST_memory_free(gps_data->data);
    }
    memset(gps_data, 0, sizeof(struct GFAST_data_struct));
    return;
}
//============================================================================//
/*!
 * @brief Free the PGD results structure
 *
 * @param[inout] pgd       PGD results structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freePGDResults(struct GFAST_pgdResults_struct *pgd)
{
    if (pgd == NULL){return;}
    GFAST_memory_free__double(&pgd->mpgd);
    GFAST_memory_free__double(&pgd->mpgd_vr);
    GFAST_memory_free__double(&pgd->srcDepths);
    GFAST_memory_free(pgd->lsiteUsed);
    memset(pgd, 0, sizeof(struct GFAST_pgdResults_struct));
    return;
}
//============================================================================//
/*!
 * @brief Free the CMT results structure
 *
 * @param[inout] cmt       CMT results structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freeCMTResults(struct GFAST_cmtResults_struct *cmt)
{
    if (cmt == NULL){return;}
    GFAST_memory_free__double(&cmt->objfn);
    GFAST_memory_free__double(&cmt->mts);
    GFAST_memory_free__double(&cmt->str1); 
    GFAST_memory_free__double(&cmt->str2);
    GFAST_memory_free__double(&cmt->dip1); 
    GFAST_memory_free__double(&cmt->dip2);
    GFAST_memory_free__double(&cmt->rak1); 
    GFAST_memory_free__double(&cmt->rak2);
    GFAST_memory_free__double(&cmt->Mw);
    GFAST_memory_free__double(&cmt->srcDepths);
    GFAST_memory_free__double(&cmt->EN);
    GFAST_memory_free__double(&cmt->NN);
    GFAST_memory_free__double(&cmt->UN);
    memset(cmt, 0, sizeof(struct GFAST_cmtResults_struct));
    return;
}
//============================================================================//
/*!
 * @brief Free the fault plane structure
 *
 * @param[inout] fp       fault plane structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freeFaultPlane(struct GFAST_faultPlane_struct *fp)
{
    if (fp == NULL){return;}
    GFAST_memory_free__double(&fp->lon_vtx);
    GFAST_memory_free__double(&fp->lat_vtx);
    GFAST_memory_free__double(&fp->dep_vtx);
    GFAST_memory_free__double(&fp->fault_xutm);
    GFAST_memory_free__double(&fp->fault_yutm);
    GFAST_memory_free__double(&fp->fault_alt);
    GFAST_memory_free__double(&fp->strike);
    GFAST_memory_free__double(&fp->dip);
    GFAST_memory_free__double(&fp->length);
    GFAST_memory_free__double(&fp->width);
    GFAST_memory_free__double(&fp->sslip);
    GFAST_memory_free__double(&fp->dslip);
    GFAST_memory_free__double(&fp->sslip_unc);
    GFAST_memory_free__double(&fp->dslip_unc);
    GFAST_memory_free__double(&fp->EN);
    GFAST_memory_free__double(&fp->NN);
    GFAST_memory_free__double(&fp->UN);
    GFAST_memory_free__double(&fp->Einp);
    GFAST_memory_free__double(&fp->Ninp);
    GFAST_memory_free__double(&fp->Uinp);
    GFAST_memory_free__int(&fp->fault_ptr);
    memset(fp, 0, sizeof(struct GFAST_faultPlane_struct));
    return;
}
//============================================================================//
/*!
 * @brief Free the finite fault results structure
 *
 * @param[inout] ff      finite fault results structure with memory to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freeFFResults(struct GFAST_ffResults_struct *ff)
{
    int ifp;
    if (ff == NULL){return;}
    if (ff->nfp < 1){return;}
    for (ifp=0; ifp<ff->nfp; ifp++){
        GFAST_memory_freeFaultPlane(&ff->fp[ifp]);
    }
    GFAST_memory_free(ff->fp);
    GFAST_memory_free__double(&ff->vr);
    GFAST_memory_free__double(&ff->Mw);
    GFAST_memory_free__double(&ff->str);
    GFAST_memory_free__double(&ff->dip);
    memset(ff, 0, sizeof(struct GFAST_ffResults_struct));
    return;    
}
//============================================================================//
/*!
 * @brief Frees memory on the active events structure
 *
 * @param[inout] events     active event list with data to be freed
 *
 * @author Ben Baker, ISTI
 *
 */
void GFAST_memory_freeEvents(struct GFAST_activeEvents_struct *events)
{
    if (events->nev > 0){
        GFAST_memory_free(events->SA);
    }
    memset(events, 0, sizeof(struct GFAST_activeEvents_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with parameter structure
 *
 * @param[inout] props     GFAST properties structure with memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeProps(struct GFAST_props_struct *props)
{
    memset(props, 0, sizeof(struct GFAST_props_struct));
    return;
}
//============================================================================//
/*!
 * @brief Allocates a double array 64 byte alignment 
 *
 * @param[in] n      size of array to allocate
 *
 * @result on successful exit this is the 64 byte memory aligned
 *         double array of length n.
 *
 * @author Ben Baker, ISTI
 *
 */
double *GFAST_memory_alloc__double(int n)
{
    const char *fcnm = "GFAST_memory_alloc__double\0";
    double *x = NULL;
    int ierr;
    if (n < 1){ 
        log_errorF("%s: Error invalid size %d\n", fcnm, n); 
        return x;
    }   
    ierr = posix_memalign( (void **)&x, CACHE_LINE_SIZE, n*sizeof(double));
    if (ierr != 0){ 
        log_errorF("%s: Error allocating array\n", fcnm);
        return NULL;
    }   
    return x;
}
//============================================================================//
/*!
 * @brief Allocates an integer array with 64 byte alignment 
 *
 * @param[in] n      size of array to allocate
 *
 * @result on successful exit this is the 64 byte memory aligned
 *         integer array of length n.
 *
 * @author Ben Baker, ISTI
 *
 */
int *GFAST_memory_alloc__int(int n)
{
    const char *fcnm = "GFAST_memory_alloc__int\0";
    int *x = NULL;
    int ierr;
    if (n < 1){
        log_errorF("%s: Error invalid size %d\n", fcnm, n);
        return x;
    }
    ierr = posix_memalign( (void **)&x, CACHE_LINE_SIZE, n*sizeof(int));
    if (ierr != 0){
        log_errorF("%s: Error allocating array\n", fcnm);
        return NULL;
    }
    return x;
}
//============================================================================//
/*!
 * @brief Allocates an boolean array with 64 byte alignment 
 *
 * @param[in] n      size of array to allocate
 *
 * @result on successful exit this is the 64 byte memory aligned
 *         boolean array of length n.
 *
 * @author Ben Baker, ISTI
 *
 */
bool *GFAST_memory_alloc__bool(int n)
{
    const char *fcnm = "GFAST_memory_alloc__bool\0";
    bool *x = NULL;
    int ierr;
    if (n < 1){
        log_errorF("%s: Error invalid size %d\n", fcnm, n);
        return x;
    }
    ierr = posix_memalign( (void **)&x, CACHE_LINE_SIZE, n*sizeof(bool));
    if (ierr != 0){
        log_errorF("%s: Error allocating array\n", fcnm);
        return NULL;
    }
    return x;
}
//============================================================================//
/*!
 * @brief Allocates a double array 64 byte alignment 
 *
 * @param[in] n      size of array to allocate
 *
 * @result on successful exit this is the 64 byte memory aligned
 *         double array of length n with all elements of array set
 *         to zero.
 *
 * @author Ben Baker, ISTI
 *
 */
double *GFAST_memory_calloc__double(int n)
{
    const char *fcnm = "GFAST_memory_calloc__double\0";
    double *x = NULL;
    if (n < 1){
        log_errorF("%s: Error invalid size %d\n", fcnm, n);
        return x;
    }
    x = GFAST_memory_alloc__double(n);
    if (x == NULL){
        log_errorF("%s: Error allocating array\n", fcnm);
        return NULL;
    }
    memset(x, 0, n*sizeof(x));
    return x;
}
//============================================================================//
/*!
 * @brief Allocates an integer array with 64 byte alignment 
 *
 * @param[in] n      size of array to allocate
 *
 * @result on successful exit this is the 64 byte memory aligned
 *         integer array of length n with all elements of array set
 *         to zero.
 *
 * @author Ben Baker, ISTI
 *
 */
int *GFAST_memory_calloc__int(int n)
{
    const char *fcnm = "GFAST_memory_calloc__int\0";
    int *x = NULL; 
    int i;
    const int zero = 0;
    if (n < 1){
        log_errorF("%s: Error invalid size %d\n", fcnm, n);
        return x;
    }
    x = GFAST_memory_alloc__int(n);
    if (x == NULL){
        log_errorF("%s: Error allocating array\n", fcnm);
        return NULL;
    }
    for (i=0; i<n; i++){
        x[i] = zero;
    }
    return x;
}
//============================================================================//
/*!
 * @brief Allocates a boolean array with 64 byte alignment 
 *
 * @param[in] n      size of array to allocate
 *
 * @result on successful exit this is the 64 byte memory aligned
 *         boolean array of length n with all elements of array set
 *         to false.
 *
 * @author Ben Baker, ISTI
 *
 */
bool *GFAST_memory_calloc__bool(int n)
{
    const char *fcnm = "GFAST_memory_calloc__bool\0";
    bool *x = NULL; 
    int i;
    const bool zero = false;
    if (n < 1){
        log_errorF("%s: Error invalid size %d\n", fcnm, n);
        return x;
    }
    x = GFAST_memory_alloc__bool(n);
    if (x == NULL){
        log_errorF("%s: Error allocating array\n", fcnm);
        return NULL;
    }
    for (i=0; i<n; i++){
        x[i] = zero;
    }
    return x;
}
//============================================================================//
/*!
 * @brief Frees a pointer and sets it to NULL
 *
 * @param[inout] p     On input the pointer to free and set to NULL.
 *                     On output a NULL pointer 
 *
 */
void GFAST_memory_free(void *p) 
{
    if (p != NULL){
        free(p);
        p = NULL;
    }   
    return;
}
//============================================================================//
/*!
 * @brief Frees a double pointer and sets it to NULL
 *
 * @param[inout] p     On input the pointer to free and set to NULL.
 *                     On output a NULL pointer 
 *
 */
void GFAST_memory_free__double(double **p)
{
    if (*p != NULL){
        free(*p);
        *p = NULL;
    }
    return;
}
//============================================================================//
/*!
 * @brief Frees an integer pointer and sets it to NULL
 *
 * @param[inout] p     On input the pointer to free and set to NULL.
 *                     On output a NULL pointer 
 *
 */
void GFAST_memory_free__int(int **p)
{
    if (*p != NULL){
        free(*p);
        *p = NULL;
    }
    return;
}
//============================================================================//
/*!
 * @brief Frees a bool pointer and sets it to NULL
 *
 * @param[inout] p     On input the pointer to free and set to NULL.
 *                     On output a NULL pointer 
 *
 */
void GFAST_memory_free__bool(bool **p)
{
    if (*p != NULL){
        free(*p);
        *p = NULL;
    }
    return;
}
