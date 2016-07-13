CXX = g++
CC = gcc-5
#CXX = icpc
#CC = icc
#PROFILE = -pg
CSHARED = -fPIC
MAKE_SHARED = -shared
CXXFLAGS = -O2 -Wall -g3 -fopenmp $(PROFILE)
CFLAGS =-O2 -std=gnu11 -Wall -fopenmp -g3 $(PROFILE)
#CFLAGS =-O2 -std=gnu11 -Wall -openmp -vec-report=2 -g3
OBJ = ./Obj


GFAST_DIR = /home/bakerb3/C/GFAST

INC_GFAST =-I$(GFAST_DIR)/include

# library and executable names 
GFAST_SHARED = $(GFAST_DIR)/src/libgfast.so
GFAST_STATIC = $(GFAST_DIR)/src/libgfast.a
GFAST = xgfast

# XML2 directory
INC_XML2 = -I/usr/include/libxml2
LIB_XML2 = /usr/lib/x86_64-linux-gnu/libxml2.so
# Lapack and BLAS
INC_LAPACK = -I/usr/include
LIB_LAPACK = /usr/lib/liblapacke.so /usr/lib/liblapack.so \
	     /usr/lib/libcblas.so /usr/lib/libblas.so
MKLROOT = /opt/intel/mkl
LMKL = -Wl,--start-group \
	${MKLROOT}/lib/intel64/libmkl_intel_lp64.a \
	${MKLROOT}/lib/intel64/libmkl_core.a \
	${MKLROOT}/lib/intel64/libmkl_sequential.a \
	-Wl,--end-group -lpthread -lm -ldl
#LIB_LAPACK = $(LMKL)

# HDF5
LIB_Z = /home/bakerb3/C/zlib-1.2.8/libz.a
INC_HDF5 = -I/home/bakerb3/Fortran/hdf5-1.8.16/include
LIB_HDF5 = /home/bakerb3/Fortran/hdf5-1.8.16/lib/libhdf5.a $(LIB_Z)

INC_LAPACK = -I/usr/include
# Iniparser
LIB_INI = /home/bakerb3/C/iniparser/libiniparser.a
INC_INI = -I/home/bakerb3/C/iniparser/src
# libgeographic - this must be set if the WITH_LIBGEO pre-processor flag is set
INC_GEO = -I/home/bakerb3/Fortran/GeographicLib-1.46/include
LIB_GEO = /home/bakerb3/Fortran/GeographicLib-1.46/src/libGeographic.a
# SAC/IO for offline mode
LIB_SACIO = /home/bakerb3/TIM/utils/sacio/libsacio.so

INC_ALL = $(INC_GFAST) $(INC_GEO) $(INC_INI) $(INC_HDF5) $(INC_XML2) $(INC_LAPACK)
LIB_ALL = $(LIB_LAPACK) $(LIB_INI) $(LIB_XML2) $(LIB_HDF5) $(LIB_SACIO) -lm -ldl


