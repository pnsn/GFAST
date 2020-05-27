# Introduction 

This is the source code for Geodetic First Approximation of Size and Timing (GFAST) geodetic earthquake early warning module.  For more detailed instructions check the doc/manual.pdf

## Updated Instructions

2020/05/27 MTH: Working on new branch (=2020) with cleaned up dependencies and build instructions.

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
5. unit\_tests contains some simple regression tests for the core modules.

# Building GFAST 

## Dependencies (verify with manual):
### Overview of Dependencies

    GFAST 
        -mkl/ipp  -or- lapacke/cblas
        -compearth
            mkl/ipp  -or- lapacke/cblas
        -ISCL
            mkl/ipp  -or- lapacke/cblas/fftw
            geographicLib (optional)
        -activeMQ
            libssl, libcrypto
        -hdf5
        -libxml2
        -iniparser
    
0. compiler versions: gcc 4.8.5 vs gcc 7.3
gcc 4.8.5 does not recognize the #pragma omp simp directive so will generate a lot of warnings on compile.
gcc 7.3 will use this directive to auto vectorize the loops, which may improve performance.

### To install gcc 7.3 on centos 7
    
1. Install the CentOS SCL release file. It is part of the CentOS extras repository and can be installed by running the following command:

        >sudo yum install centos-release-scl
2. Install Developer Toolset version 7

        >sudo yum install devtoolset-7
3. Activate a new shell instance using the Software Collection scl tool:

        >scl enable devtoolset-7 bash
4. Check gcc version

        >gcc --version    // gcc (GCC) 7.3.1 20180303 (Red Hat 7.3.1-5)

## Which Linear Algebra package to use ?
1. mkl/ipp - optimized for intel
2. openBLAS - nearly as fast as 1. and a bit more general
3. lapack/blas "classic" - slowest option but easiest to install

### To install intel mkl/ipp on centos 7
    
Note: It may be possible to install these using the yum pkg manager as part of python:
https://software.intel.com/content/www/us/en/develop/articles/installing-intel-free-libs-and-python-yum-repo.html

But here's how to do it in stand-alone
1. Register and download the MKL/IPP tar.gz files from:
    https://software.intel.com/content/www/us/en/develop/articles/how-to-get-intel-mkl-ipp-daal.html

    or, we'll make them available in this repository
2. Install (by default installs into /opt/intel)

        >cd l_mkl_2020.0.166
        >sudo yum ./install.sh
        >cd ../l_ipp_2020.0.166
        >sudo yum ./install.sh
        
### To install lapack/blas/cblas on centos 7

