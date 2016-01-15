%module gfast
%{
#define SWIG_FILE_WITH_INIT
#include <gfast.h>
%}
%include "typemaps.i"
int GFAST_CMTgreenF(int INPUT, int INPUT,
                    double *INPUT, double *INPUT, double *INPUT, double *INPUT,
                    double *OUTPUT);

