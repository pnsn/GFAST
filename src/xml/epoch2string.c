#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"
/*!
 * @brief converts the epochal time (s) to YYYY-MM-DDThh:mm:ss.sssZ
 *        format
 *
 * @param[in] epoch    epochal time to convert to string format
 *
 * @param[out] cepoch  corresponding string formatted time
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int xml_epoch2string(double epoch, char cepoch[128])
{
    const char *fcnm = "xml_epoch2string\0";
    double sec;
    int ierr, mday, month, nzhour, nzjday, nzmin, nzmusec, nzsec, nzyear;
    memset(cepoch, 0, 128);
    ierr = time_epoch2calendar(epoch,
                                &nzyear, &nzjday, &month, &mday,
                                &nzhour, &nzmin, &nzsec, &nzmusec);
    if (ierr != 0){
        log_errorF("%s: Error converting epochal time\n", fcnm);
        return -1;
    }
    sec = (double) nzsec + (double) nzmusec*1.e-6;
    sprintf(cepoch,
            "%04d-%02d-%02dT%02d:%02d:%04.3fZ",
            nzyear, month, mday, nzhour, nzmin, sec);
    return 0;
}
