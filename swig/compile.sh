#!/bin/bash

# Note:
# If building against an instance of libgfast that's not installed
# in standard system directories, define these variables in the calling,
# environment using the form:
#

if [ -e gfast.py ]; then rm gfast.py; fi
if [ -e *egg-info ]; then rm *egg-info; fi
if [ -e gfast.pyc ]; then rm gfast.pyc; fi
if [ -e gfast_wrap.c ]; then rm gfast_wrap.c; fi
if [ -e _gfast.so ]; then rm _gfast.so; fi
if [ -d build ]; then rm -rf build; fi

export GFAST_INCLUDE_PATH=/home/bakerb3/C/GFAST/c_csrc/include
export GFAST_LD_PATH=/home/bakerb3/C/GFAST/c_csrc

if [ -z ${GFAST_INCLUDE_PATH+isNotSet} ]; then GFAST_INCLUDE_PATH=""; fi
if [ -z ${GFAST_LD_PATH+isNotSet} ]; then GFAST_LD_PATH=""; fi

swig -python gfast.i

python setup.py build_ext --include-dirs ${GFAST_INCLUDE_PATH} \
			  --library-dirs ${GFAST_LD_PATH} --libraries gfast 

python setup.py install --install-platlib=.

