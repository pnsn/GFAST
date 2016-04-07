#include <string>
#include <cstring>
#include "gfast.h"

namespace GFAST
{
   /* GFAST properties class */
   class Properties
   {
       //char *_propfilename;
       std::string propfilename;
       public:
           //Properties(std::string propfilename);
           struct GFAST_props_struct props;
           int init()
           {
               int ierr;
               //return GFAST_properties__init(_propfilename, &props);
               char *fname = new char [propfilename.length()+1]; 
               std::strcpy(fname, propfilename.c_str());
               ierr = GFAST_properties__init(fname, &props);
               delete[] fname;
               return ierr; 
           }
           void clear()
           {
               return GFAST_memory_freeProps(&props);
           }
/*
           void getProps(struct GFAST_props_struct *props)
           {
               return;
           }
*/
           struct GFAST_props_struct getProps()//(struct GFAST_props_struct *props)
           {
               return props;//struct GFAST_props_struct props;
           }
   }; // End the GFAST_Properties class

   /* GFAST buffer (data) class */
   class Buffer
   {
       public:
           struct GFAST_props_struct props;
           struct GFAST_data_struct gps_data;
       public:
           /*!< Initialize locations */
           int initLocations()
           {
               return GFAST_buffer__setLocations(props, &gps_data);
           }
           /*!< Initialize sampling period */
           int initSamplingPeriod()
           {
               return GFAST_buffer__setSiteSamplingPeriod(props, &gps_data);
           }
           /*!< Initialize buffers */
           int init()
           {
               int ierr = 0;
               gps_data.stream_length 
                  = GFAST_buffer__getNumberOfStreams(props);
               ierr = ierr + initLocations();
               ierr = ierr + initSamplingPeriod();
               return ierr;
           }
           /*!< Print locations */
           void print()
           {
               GFAST_buffer_print__locations(gps_data);
               GFAST_buffer_print__samplingPeriod(gps_data);
               return;
           }
           /*!< Free data */
           void clear()
           {
               return GFAST_memory_freeData(&gps_data);
           }
   }; // End the GFAST_Buffer class
} // namespace GFAST
