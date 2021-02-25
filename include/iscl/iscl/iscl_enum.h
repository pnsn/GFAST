#ifndef ISCL_ISCL_ENUM__
#define ISCL_ISCL_ENUM__ 1

enum isclError_enum
{
    ISCL_UNKNOWN_FAILURE =-1000, /*!< Unknown failure. */
    ISCL_MKL_FAILURE =-950,      /*!< Failure in MKL. */
    ISCL_MKL_ALLOC_FAILURE =-949, /*!< Failure in MKL to allocate memory. */
    ISCL_IPP_FAILURE =-945,      /*!< Failure in IPP. */
    ISCL_GMTIME_FAILURE = -940,  /*!< Error in gmtime. */
    ISCL_GEOLIB_FAILURE = -935,  /*!< GeographicLib failure. */
    ISCL_SYS_FAILURE = -930,     /*!< Error in sys functions like mkdir. */
    ISCL_LAPACK_FAILURE =-925,   /*!< Error in LAPACK(e). */
    ISCL_BLAS_FAILURE =-924,     /*!< Error in (c)BLAS. */
    ISCL_CONVERGENCE_ERROR =-50, /*!< Algorithm failed to converge. */
    ISCL_NULL_PATH =-41,         /*!< Path (directory or file) is NULL. */
    ISCL_EMPTY_PATH =-40,        /*!< Path (directory or file) is blank. */
    ISCL_ARRAY_TOO_SMALL =-3, /*!< Input array dimensions are too small. */
    ISCL_NULL_PTR =-2,        /*!< A required input/output pointer was NULL. */
    ISCL_INVALID_INPUT =-1,   /*!< A parameter to the function was incorrect. */ 
    ISCL_SUCCESS = 0,         /*!< No error.  Function was successful. */
    ISCL_ALLOC_FAILURE = 101, /*!< Memory allocation was unsuccesful. */
    ISCL_ALGORITHM_FAILURE = 102, /*!< Algorithmic failure. */
    ISCL_DIVISION_BY_ZERO = 103,  /*!< Division by zero. */
    ISCL_ARRAY_ALL_NANS = 104,    /*!< Array is all NaNs */
    ISCL_LSQR_EXCEEDED_CONLIM = 503, /*!< LSQR exceeded condition limit. */
    ISCL_LSQR_ITERLIM = 504          /*!< LSQR hit the iteration limit. */
};

enum isclVerbosityLevel_enum
{
    ISCL_SHOW_NONE = 0,                /*!< Do not show errors. */
    ISCL_SHOW_ERRORS = 1,              /*!< Show errors only (default). */
    ISCL_SHOW_ERRORS_AND_WARNINGS = 2, /*!< Show errors and warnings. */
    ISCL_SHOW_ALL = 3                  /*!< Show everything - errors, warnings,
                                            and debug information. */
};

enum iirDesignBandType_enum
{
    DESIGN_IIRBAND_LOWPASS = 0,   /*!< Lowpass filter design. */
    DESIGN_IIRBAND_HIGHPASS = 1,  /*!< Highpass filter design. */
    DESIGN_IIRBAND_BANDPASS = 2,  /*!< Bandpass filter design. */
    DESIGN_IIRBAND_BANDSTOP = 3,  /*!< Bandstop filter design. */
    DESIGN_IIRBAND_INVALID = 4    /*!< Invalid. */
};

enum iirDesignFilterType_enum
{
    DESIGN_IIRTYPE_BUTTER = 0, /*!< Butterworth filter design. */
    DESIGN_IIRTYPE_CHEBY1 = 1, /*!< Chebyshev I filter design. */
    DESIGN_IIRTYPE_CHEBY2 = 2, /*!< Chebyshev II filter design. */
    DESIGN_IIRTYPE_BESSEL = 3, /*!< Bessel filter design. */
    DESIGN_IIRTYPE_INVALID = 4 /*!< Invalid IIR design prototype. */
};

#endif
