#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast.h"

/*!
 * @brief Frees memory associated with strong motion data structure
 *
 * @param[inout] sm      strong motion data structure with memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeStrongMotionData(struct GFAST_strongMotion_struct *sm)
{
    if (sm == NULL){return;}
    if (sm->z != NULL){
        free(sm->z);
        sm->z = NULL;
    }
    if (sm->n != NULL){
        free(sm->n);
        sm->n = NULL;
    }
    if (sm->e != NULL){
        free(sm->e);
        sm->e = NULL;
    }
    memset(sm, 0, sizeof(struct GFAST_strongMotion_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with collocated instrument
 *
 * @param[inout] data     collocated data structure with memory to be freed 
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeCollocatedData(struct GFAST_collocatedData_struct *data)
{
    if (data == NULL){return;}
    if (data->ubuff != NULL){
        free(data->ubuff);
        data->ubuff = NULL;
    }
    if (data->nbuff != NULL){
        free(data->nbuff);
        data->nbuff = NULL;
    }
    if (data->ebuff != NULL){
        free(data->ebuff);
        data->ebuff = NULL;
    }
    if (data->tbuff != NULL){
        free(data->tbuff);
        data->tbuff = NULL;
    }
    GFAST_memory_freeStrongMotionData(&data->sm);
    memset(data, 0, sizeof(struct GFAST_collocatedData_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with GPS data structure
 *
 * @param[inout] gps_data    GPS data structure with memory to be freed
 * 
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeData(struct GFAST_data_struct *gps_data)
{
    int k;
    if (gps_data == NULL){return;}
    if (gps_data->stream_length > 0 && gps_data->data != NULL){
        for (k=0; k<gps_data->stream_length; k++){
            GFAST_memory_freeCollocatedData(&gps_data->data[k]);
        }
        free(gps_data->data);
        gps_data->data = NULL;
    }
    memset(gps_data, 0, sizeof(struct GFAST_data_struct));
    return;
}
//============================================================================//
/*!
 * @brief Frees memory associated with parameter structure
 *
 * @param[inout] props     GFAST properties structure with memory to be freed
 *
 * @author Ben Baker (benbaker@isti.com)
 *
 */
void GFAST_memory_freeProps(struct GFAST_props_struct *props)
{
    memset(props, 0, sizeof(struct GFAST_props_struct));
    return;
}
