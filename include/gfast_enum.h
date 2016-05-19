#ifndef __GFAST_ENUM__
#define __GFAST_ENUM__

enum dtinit_type
{
    INIT_DT_FROM_DEFAULT = 1,  /*!< Sets GPS sampling period to default */
    INIT_DT_FROM_FILE = 2,     /*!< Obtains GPS sampling period from file */
    INIT_DT_FROM_TRACEBUF = 3, /*!< Obtains GPS sampling period from 
                                    Earthworm tracebuf */
    INIT_DT_FROM_SAC = 4       /*!< Obtains GPS sampling period from 
                                    SAC file */
};

enum locinit_type
{
    INIT_LOCS_FROM_FILE = 1,     /*!< Sets the GPS site locations from 
                                      SOPAC SECTOR web service file */
    INIT_LOCS_FROM_TRACEBUF = 2, /*!< Sets the GPS site locations from
                                      Earthworm tracbuf */
    INIT_LOCS_FROM_SAC = 3       /*!< Sets the GPS site locations from
                                      SAC file */
};

enum opmode_type
{
    REAL_TIME = 1,   /*!< GFAST is running in `real time' mode */
    PLAYBACK = 2,    /*!< GFAST is running in historical playback mode */
    OFFLINE = 3      /*!< GFAST is running offline and obtaining data 
                          and configuration purely from files */
};

enum pgd_return_enum
{
    PGD_SUCCESS = 0,           /*!< PGD computation was successful */
    PGD_STRUCT_ERROR = 1,      /*!< PGD structure is invalid */
    PGD_PD_DATA_ERROR = 2,     /*!< PGD data structure invalid */
    PGD_INSUFFICIENT_DATA = 3, /*!< Insufficient data to invert */
    PGD_COMPUTE_ERROR = 4      /*!< An internal error was encountered */
};

enum cmt_return_enum
{
    CMT_SUCCESS = 0,            /*<! CMT computation was successful */
    CMT_STRUCT_ERROR = 1,       /*!< CMT structure is invalid */
    CMT_OS_DATA_ERROR = 2,      /*!< CMT offset data structure invalid */
    CMT_INSUFFICIENT_DATA = 3,  /*!< Insufficient data to invert */
    CMT_COMPUTE_ERROR = 4       /*!< An internal error was encountered */
};

enum ff_return_enum
{
    FF_SUCCESS = 0,            /*!< FF computation was successful */
    FF_STRUCT_ERROR = 1,       /*!< FF structure is invalid */
    FF_OS_DATA_ERROR = 2,      /*!< FF offset data structure is invalid */
    FF_INSUFFICIENT_DATA = 3,  /*!< Insufficient data to invert */
    FF_COMPUTE_ERROR = 4,      /*!< An internal error was encountered */
    FF_MEMORY_ERROR = 5        /*!< Error during memory allocation */
};

#endif /* __GFAST_ENUM__ */

