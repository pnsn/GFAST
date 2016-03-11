#include "gfast.h"

class GFAST_props
{
    public:
        struct GFAST_props_struct props;
        int paraminit()
        {
            return GFAST_paraminit(propfilename, &props);
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
    private:
        char *propfilename;
};

class GFAST_data
{
    public:
       struct GFAST_data_struct gps_data;
       /*!< Initialize locations */
       int locinit()
       {
           return GFAST_locinit(_props, &gps_data);
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
    private:
       struct GFAST_props_struct _props;
};
