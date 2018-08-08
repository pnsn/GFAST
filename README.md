# Introduction 

This is the source code for Geodetic First Approximation of Size and Timing (GFAST) geodetic earthquake early warning module.  For more detailed instructions check the doc/manual.pdf

# Directories

1. doc contains the GFAST user's manual.
2. include contains the GFAST C include files.
3. legacy is the original Python source code.
4. src contains the source code.
  + src/activeMQ contains C++ readers/writers and C interfaes for using ActiveMQ.
  + src/core contains the core GFAST computations.
  + eewUtils contains application specific functions for performing the earthquake early warning tasks.  This may be a useful directory for developers of other applications looking for examples of how to use GFAST's core functionality.
  + src/hdf5 contains the HDF5 interfaces for generating a self-describing archive or play-by-play of GFAST.
  + src/traceBuffer contains routines for reading an Earthworm ring and converting to a GFAST specific buffer.  The GFAST specific buffer is targeted for deprecation and should be avoided. 
  + src/uw contains functions specific to the University of Washington and Amazon project.
  + src/xml contains functions for certainly writing and potentially reading QuakeML and ShakeAlert specific XML.
5. unit_tests contains some simple regression tests for the core modules.

# Building GFAST 

## Dependencies (verify with manual):

1. [cmake](https://cmake.org/) >= v2.6 for generation of Makefiles

2. [LAPACK(E)](http://www.netlib.org/lapack/) and [(C)BLAS](http://www.netlib.org/blas/) for matrix algebra.  These are available through [MKL](https://software.intel.com/en-us/mkl) and [IPP](https://software.intel.com/en-us/intel-ipp/details).

3. [iniparser](https://github.com/ndevilla/iniparser) for parsing ini files.

4. [hdf5](https://support.hdfgroup.org/HDF5/) for archival of results.  Currently v1.10.1 is required but as soon the ring archiver in eewUtils is completed the required version will be downgraded.  If linking to static libraries this may also required [zlib](http://www.zlib.net) for compression.

5. [libxml2](http://xmlsoft.org/) for reading/writing ShakeAlert and QuakeML XML.

6. [ISCL](https://gitlab.isti.com/bbaker/iscl) for many small computations throughout GFAST.  If not using MKL and using static libraries only then this will require [fftw](http://www.fftw.org/).  Also, depending on your configuration it may also require [GeographicLib](https://geographiclib.sourceforge.io/).

7. [compearth](https://github.com/bakerb845/compearth) for the moment tensor decompositions.  Note, this will eventually be merged back into Carl Tape's compearth repository.  Also, you'll need to descend into momenttensor/c_src. 

8. [Earthworm](http://earthworm.isti.com/trac/earthworm/) v7.8 or greater with geojson2ew.  geojson2ew will require [rabbitmq](https://github.com/alanxz/rabbitmq-c) and [Jansson](https://github.com/akheron/jansson).

9. [ActiveMQ](http://activemq.apache.org/) both the Java and C++ portions.  These will require other things that you likely already have like libssl, libcrypto, and the Apache runtime library.

## Configuring 

To get CMake to behave I usually use configuration scripts.  

On a machine with MKL/IPP I might do something like

    #!/bin/bash
    /home/bakerb25/cmake-3.6.1/bin/cmake ./ -DCMAKE_BUILD_TYPE=DEBUG \
    -DCMAKE_INSTALL_PREFIX=./ \
    -DCMAKE_C_FLAGS="-g3 -O2 -fopenmp -Wall -Wno-unknown-pragmas" \
    -DEW_BUILD_FLAGS="-Dlinux -D_LINUX -D_INTEL -D_USE_SCHED -D_USE_PTHREADS" \
    -DCMAKE_CXX_FLAGS="-g3 -O2" \
    -DGFAST_INSTANCE="PNSN" \
    -DGFAST_USE_INTEL=FALSE \
    -DGFAST_USE_AMQ=TRUE \
    -DGFAST_USE_EW=TRUE \
    -DAPR_INCLUDE_DIR=/usr/include/apr-1 \
    -DLIBAMQ_INCLUDE_DIR=/usr/include/activemq-cpp-3.8.2 \
    -DLIBAMQ_LIBRARY=/usr/lib64/libactivemq-cpp.so \
    -DLSSL_LIBRARY=/usr/lib64/libssl.so.10 \
    -DLCRYPTO_LIBRARY=/usr/lib64/libcrypto.so.10 \
    -DLAPACKE_INCLUDE_DIR=/home/bakerb25/lapack-3.6.1/LAPACKE/include \
    -DLAPACKE_LIBRARY=/home/bakerb25/lapack-3.6.1/liblapacke.a \
    -DLAPACK_LIBRARY=/home/bakerb25/lapack-3.6.1/liblapack.a \
    -DCBLAS_INCLUDE_DIR=/home/bakerb25/lapack-3.6.1/CBLAS/include \
    -DCBLAS_LIBRARY=/home/bakerb25/lapack-3.6.1/libcblas.a \
    -DBLAS_LIBRARY=/home/bakerb25/lapack-3.6.1/libblas.a \
    -DH5_C_INCLUDE_DIR=/home/bakerb25/hdf5-1.10.1/include \
    -DH5_LIBRARY=/home/bakerb25/hdf5-1.10.1/lib/libhdf5.so \
    -DINIPARSER_INCLUDE_DIR=/home/bakerb25/iniparser/src \
    -DINIPARSER_LIBRARY=/home/bakerb25/iniparser/libiniparser.a \
    -DCOMPEARTH_INCLUDE_DIR=/home/bakerb25/compearth/momenttensor/c_src/include \
    -DCOMPEARTH_LIBRARY=/home/bakerb25/compearth/momenttensor/c_src/lib/libcompearth_shared.so \
    -DISCL_INCLUDE_DIR=/home/bakerb25/iscl/include \
    -DISCL_LIBRARY=/home/bakerb25/iscl/lib/libiscl_static.a \
    -DGEOLIB_LIBRARY=/home/bakerb25/GeographicLib-1.46/lib/libGeographic.a \
    -DFFTW3_LIBRARY=/home/bakerb25/fftw-3.3.5/lib/libfftw3.a \
    -DEW_INCLUDE_DIR=/home/bakerb25/earthworm/earthworm-working/include \
    -DEW_LIBRARY="/home/bakerb25/earthworm/earthworm-working/lib/libew.a" \
    -DLIBXML2_INCLUDE_DIR=/usr/include/libxml2 \
    -DLIBXML2_LIBRARY=/usr/lib64/libxml2.so

Another example, when building with MKL/IPP I'd do something like

    #!/bin/bash
    /usr/bin/cmake ./ -DCMAKE_BUILD_TYPE=DEBUG \
    -DCMAKE_INSTALL_PREFIX=./ \
    -DCMAKE_C_COMPILER=/usr/local/bin/clang \
    -DCMAKE_CXX_COMPILER=/usr/local/bin/clang++ \
    -DCMAKE_C_FLAGS="-g3 -O2 -Weverything -Wno-reserved-id-macro -Wno-padded -fopenmp" \
    -DEW_BUILD_FLAGS="-Dlinux -D_LINUX -D_INTEL -D_USE_SCHED -D_USE_PTHREADS" \
    -DCMAKE_CXX_FLAGS="-g3 -O2 -Weverything -fopenmp" \
    -DGFAST_INSTANCE="PNSN" \
    -DGFAST_USE_INTEL=TRUE \
    -DGFAST_USE_AMQ=TRUE \
    -DGFAST_USE_EW=TRUE \
    -DAPR_INCLUDE_DIR=/usr/include/apr-1.0 \
    -DLIBAMQ_INCLUDE_DIR=/home/bakerb25/cpp/activemq-cpp-library-3.9.3/include/activemq-cpp-3.9.3 \
    -DLIBAMQ_LIBRARY=/home/bakerb25/cpp/activemq-cpp-library-3.9.3/lib/libactivemq-cpp.so \
    -DLSSL_LIBRARY=/usr/lib/x86_64-linux-gnu/libssl.so \
    -DLCRYPTO_LIBRARY=/usr/lib/x86_64-linux-gnu/libcrypto.so \
    -DMKL_LIBRARY="/opt/intel/mkl/lib/intel64/libmkl_intel_lp64.so;/opt/intel/mkl/lib/intel64/libmkl_core.so;/opt/intel/mkl/lib/intel64/libmkl_sequential.so" \
    -DIPP_LIBRARY="/opt/intel/ipp/lib/intel64/libipps.so;/opt/intel/ipp/lib/intel64/libippvm.so;/opt/intel/ipp/lib/intel64/libippcore.so" \
    -DH5_C_INCLUDE_DIR=/home/bakerb25/C/hdf5-1.10.1_intel/include \
    -DH5_LIBRARY=/home/bakerb25/C/hdf5-1.10.1_intel/lib/libhdf5.so \
    -DINIPARSER_INCLUDE_DIR=/home/bakerb25/C/iniparser/src \
    -DINIPARSER_LIBRARY=/home/bakerb25/C/iniparser/libiniparser.a \
    -DCOMPEARTH_INCLUDE_DIR=/home/bakerb25/C/compearth/momenttensor/c_src/include \
    -DCOMPEARTH_LIBRARY=/home/bakerb25/C/compearth/momenttensor/c_src/lib/libcompearth_shared.so \
    -DISCL_INCLUDE_DIR=/home/bakerb25/C/iscl/include \
    -DISCL_LIBRARY="/home/bakerb25/C/iscl/lib/libiscl_shared.so;/opt/intel/lib/intel64/libirc.so" \
    -DGEOLIB_LIBRARY=/home/bakerb25/C/GeographicLib-1.46/lib/libGeographic.so \
    -DEW_INCLUDE_DIR=/home/bakerb25/C/earthworm/earthworm-working/include \
    -DEW_LIBRARY="/home/bakerb25/C/earthworm/earthworm-working/lib/libew.a" \
    -DLIBXML2_INCLUDE_DIR=/usr/include/libxml2 \
    -DLIBXML2_LIBRARY=/usr/lib/x86_64-linux-gnu/libxml2.so \
    -DUW_AMAZON=TRUE \
    -DJANSSON_LIBRARY=/home/bakerb25/C/jansson/lib/libjansson.a \
    -DJANSSON_INCLUDE_DIR=/home/bakerb25/C/jansson/include

You'll only need Janson if making the UW source.  


