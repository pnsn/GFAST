# Introduction 

This is the source code for Geodetic First Approximation of Size and Timing (GFAST) geodetic earthquake early warning module.  For more detailed instructions check the doc/manual.pdf

## Recent updates

2023/03/20 CWU: Sync most recent changes from ShakeAlert GFAST version to the PNSN repo (GFAST v1.2.3-beta)  
~2021/10 CWU: merge 2020 and SAdev branches into one branch to use going forward, called "development"  
2020/05/27 MTH: Working on new branch (=2020) with cleaned up dependencies and build instructions.

# Directories

1. doc contains the GFAST user's manual.
2. include contains the GFAST C include files.
3. legacy is the original Python source code.
4. src contains the source code.
  + src/activeMQ contains C++ readers/writers and C interfaes for using ActiveMQ.
  + src/core contains the core GFAST computations.
  + src/dmlib contains functionality making use of some ShakeAlert libraries, primarily related to ActiveMQ connections and xml encoding/decoding. Access to the ShakeAlert code repository would be necessary to build using these codes.
  + src/eewUtils contains application specific functions for performing the earthquake early warning tasks.
  + src/hdf5 contains the HDF5 interfaces for generating a self-describing archive or play-by-play of GFAST.
  + src/traceBuffer contains routines for reading an Earthworm ring and converting to a GFAST specific buffer.
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
gcc 4.8.5 does not recognize the *#pragma omp simp* directive so will generate a lot of warnings on compile.
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

        GeographicLib requires cmake >= 3.1.0
        >sudo yum install cmake3

        Download GeographicLib from: https://geographiclib.sourceforge.io/html/install.html
        >cd GeographicLib-1.50.1
        >./configure 
        >mkdir BUILD
        >cd BUILD
        >cmake3 ..      // If you want to change where it's installed do
                        // >cmake3 -D CMAKE_INSTALL_PREFIX=/tmp/geographic ..
        >make
        >make test
        >sudo make install   // By default will install .h files into /usr/local/include/GeographicLib
                             //   and lib into /usr/local/lib/libGeographic.so.19.0.1

        Clone and build ISCL
        >git clone https://gitlab.isti.com/bbaker/iscl
        Edit zbuild_gcc.sh to select intel (MKL/IPP) if desired:
        USE_INTEL=true (be sure to edit your paths to the intel MKL/IPP libs)
        // MTH: Need to copy zbuild_gcc.sh into the ISCL repo
        >zbuild_gcc.sh    // This will run cmake and update CMakeCache.txt, etc.
        >make              // will build lib/libiscl_shared.so and lib/libiscl_static.a


