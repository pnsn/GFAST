#include <stdlib.h>
#include <string.h>
#include "iscl/memory/memory.h"
#ifdef ISCL_USE_MKL_MEMORY
#include <mkl.h>
#endif

/*!
 * @defgroup memory_free free
 * @brief Frees memory allocated by ISCL.
 * @ingroup memory 
 */
/*!
 * @brief Frees a pointer and sets the pointer to NULL.
 *
 * @param[in,out] p    On input the pointer to free and set to NULL.
 * @param[in,out] p    On output a NULL pointer.
 *
 * @ingroup memory_free
 *
 */
void memory_free__void(void **p)
{
#ifdef ISCL_USE_MKL_MEMORY
    if (*p != NULL)
    {
        mkl_free(*p);
        *p = NULL;
    }
#else
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
#endif
    return;
}
//============================================================================//
/*!
 * @brief Frees a double pointer and sets the pointer to NULL.
 *
 * @param[in,out] p    On input the pointer to free and set to NULL.
 * @param[in,out] p    On output a NULL pointer 
 *
 * @ingroup memory_free
 *
 */
void memory_free64f(double **p)
{
#ifdef ISCL_USE_MKL_MEMORY
    if (*p != NULL)
    {
        mkl_free(*p);
        *p = NULL;
    }
#else
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
#endif
    return;
}
//============================================================================//
/*!
 * @brief Frees an integer pointer and sets the pointer to NULL.
 *
 * @param[in,out] p    On input the pointer to free and set to NULL.
 * @param[in,out] p    On output a NULL pointer.
 *
 * @ingroup memory_free
 *
 */
void memory_free32i(int **p)
{
#ifdef ISCL_USE_MKL_MEMORY
    if (*p != NULL)
    {
        mkl_free(*p);
        *p = NULL;
    }
#else
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
#endif
    return;
}
//============================================================================//
/*!
 * @brief Frees a complex pointer and sets the pointer to NULL.
 *
 * @param[in,out] p    On input the pointer to free and set to NULL.
 * @param[in,out] p    On output a NULL pointer.
 *
 * @ingroup memory_free
 *
 */
void memory_free64z(double complex **p)
{
#ifdef ISCL_USE_MKL_MEMORY
    if (*p != NULL)
    {
        mkl_free(*p);
        *p = NULL;
    }
#else
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
#endif
    return;
}
//============================================================================//
/*!
 * @brief Frees a char pointer and sets the pointer to NULL.
 *
 * @param[in,out] p    On input the pointer to free and set to NULL.
 * @param[in,out] p    On output a NULL pointer.
 *
 * @ingroup memory_free
 *
 */
void memory_free8c(char **p)
{
#ifdef ISCL_USE_MKL_MEMORY
    if (*p != NULL)
    {   
        mkl_free(*p);
        *p = NULL;
    }
#else
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
#endif
    return;
}
//============================================================================//
/*!
 * @brief Frees a bool pointer and sets the pointer to NULL.
 *
 * @param[in,out] p    On input the pointer to free and set to NULL.
 * @param[in,out] p    On output a NULL pointer.
 *
 * @ingroup memory_free
 *
 */
void memory_free8l(bool **p)
{
#ifdef ISCL_USE_MKL_MEMORY
    if (*p != NULL)
    {
        mkl_free(*p);
        *p = NULL;
    }
#else
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
#endif
    return;
}
// Comment out because not used - CWU 3/29/21
// //============================================================================//
// /*!
//  * @brief Frees a zpk filter structure.
//  * @param[in,out] zpk   On input this is a zpk structure.
//  * @param[in,out] zpk   On exit the zpk structure memory is released and
//  *                      variables set to 0.
//  * @ingroup memory_free
//  */
// void memory_freeZPK(struct signalZPK_struct *zpk)
// {
//     memory_free64z(&zpk->p);
//     memory_free64z(&zpk->z);
//     memset(zpk, 0, sizeof(struct signalZPK_struct));
//     return; 
// }
// /*!
//  * @brief Frees a ba filter structure.
//  * @param[in,out] ba   On input this is a ba structure.
//  * @param[in,out] ba   On exit the ba structure memory is released and
//  *                     variables set to 0.
//  * @ingroup memory_free
//  */
// void memory_freeBA(struct signalBA_struct *ba)
// {
//     memory_free64f(&ba->b);
//     memory_free64f(&ba->a);
//     memset(ba, 0, sizeof(struct signalBA_struct));
// }
// /*!
//  * @brief Frees a SOS filter structure.
//  * @param[in,out] sos  On input this is a sos structure.
//  * @param[in,out] sos  On exit the ba structure memory is released and
//  *                     variables set to 0.
//  * @ingroup memory_free
//  */
// void memory_freeSOS(struct signalSOS_struct *sos)
// {
//     memory_free64f(&sos->b);
//     memory_free64f(&sos->a);
//     memset(sos, 0, sizeof(struct signalSOS_struct));
// }
//============================================================================//
/*!
 * @brief Frees a float pointer and sets the pointer to NULL.
 *
 * @param[in,out] p    On input the pointer to free and set to NULL.
 * @param[in,out] p    On output a NULL pointer.
 *
 * @ingroup memory_free
 *
 */
void memory_free32f(float **p)
{
#ifdef ISCL_USE_MKL_MEMORY
    if (*p != NULL)
    {
        mkl_free(*p);
        *p = NULL;
    }
#else
    if (*p != NULL)
    {
        free(*p);
        *p = NULL;
    }
#endif
    return;
}
