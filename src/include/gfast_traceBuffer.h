#include <hdf5.h>
#include <stdbool.h>
#include <limits.h>
#include "gfast_struct.h"

#ifndef __GFAST_TRACEBUFFER_H__
#define __GFAST_TRACEBUFFER_H__

struct h5trace_struct
{
    char *groupName;      /*!< Full path to HDF5 group (null terminated) */
    double *buffer1;      /*!< Dataset 1 [maxpts] */
    double *buffer2;      /*!< Dataset 2 [maxpts] */
    double *data;         /*!< Data to be copied onto other structure [ncopy] */
    double t1;            /*!< Start time of data (UTC-seconds) */
    double t1beg;         /*!< Epochal start time of buffer1 (UTC-seconds) */
    double t2beg;         /*!< Epochal start time of buffer2 (UTC-seconds) */
    double dt;            /*!< Sampling period (seconds) */
    int idest;            /*!< Maps this trace back to the appropriate
                               three-component data stream */
    int maxpts;           /*!< Max number of points in data buffers */
    int npts1;            /*!< Number of points in buffer 1 */
    int npts2;            /*!< Number of points in buffer 2 */
    int ncopy;            /*!< Number of points to copy */
};

struct h5traceBuffer_struct
{
    struct h5trace_struct *traces; /*!< HDF5 trace data structure */
    hid_t fileID;                  /*!< HDF5 file handle */
    bool linit;                    /*!< True if the structure has been
                                        initialized */
    int ntraces;                   /*!< Number of traces to collect */
};

#ifdef __cplusplus
extern "C"
{
#endif

/* Copies the trace buffer to the GFAST structure */
int traceBuffer_h5_copyTraceBufferToGFAST(
    struct h5traceBuffer_struct *traceBuffer,
    struct GFAST_data_struct *gps_data);
/* Initialize tracebuffer structure from GFAST */
int traceBuffer_h5_setTraceBufferFromGFAST(
    const double bufflen,
    struct GFAST_data_struct gps_data,
    struct h5traceBuffer_struct *traceBuffer);
/* Close the HDF5 file with the traces */
int traceBuffer_h5_finalize(struct h5traceBuffer_struct *h5trace);
/* Get data from the HDF5 file */
int traceBuffer_h5_getData(const double t1, const double t2,
                           struct h5traceBuffer_struct *h5traceBuffer);
/* Get a double array */
int traceBuffer_h5_getDoubleArray(const hid_t groupID,
                                  const int i1, const int i2, 
                                  const char *citem,
                                  const double traceNaN, 
                                  const int nwork,
                                  double *__restrict__ work);
/* Get scalar data from the HDF5 file */
int traceBuffer_h5_getScalars(const hid_t groupID,
                              const int intNaN,
                              const double doubleNaN,
                              int *maxpts,
                              double *dt, double *ts1, double *ts2);
/* Initialize the HDF5 tracebuffer structure */
int traceBuffer_h5_initialize(const int job,
                              const bool linMemory,
                              const char *h5dir,
                              const char *h5file,
                              struct h5traceBuffer_struct *h5traceBuffer);
/* Sets the HDF5 file name */
int traceBuffer_h5_setFileName(const char *h5dir,
                               const char *h5file,
                               char h5name[PATH_MAX]);

#define GFAST_traceBuffer_h5_copyTraceBufferToGFAST(...)       \
              traceBuffer_h5_copyTraceBufferToGFAST(__VA_ARGS__)
#define GFAST_traceBuffer_h5_finalize(...)       \
              traceBuffer_h5_finalize(__VA_ARGS__)
#define GFAST_traceBuffer_h5_getData(...)       \
              traceBuffer_h5_getData(__VA_ARGS__)
#define GFAST_traceBuffer_h5_getDoubleArray(...)       \
              traceBuffer_h5_getDoubleArray(__VA_ARGS__)
#define GFAST_traceBuffer_h5_getScalars(...)       \
              traceBuffer_h5_getScalars(__VA_ARGS__)
#define GFAST_traceBuffer_h5_initialize(...)       \
              traceBuffer_h5_initialize(__VA_ARGS__)
#define GFAST_traceBuffer_h5_setFileName(...)       \
              traceBuffer_h5_setFileName(__VA_ARGS__)
#define GFAST_traceBuffer_h5_setTraceBufferFromGFAST(...)       \
              traceBuffer_h5_setTraceBufferFromGFAST(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* __GFAST_TRACEBUFFER_H__ */
