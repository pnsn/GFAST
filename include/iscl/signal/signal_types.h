#ifndef SIGNAL_TYPES_H__
#define SIGNAL_TYPES_H__ 1
#include "iscl/iscl/iscl_defs.h"
#include "iscl/iscl/iscl_enum.h"

#ifndef __cplusplus

struct signalZPK_struct
{
    double complex *p;  /*!< Poles.  This is an array of dimension [npoles]. */
    double complex *z;  /*!< Zeros.  This is an array of dimension [nzeros]. */
    double k;           /*!< Gain. */
    int npoles;         /*!< Number of poles. */
    int nzeros;         /*!< Number of zeros. */
};

#else

struct signalZPK_struct
{
    void *p;     /*!< Poles.  This is an array of dimension [npoles]. */
    void *z;     /*!< Zeros.  This is an array of dimension [nzeros]. */
    double k;    /*!< Gain. */
    int npoles;  /*!< Number of poles. */
    int nzeros;  /*!< Number of zeros. */
};



#endif

struct signalBA_struct
{
    double *b; /*!< Numerator polynomial coefficients [nb] */ 
    double *a; /*!< Denominator polynomial coefficients [na] */
    int na;    /*!< Number of numerator coefficients */
    int nb;    /*!< Number of denominator coefficients */
};

enum signalSOSPairings_enum
{
    SOS_NEAREST = 0, /*!< Attempt to minimize the peak gain */
    SOS_KEEP_ODD = 1 /*!< Attempt to minimize peak gain s.t. odd order
                          systems should retain one section as first order */
};

struct signalSOS_struct
{
    double *b;  /*!< Numerators coefficients in second-order sections [3*ns].
                     Note, the is'th section coefficients begin at
                     index 3*is. */
    double *a;  /*!< Denominator coefficients in second-order sections [3*ns].
                     Note, the is'th section coefficients begina t index
                     3*is. */
    int ns; /*!< Number of sections */
};

#endif /* _signal_types_h__ */ 

