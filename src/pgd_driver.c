#include <stdio.h>
/*! @defgroup PGD 
 *
 * This module is responsible for estimating the peak ground displacement 
 *
 */

/*!
 * @brief Peak ground displacement estimation driver routine
 *
 * @result 0 indicate success
 *
 * @addtogroup PGD
 */
int GFAST_eewdataengine_PGD( )
{
    GFAST_scaling_PGD(); 
    return 0;
}
