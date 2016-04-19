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

#endif /* __GFAST_ENUM__ */

