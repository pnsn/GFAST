CC = gcc
CSHARED = -fPIC
MAKE_SHARED = -shared
CFLAGS =-O2 -Wall
INC_DIR = ./include
OBJ = ./Obj

# library and executable names 
GFAST_SHARED = libgfast.so
GFAST_STATIC = libgfast.a
GFAST = xgfast

# Lapack and BLAS
LLAPACK = /usr/lib/liblapacke.so /usr/lib/liblapack.so /usr/lib/libblas.so /usr/lib/libcblas.so
# Iniparser
LIBINI = /usr/lib/libiniparser.a
INCINI = -I/usr/include
