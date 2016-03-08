CXX = g++
CC = gcc
CSHARED = -fPIC
MAKE_SHARED = -shared
CXXFLAGS = -O2 -Wall
CFLAGS =-O2 -Wall
INC_DIR = ./include
OBJ = ./Obj

# library and executable names 
GFAST_SHARED = libgfast.so
GFAST_STATIC = libgfast.a
GFAST = xgfast

# Lapack and BLAS
LLAPACK = /usr/lib/liblapacke.so /usr/lib/liblapack.so /usr/lib/libblas.so /usr/lib/libcblas.so
INC_LAPACK = -I/usr/include
# Iniparser
LIB_INI = /usr/lib/libiniparser.a
INC_INI = -I/usr/include
