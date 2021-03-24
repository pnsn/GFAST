#ifndef ISCL_ISCL_H__
#define ISCL_ISCL_H__ 1
#include <stdbool.h>
#include "iscl/iscl/iscl_defs.h"
#include "iscl/iscl/iscl_enum.h"
// #include "iscl/signal/signal_types.h"
#ifdef OPENMP
#include <omp.h>
#endif

/*!
 * @defgroup iscl ISCL
 * @brief ISCL initialization and configuration routines.
 * @copyright ISTI distributed under the Apache 2 license.
 */

enum iscl_enum_type
{
    ISCL_INT_TYPE = 1,       /*!< Integer32 (int or int32_t) */
    ISCL_DOUBLE_TYPE = 11,   /*!< Float 64 (double) */
    ISCL_COMPLEX_TYPE = 21,  /*!< Complex 128 (double complex) */
    ISCL_BOOL_TYPE = 31      /*!< Boolean type (bool) */
};

#ifdef __cplusplus
extern "C"
{
#endif

/* Initializes the ISCL library */
void iscl_init(void);
/* Sets the number of threads for OpenMP programs */
enum isclError_enum iscl_setNumThreads(const int nt);
/* Gets the number of threads */
int iscl_getNumThreads(void);
/* Sets  the verbosity level */
enum isclError_enum
    iscl_setVerbosityLevel(const enum isclVerbosityLevel_enum verbosity);
/* Gets the verbosity level */
enum isclVerbosityLevel_enum  iscl_getVerbosityLevel(void);
/* Private function that sets global variable liscl_init to false */
void __iscl_uninit(void);
/* Private function to determine if library was initialized */ 
bool __iscl_isinit(void);
/* Finalize the ISCL library */
void iscl_finalize(void);
// /* Load an IIR filter */
// bool iscl_iirfilter_loadBA(const int n, const double *Wn,
//                            const double rp, const double rs,
//                            const enum iirDesignBandType_enum btype,
//                            const bool lanalog,
//                            const enum iirDesignFilterType_enum ftype,
//                            struct signalBA_struct *baOut);
// bool iscl_iirfilter_loadZPK(const int n, const double *Wn,
//                             const double rp, const double rs,
//                             const enum iirDesignBandType_enum btype,
//                             const bool lanalog,
//                             const enum iirDesignFilterType_enum ftype,
//                             struct signalZPK_struct *zpkOut);
// /* Save an IIR filter */
// void iscl_iirfilter_saveZPK(const int n, const double *Wn,
//                             const double rp, const double rs, 
//                             const enum iirDesignBandType_enum btype,
//                             const bool lanalog,
//                             const enum iirDesignFilterType_enum ftype,
//                             const struct signalZPK_struct zpk);
// void iscl_iirfilter_saveBA(const int n, const double *Wn,
//                            const double rp, const double rs,
//                            const enum iirDesignBandType_enum btype,
//                            const bool lanalog,
//                            const enum iirDesignFilterType_enum ftype,
//                            const struct signalBA_struct ba);
/* Deallocate */
void iscl_iirfilter_destroy(void);

// void __iscl_iirfilter_load(const int n, const double *Wn,
//                            const double rp, const double rs, 
//                            const enum iirDesignBandType_enum btype,
//                            const bool lanalog,
//                            const enum iirDesignFilterType_enum ftype,
//                            struct signalZPK_struct *zpkout,
//                            bool *loaded);
// void __iscl_iirfilter_load__ba(const int n, const double *Wn,
//                                const double rp, const double rs, 
//                                const enum iirDesignBandType_enum btype,
//                                const bool lanalog,
//                                const enum iirDesignFilterType_enum ftype,
//                                struct signalBA_struct *ba,
//                                bool *loaded);
// /* Save an IIR filter */
// void __iscl_iirfilter_save(const int n, const double *Wn,
//                            const double rp, const double rs,
//                            const enum iirDesignBandType_enum btype,
//                            const bool lanalog,
//                            const enum iirDesignFilterType_enum ftype,
//                            struct signalZPK_struct zpk);
// void __iscl_iirfilter_save__ba(struct signalBA_struct ba);
/* Destroy an IIR filter */
void __iscl_iirfilter_destroy(void);


#define ISCL_iscl_init(...) iscl_init(__VA_ARGS__)
#define ISCL_iscl_finalize(...) iscl_finalize(__VA_ARGS__)
#define ISCL_iscl_getNumThreads(...) iscl_getNumThreads(__VA_ARGS__)
#define ISCL_iscl_setNumThreads(...) iscl_setNumThreads(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* ISCL_ISCL_H__ */