7. [compearth/mtbeach](https://github.com/carltape/mtbeach.git) for the moment tensor decompositions.  Note, this was in Ben Baker's [compearth repository](https://github.com/bakerb845/compearth), but was merged back into Carl Tape's repository, and the moment tensor part was renames mtbeach.  Also, you'll need to descend into c_src. 

        Clone and build
        >git clone https://github.com/carltape/mtbeach.git
        >cd mtbeach/c_src
        Edit zbuild_gcc.sh to select intel (MKL/IPP) if desired:
        USE_INTEL=false
        // MTH: Need to add this zbuild_gcc.sh into the mtbeach/c_src repo!
        >zbuild_gcc.sh      // This will run cmake and update CMakeCache.txt, etc.
        >make               // will build mtbeach/c_src/lib/libcompearth_shared.so and mtbeach/c_src/lib/libcompearth_static.a
        
8. [Earthworm](http://earthworm.isti.com/trac/earthworm/) v7.8 or greater with geojson2ew.  geojson2ew will require [rabbitmq](https://github.com/alanxz/rabbitmq-c) and [Jansson](https://github.com/akheron/jansson).

    Note that in order to build the gfast shared lib, we need to build an earthworm shared lib to link to.
    Currently [June 2020] earthworm is not configured to do this by default.
    An easy solution is given below - we compile the object code in the static libs using the -fPIC flag.
    Then the "static" libs can be used in both static/shared mode.
    
        >svn checkout svn://svn.isti.com/earthworm/trunk earthworm_svn
        // Assuming this resides in /opt/earthworm:
        >cd /opt/earthworm/earthworm_svn
        >vi src/libsrc/makefile.unix
        # Add this line at top of file:
        GLOBALFLAGS += -fPIC
        
        # Note where the resulting libs will be put:
        L = $(EW_HOME)/$(EW_VERSION)/lib
        
        # When you source the EW environment (which you must do before trying to build the lib):
        >source environment/ew_linux.bash
        
        # It will set the envs:
        EW_HOME=/opt/earthworm
        EW_VERSION=earthworm_7.10
        
        Therefore, by default the EW libs that GFAST needs will be put in:
        /opt/earthworm/earthworm_7.10/lib/{lib_mt.a, lib_util.a}
        Note: versions 7.9 and before did not have lib_util.a, but all that is needed is $(EW_HOME)/lib/swap.o (see Make.include.Linux for more info)
        
        Alternatively, you can override the EW_VERSION after sourcing the env script:
        >export EW_VERSION=earthworm_svn
        
        So that the libs will be put in:
        /opt/earthworm/earthworm_svn/lib

        # Now build the libs: 
        >cd src
        >make unix 

        # Confirm lib/libew_mt.a and lib/libew_util.a were created (into lib path discussed above)

9. [ActiveMQ] This is now OPTIONAL. Set use_AMQ=false in zbuild.sh
   before running it. Then run:>make and you should need to install
   either 9a. the C++ lib nor run the java jar in 9b.

    9a. [ActiveMQ](http://activemq.apache.org/) The C++ portion.  These will require other things that you likely already have like libssl, libcrypto, and the Apache runtime library.

    Note: [MTH] I haven't found the APR to be necessary
    Note: For building GFAST, the instructions here for downloading and
          installing activemq-cpp are sufficient.
          However, as soon as gfast_eew starts, it will try to connect
          to an activemq broker at the url and port specified in gfast.props.


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
    
    9b. [ActiveMQ](http://activemq.apache.org/) The Java portion.

    For building GFAST, the instructions above for downloading and installing activemq-cpp are sufficient.
    However, as soon as gfast_eew starts, it will try to connect to an activemq broker at the url and port specified in gfast.props

    In practice, this AMQ section of gfast.props will presumably point to the broker at PNSN.
    However, since gfast_eew must be able to connect to an activemq broker, for the purpose of local testing and development,
    I'll describe here how to set up and run a local broker.

    Download the java jar from:
    https://activemq.apache.org/components/classic/download/

    Make sure ports in conf/activemq.xml match those specified in gfast.props.
    eg., 

            [ActiveMQ]
            # ActiveMQ hostname to access ElarmS messages
            host=localhost
            # ActiveMQ port to access ElarmS messages (61620)
            port=61616
            
    Start up the broker *before* starting gfast_eew:

            >java -jar activemq-all-5.14.3.jar start
    
10. [Plog](https://github.com/SergiusTheBest/plog) is a logging library. It currently only works within the ShakeAlert build system (using make: see below), although perhaps only minor modifications would be needed to add it to the cmake system. Simply install the code somewhere and add plog/include to the project include path.

## Configure, Build, Unit Test 

### Using cmake
The cmake system is primarily designed for use outside of ShakeAlert (i.e. not using ShakeAlert-related libraries). Look in zbuild-gcc.sh for the paths to the various dependencies.
If you downloaded/installed them as outlined above, then it's
possible you only need to set PKG_DIR to point to whatever
directory you put compearth, iniparser, iscl.
In addition, the mods inside CMakeModules will attempt to find
many of the needed dependencies on your system automatically.

    # Run it // This will update CMakeCache.txt, etc.
    >zbuild-gcc.sh

    # Make it
    >cd build
    >make
    By default, the gfast_eew binary will be put in the bin/ directory

    # Run the unit tests:
    >cd ../unit_tests
    >./xcoreTests

    main: Beginning core tests...
    readCoreInfo_test: Success!
    [INFO] coord_test_ll2utm: Success!

    main: Beginning matrix generation tests...
    [INFO] cmt_greens_test: Success!
    [INFO] ff_greens_test: Success!
    [INFO] ff_regularizer_test: Success!

    main: Beginning inversions tests...
    [DEBUG] core_scaling_pgd_depthGridSearch: Beginning search on depths...
    [DEBUG] core_scaling_pgd_depthGridSearch: Grid-search time: 0.097891 (s)
    [INFO] pgd_inversion_test: Success!
    [INFO] pgd_inversion_test2: Success!
    [INFO] cmt_inversion_test: Success!
    [INFO] ff_inversion_test: Success!
    main: All tests passed

### Using make
The make system is designed primarily for use with ShakeAlert libraries, usage without them (via compile flags) is not fully supported as of March 2023. 

    # At the top level, make with:
    >make all

    # Or, alternatively
    >cd src
    >make

    # The gfast_eew binary will be at src/gfast_eew

## Ridgecrest example 

Please read: test_data/ridgecrest/RIDGECREST.md
for instructions on how to run the Ridgecrest example.


