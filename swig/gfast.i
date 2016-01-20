%module gfast
%{
#define SWIG_FILE_WITH_INIT
#include <gfast.h>
%}
%include "typemaps.i"
int GFAST_CMTgreenF(int INPUT, int INPUT,
                    double *INPUT, double *INPUT, double *INPUT, double *INPUT,
                    double *OUTPUT);
void obspy_rotate_NE2RT(int INPUT, double *INPUT, double *INPUT, double INPUT,
                        double *OUTPUT, double *OUTPUT);