1. lapack/blas and lapacke are likely already installed as part of centos 7:

        >ls /lib64/*lapack*
            /lib64/liblapacke.so@	 /lib64/liblapacke.so.3.4@    /lib64/liblapack.so@    /lib64/liblapack.so.3.4@
            /lib64/liblapacke.so.3@  /lib64/liblapacke.so.3.4.2*  /lib64/liblapack.so.3@  /lib64/liblapack.so.3.4.2*
          
     
2. However, it is likely that the lapacke.h header files need to be downloaded:

        > sudo yum install lapack-devel
        > rpm -ql lapack-devel
          /usr/include/lapacke
          /usr/include/lapacke/lapacke.h
          /usr/include/lapacke/lapacke_config.h
          /usr/include/lapacke/lapacke_mangling.h
          /usr/include/lapacke/lapacke_mangling_with_flags.h
          /usr/include/lapacke/lapacke_utils.h
          /usr/lib64/liblapack.so
          /usr/lib64/liblapacke.so
        
3. It is still necessary to install the cblas library + headers

        >sudo yum install atlas
        >sudo yum install atlas-devel

        >rpm -ql atlas
         /usr/lib64/atlas/libsatlas.so.3
         /usr/lib64/atlas/libsatlas.so.3.10
         /usr/lib64/atlas/libtatlas.so.3
         /usr/lib64/atlas/libtatlas.so.3.10
         
        >rpm -ql atlas-devel
         ...
         /usr/include/cblas.h
         /usr/include/clapack.h
         /usr/lib64/atlas/libsatlas.so
         /usr/lib64/atlas/libtatlas.so
    
### packages

1. [cmake](https://cmake.org/) >= v2.6 for generation of Makefiles

   centos7 ships with cmake ver 2.8.12.8 which may be adequate.
   Alternatively, cmake >= 3 can be installed by either:
   
        >sudo yum install cmake3
        
   or by downloading and building directly.

2. [LAPACK(E)](http://www.netlib.org/lapack/) and [(C)BLAS](http://www.netlib.org/blas/) for matrix algebra.  These are available through [MKL](https://software.intel.com/en-us/mkl) and [IPP](https://software.intel.com/en-us/intel-ipp/details).

    **See instructions above**

3. [iniparser](https://github.com/ndevilla/iniparser) for parsing ini files.

        >cd iniparser
        >make          // builds libiniparser.a and libiniparser.so.1
                       // include files in src/

4. [hdf5](https://support.hdfgroup.org/HDF5/) for archival of results.  Currently v1.10.1 is required but as soon the ring archiver in eewUtils is completed the required version will be downgraded.  If linking to static libraries this may also required [zlib](http://www.zlib.net) for compression.

        >sudo yum install hdf5
        >sudo yum install hdf5-devel

5. [libxml2](http://xmlsoft.org/) for reading/writing ShakeAlert and QuakeML XML.

        >sudo yum install libxml2
        >sudo yum install libxml2-devel

6. [ISCL](https://gitlab.isti.com/bbaker/iscl) for many small computations throughout GFAST.  If not using MKL and using static libraries only then this will require [fftw](http://www.fftw.org/).  Also, depending on your configuration it may also require [GeographicLib](https://geographiclib.sourceforge.io/).

        Download fftw-3.3.8.tar.gz from: http://www.fftw.org/download.html
        >cd fftw-3.3.8
        >./configure --enable-shared
        >make
        >make install
        
        Download GeographicLib from: https://geographiclib.sourceforge.io/html/install.html
        >cd GeographicLib-1.50.1
        >./configure --prefix=/usr/local/geographic [Default = /usr/local] 
        >makedir BUILD
        >cd BUILD
        >cmake ..
        
        Clone and build ISCL
        >git clone https://gitlab.isti.com/bbaker/iscl
        Edit zbuild_gcc.sh to select intel (MKL/IPP) if desired:
        USE_INTEL=true
        >Run zbuild_gcc.sh // This will run cmake and update CMakeCache.txt, etc.
        >make              // will build lib/libiscl_shared.so and lib/libiscl_static.a


7. [compearth](https://github.com/bakerb845/compearth) for the moment tensor decompositions.  Note, this will eventually be merged back into Carl Tape's compearth repository.  Also, you'll need to descend into momenttensor/c_src. 

        Clone and build
        >git clonehttps://github.com/bakerb845/compearth
        >cd compearth/momenttensor/c_src
        Edit zbuild_gcc.sh to select intel (MKL/IPP) if desired:
        USE_INTEL=false
        >Run zbuild_gcc.sh  // This will run cmake and update CMakeCache.txt, etc.
        >make               // will build lib/libcompearth_shared.so and lib/libcompearth_static.a
        
8. [Earthworm](http://earthworm.isti.com/trac/earthworm/) v7.8 or greater with geojson2ew.  geojson2ew will require [rabbitmq](https://github.com/alanxz/rabbitmq-c) and [Jansson](https://github.com/akheron/jansson).

    Checkout latest (confirm this is public access)
        >svn checkout svn://svn.isti.com/earthworm/trunk earthworm_svn
        often this resides in /opt/earthworm
        >cd /opt/earthworm/earthworm_svn
        >source environment/ew_linux.bash
        >cd src
        >make unix         // Check that /opt/earthworm/earthworm_svn/lib/libew_* is present
    
9. [ActiveMQ](http://activemq.apache.org/) both the Java and C++ portions.  These will require other things that you likely already have like libssl, libcrypto, and the Apache runtime library.

    Note: [MTH] I haven't found the APR to be necessary
    
    https://centos.pkgs.org/7/epel-aarch64/activemq-cpp-3.9.3-3.el7.aarch64.rpm.html

    The first two steps may not be necessary

    1. Download latest epel-release rpm from:
        http://download-ib01.fedoraproject.org/pub/epel/7/aarch64/

    2. Install epel-release rpm:
    
            >sudo rpm -Uvh epel-release*rpm
        
    3. Install activemq-cpp rpm package:
    
            >yum install activemq-cpp-devel
        
            >rpm -ql activemq-cpp-devel
            ...
            /usr/include/activemq-cpp-3.9.3/decaf/..
            /usr/lib64/libactivemq-cpp.so
            /usr/lib64/pkgconfig/activemq-cpp.pc
         
            > ls /usr/include/activemq-cpp-3.9.3/
                  activemq/  cms/  decaf/

    4. If libssl and libcrypto are not present:
    


## Configuring 

To get CMake to behave I usually use configuration scripts.  

On a machine with MKL/IPP I might do something like

    #!/bin/bash
    if [ -f Makefile ]; then
       make clean
    fi
    if [ -f CMakeCache.txt ]; then
       echo "Removing CMakeCache.txt"
       rm CMakeCache.txt
    fi
    if [ -d CMakeFiles ]; then
       echo "Removing CMakeFiles"
       rm -rf CMakeFiles
    fi
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
    if [ -f Makefile ]; then
       make clean
    fi
    if [ -f CMakeCache.txt ]; then
       echo "Removing CMakeCache.txt"
       rm CMakeCache.txt
    fi
    if [ -d CMakeFiles ]; then
       echo "Removing CMakeFiles"
       rm -rf CMakeFiles
    fi
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


