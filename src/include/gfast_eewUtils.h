#include <omp.h>
#include "gfast_struct.h"
#ifndef __GFAST_EEWUTILS_H__
#define __GFAST_EEWUTILS_H__
#ifdef __cplusplus
extern "C" 
{
#endif

/* Parses the core XML message */
int eewUtils_parseCoreXML(const char *message,
                          const double SA_NAN,
                          struct GFAST_shakeAlert_struct *SA);

#define GFAST_eewUtils_parseCoreXML(...)       \
              eewUtils_parseCoreXML(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* __GFAST_EEWUTILS_H__ */
