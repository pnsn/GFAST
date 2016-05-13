#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include "gfast.h"
/*!
 * @brief Unpacks the vertex's latitude, longitude, and depth 
 *
 * @param[in] xml_reader  corresponds to the children of the xmlNodePtr
 *                        segment node pointer
 * @param[in] VTX_NAN     default value for element 
 *
 * @param[out] ss         slip along strike (m)
 * @param[out] ss_uncer   uncertainty in the slip along strike (m)
 * @param[out] ds         slip down dip (m)
 * @param[out] ds_uncer   uncertainty in slip down dip (m)
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 * @date May 2016
 *
 * @bug There are no unit conversions thus the shakeAlert XML is expected to
 *      have units meters for all variables 
 *
 */
int GFAST_xml_slip__read(void *xml_reader, double VTX_NAN,
                         double *ss, double *ss_uncer,
                         double *ds, double *ds_uncer)
{
    const char *fcnm = "GFAST_xml_slip__read\0";
    xmlNodePtr slip_xml;
    xmlChar *value;
    enum unpack_types_enum
    {
        UNPACK_DOUBLE = 1
    };
    const int nitems = 4;
    const xmlChar *citems[4] = { BAD_CAST "ss\0",
                                 BAD_CAST "ss_uncer\0",
                                 BAD_CAST "ds\0",
                                 BAD_CAST "ds_uncer\0" };
    double values[4];
    const int types[4] = {UNPACK_DOUBLE,
                          UNPACK_DOUBLE,
                          UNPACK_DOUBLE,
                          UNPACK_DOUBLE};
    int ierr, item, item0, nfound;
    bool lunpack;
    //------------------------------------------------------------------------//
    //
    // Initialize
    ierr = 0;
    nfound = 0;
    for (item=0; item<nitems; item++)
    {   
        values[item] = VTX_NAN;
    }   
    if (xml_reader == NULL)
    {
        log_errorF("%s: Error NULL poitner\n", fcnm);
        goto ERROR;
    }
    slip_xml = (xmlNodePtr )xml_reader;
    if ((xmlStrcmp(slip_xml->name, (const xmlChar *) "slip\0")))
    {
        log_errorF("%s: Error reader must start at <slip>\n", fcnm);
        ierr = 1;
        goto ERROR;
    }
    // Try to get the ss, ss_uncer, ds, ds_uncer 
    item0 =-1;
    while (slip_xml != NULL){
        // Require we are working on an element node
        if (slip_xml->type != XML_ELEMENT_NODE){goto NEXT_SLIP_XML;}
        lunpack = false;
        // Try taking the next item in the citems
        if (item0 < nitems){
            if (!xmlStrcmp(slip_xml->name, citems[item0+1]))
            {
                lunpack = true;
                item0 = item0 + 1;
            }
        }
        // Otherwise do a linear search of citems
        if (!lunpack)
        {
            for (item=0; item<nitems; item++)
            {
                if (!xmlStrcmp(slip_xml->name, citems[item]))
                {
                    lunpack = true;
                    item0 = item + 1;
                    break;
                }
            }
        }
        // I didn't find this value in citems - weird - continue
        if (!lunpack)
        {
            log_warnF("%s: Warning couldn't find item %s\n", fcnm,
                      (char *)slip_xml->name);
            goto NEXT_SLIP_XML;
        }
        // Check on item index to avoid a segfault
        if (item0 < 0 || item0 > nitems - 1)
        {
            log_errorF("%s: Invalid index %s %d\n",
                        fcnm, (char *)slip_xml->name, item0);
            ierr = 1;
            goto ERROR;
        }
        // FIXME - unpack units and convert

        // Is there a value to unpack
        value = xmlNodeGetContent(slip_xml);
        if (value != NULL)
        {
            if (types[item0] == UNPACK_DOUBLE)
            {
                values[item0] = xmlXPathCastStringToNumber(value);
            }
            else
            {
                log_errorF("%s: Invalid type %d!\n", fcnm, types[item0]);
                goto ERROR;
            }
            nfound = nfound + 1;
            free(value);
        }
NEXT_SLIP_XML:;
        slip_xml = slip_xml->next;
    } // Loop on slip 
ERROR:;
    // Get what we can
    if (ierr == 0){
        *ss       = values[0];
        *ss_uncer = values[1];
        *ds       = values[2];
        *ds_uncer = values[3];
        if (nfound < 4)
        {
            for (item=0; item<nitems; item++)
            {
                log_warnF("%s: Couldn't find: %s\n", fcnm, citems[item]);
            }
        }
    }else{
        log_errorF("%s: Internal error\n", fcnm);
        ierr = 1;
    }
    return ierr;
}
