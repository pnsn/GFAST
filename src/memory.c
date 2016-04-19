#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"

/*!
 * @brief Frees memory associated with strong motion data structure
 *
 * @param[inout] sm      strong motion data structure with memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeStrongMotionData(struct GFAST_strongMotion_struct *sm)
{
    if (sm == NULL){return;}
    GFAST_memory_free(sm->z);
    GFAST_memory_free(sm->n);
    GFAST_memory_free(sm->e);
    memset(sm, 0, sizeof(struct GFAST_strongMotion_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with collocated instrument
 *
 * @param[inout] data     collocated data structure with memory to be freed 
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeCollocatedData(struct GFAST_collocatedData_struct *data)
{
    if (data == NULL){return;}
    GFAST_memory_free(data->ubuff);
    GFAST_memory_free(data->nbuff);
    GFAST_memory_free(data->ebuff);
    GFAST_memory_free(data->tbuff);
    GFAST_memory_freeStrongMotionData(&data->sm);
    memset(data, 0, sizeof(struct GFAST_collocatedData_struct));
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
    if (gps_data->stream_length > 0 && gps_data->data != NULL){
        for (k=0; k<gps_data->stream_length; k++){
            GFAST_memory_freeCollocatedData(&gps_data->data[k]);
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
    GFAST_memory_free(pgd->lsiteUsed);
    memset(pgd, 0, sizeof(struct GFAST_pgdResults_struct));
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
void memory_free__bool(bool **p)
{
    if (*p != NULL){
        free(*p);
        *p = NULL;
    }
    return;
}
