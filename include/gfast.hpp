#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include "gfast.h"

namespace GFAST
{
   class Properties
   {
       /*! @brief GFAST properties class containing module initialization
                  parameters. */
       //char *_propfilename;
       public:
           std::string propfilename;
           struct GFAST_props_struct props;
       public:
           //Properties(std::string propfilename);
           int init()
           {
               /*! @brief Initializes the GFAST properties from an
                          initialization file */
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

   class Buffer
   {
      /*! @brief GFAST buffer (data) class */
       //private:
           //std::vector<double> _ubuff; /*!< Vertical displacement buffer */
           //std::vector<double> _nbuff; /*!< North displacement buffer */
           //std::vector<double> _ebuff; /*!< East displacement buffer */
           //std::vector<double> _tbuff; /*!< Time buffer */
       public:
           struct GFAST_props_struct props;
           struct GFAST_data_struct gps_data;
       public:

           int initLocations()
           {
               /*! @brief Initialize site names and locations */
               return GFAST_buffer__setSitesAndLocations(props, &gps_data);
           }

           int initSamplingPeriod()
           {
               /*! @brief Initialize sampling periods for all streams */
               return GFAST_buffer__setSiteSamplingPeriod(props, &gps_data);
           }

           int init()
           {
               /*! @brief Convenience utility to fully initialize buffer */
               int ierr = 0;
               gps_data.stream_length 
                  = GFAST_buffer__getNumberOfStreams(props);
               if (gps_data.stream_length < 1){
                   log_errorF("GFAST::Buffer no data to read\n");
                   return 1;
               }
               ierr = ierr + initLocations();
               ierr = ierr + initSamplingPeriod();
               return ierr;
           }

           void print()
           {
               /*! @brief Print buffer information */
               GFAST_buffer_print__locations(gps_data);
               GFAST_buffer_print__samplingPeriod(gps_data);
               return;
           }

           void clear()
           {
               /*! @brief Free memory on the buffer */
               return GFAST_memory_freeData(&gps_data);
           }
   }; // End the GFAST_Buffer class

   class Scaling 
   {
       /*! @brief GFAST Peak ground displacement estimation class */
       private:
           std::vector<double> _d;     /*!< Displacement */ 
           std::vector<double> _r;     /*!< Hypocentral distance (km) */
           std::vector<double> _repi;  /*!< Epicentral distance (km) */
           double _variance_reduction; /*!< Variance reduction result from
                                            PGD least-squares solution */
           double _magnitude;          /*!< Estimated magnitude result from
                                            least squares solution */
       public:
           struct GFAST_props_struct props;
           struct GFAST_data_struct gps_data;
       public:
           int evaluate()
           {
               /*! @brief Evaluates the PGD scaling */
               int ierr; //, n;
               ierr = 0;
/*
               n = _d.size();
               ierr = GFAST_scaling_PGD(n, props.verbose,
                                        props.pgd_dist_tol, props.pgd_dist_def,
                                        &_d[0], &_r[0], &_repi[0],
                                        &_variance_reduction, &_magnitude);
*/
               if (ierr != 0){
                   log_errorF("Error computing PGD scaling\n");
                   return ierr;
               } 
               return ierr;
           }

           void clear()
           {
               /*! @brief Frees memory associated with PGD scaling */
               _d.clear();
               _r.clear();
               _repi.clear();
               return;
           }
   }; // End GFAST Scaling class

   /*! @brief GFAST Centroid moment tensor inversion class */
   class CMT
   {

   };

   /*! @brief GFAST Finite fault inversion class */
   class FF
   {

   };
} // namespace GFAST
