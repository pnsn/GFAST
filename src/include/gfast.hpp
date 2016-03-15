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
           int paraminit()
           {
               int ierr;
               //return GFAST_paraminit(_propfilename, &props);
               char *fname = new char [propfilename.length()+1]; 
               std::strcpy(fname, propfilename.c_str());
               ierr = GFAST_paraminit(fname, &props);
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
       struct GFAST_props_struct props;
       public:
           struct GFAST_data_struct gps_data;
           /*!< Initialize locations */
           int locinit()
           {
               return GFAST_locinit(props, &gps_data);
           }
           /*!< Print locations */
           void print()
           {
               return GFAST_locinit_printLocations(&gps_data);
           }
           /*!< Free data */
           void clear()
           {
               return GFAST_memory_freeData(&gps_data);
           }
   }; // End the GFAST_Buffer class
} // namespace GFAST
