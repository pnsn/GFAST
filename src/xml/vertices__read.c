#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include "gfast.h"

int GFAST_xml_vertices__read(void *xml_reader,
                             enum xml_segmentShape_enum shape,
                             double VTX_NAN,
                             double *__restrict__ lat,
                             double *__restrict__ lon,
                             double *__restrict__ depth)
{
    const char *fcnm = "GFAST_xml_vertices__read\0";
    xmlNodePtr vertices_xml, vertex_xml;
    int ierr, nv, nvref;
    //------------------------------------------------------------------------//
    //
    // Check the input
    ierr = 0; 
    nvref =-1;
    if (shape == RECTANGLE){
        nvref = 4;
    }else if (shape == TRIANGLE){
        nvref = 3;
    }else if (nvref == 2){
        nvref = 2;
    }else{
        log_errorF("%s: Invalid shape %d\n", fcnm, shape);
        return -1;
    }
    // Initialize result
    for (nv=0; nv<nvref; nv++){
        lat[nv] = VTX_NAN;
        lon[nv] = VTX_NAN;
        depth[nv] = VTX_NAN;
    }
    vertices_xml = (xmlNodePtr )xml_reader;
    // Get the group length
    nv = 0;
    ierr = 0;
    while(vertices_xml != NULL){
        if ((xmlStrcmp(vertices_xml->name, (const xmlChar *) "vertex\0"))){
            vertices_xml = vertices_xml->next;
            continue;
        }
        nv = nv + 1;
        vertex_xml = vertices_xml->xmlChildrenNode;
        ierr = ierr + GFAST_xml_vertex__read((void *)vertex_xml, VTX_NAN,
                                             &lat[nv], &lon[nv], &depth[nv]);
        if (ierr != 0){
            log_errorF("%s Error unpacking vertex %d\n", fcnm, nv + 1);
            ierr = ierr + 1;
        }
        vertices_xml = vertices_xml->next; 
    } // Loop on vertices
    // Note the failures
    if (shape == LINE && nv != nvref){
        log_errorF("%s: Error failed to get all vertices on line\n", fcnm);
        ierr = 1;
    }
    if (shape == TRIANGLE && nv != nvref){
        log_errorF("%s: Error failed to get all vertices on triangle \n",fcnm);
        ierr = 1;
    }
    if (shape == RECTANGLE && nv != nvref){
        log_errorF("%s: Error failed to get all vertices on rectangle\n", fcnm);
        ierr = 1;
    }
    if (ierr != 0){
        log_errorF("%s: Errors detect in unpacking vertices\n", fcnm);
    }
    return ierr;
}
