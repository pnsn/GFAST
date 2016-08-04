#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "gfast_xml.h"
#include "iscl/log/log.h"

int xml_quakeML_writeOrigin(const char *publicIDroot,
                            const char *evid,
                            const char *method,
                            struct qmlOrigin_struct *origin,
                            void *xml_writer)
{
    // Nothing to do
    if (origin == NULL){return 0;}

    return 0;
}
