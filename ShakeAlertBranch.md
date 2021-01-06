
# ShakeAlert Branch
The ShakeAlertDev branch is a branch of Mike H's '2020' GFAST branch at https://github.com/pnsn

## Static vs dynamic linking

The original ShakeAlert specification called for static linking of all non-system libraries.  That is interpretted in this implementation as staticlally linking all libraries that are not installed via yum.

## Dependencies

GFAST was originally ported to c/c++ with many dependencies that cannot
be accomodated in the standard ShakeAlert environment. Here we describe
these dependencies and how they are handled in the ShakeAlert Branch.
A prime goal in this port is to make merging of future fixes and enhancements
in Ben Baker's master branch as seamles and easy as possible while
still operating within the standard ShakeAlert infrastructure.

Here we only consider the dependencies related to running GFAST in a Shake Alert environment.  Earthworm, ActiveMQ and other peripheral packages are assumed.

### cmake

No longer a dependency in ShakeAlert branch except to build some dependencies.

Baker's original branch (and many of its dependencies) use cmake in the build tree.  Most of the reasons for doing this do not apply to ShakeAlert (for example we don't intend to compile on Windows) and cmake is difficult to accomodate in the standard ShakeAlert build procedure.  The ShakeAlert branch implements a conventional Makefile tree rooted in the src/ directory.  Build-host specific paths are defined in src/Make.include.Linux which is "included" in all Makefiles.

Not required for GFAST, but required to compile [ISCL](#ISCL) and [compearth](#compearth) dependencies. 

### GCC 7.3

Not a dependency in ShakeAlert branch.

As mentioned in the README, compiler versions: gcc 4.8.5 vs gcc 7.3 gcc 4.8.5 does not recognize the "#pragma omp simd" directive so will generate a lot of warnings on compile. gcc 7.3 will use this directive to auto vectorize the loops, which may improve performance.  The code als makes heavy use of the "#pragma omp parallel" directive.

The "parallel" directive goes way back and is not a problem.  "simd" appears to have been introduced in OpenMP v4.0".

ShakeAlert servers currently (7/8/2020) have gcc 4.8.5 which implements OpenMP version 3.1.  OpenMP v4.0 was not included in gcc until version 4.9.  gcc v7.3 implements OpenMP v4.5.  It is possible to upgrade gcc on RHEL servers by enabling the SCL (Software Collections Libraries) but it it unclear whether this is not necessary in the ShakeAlert RHEL servers.  (unresolved)

This seems to only be a problem when compiling the [ISCL](#ISCL) dependency.

### Lapack

/usr/lib64/liblapack.so.3.4.2 is already part of standard ShakeAlert
install via lapack yum package. Corresponding lapacke.so also present.
Ubuntu needs lapack liblapack-dev and liblapacke-dev to link.
Centos7 needs lapack, lapack-devel, and lapacke. lapack-devel also install blas-devel as a dependency.

eew-uw-dev1 cannot find lapack-devel or lapacke which breaks the [ISCL](#ISCL) compile.  The stumbling block seems to be finding liblapacke.so or liblapack.so.  On Red Hat Enterprise Linux 7, this seems to be included in the lapack package.

### BLAS

/usr/lib/libblas is already installed on ShakeAlert machines via blas yum package. 

libcblas.so is not available via yum on RHEL dev machines. Manual install
is a bit tricky. Available on Ubuntu under that name
through the libatlas-base-dev. 

cblass functions are available on Centos/RHEL in the atlas-devel
repository. They do not include libcblas, so to link you need to 'yum
install atlas-devel' and replace '-lcblas' with '-L/usr/lib64/atlas
-lsatlas' or '-L/usr/lib64/atlas -ltatlas'

So on Centos you must install atlas and atlas-devel packages.

eew-uw-dev1 could not find atlas-devel package despite having epel and atlas packages installed.  /usr/include/openblas/cblas.h was provided by openblas-devel which was already installed.

Similarly, RHEL7 does not seem to provide a atlas-devel package which breaks the [ISCL](#ISCL) compile.

### Activemq-cpp

Already part of ShakeAlert standard install.

Need to define ACTIVEMQ in build tree pointing to package root.

Must be manually installed on Ubuntu but on Centos available via yum activemq-cpp-devel package.

Activemq-cpp also requires libcrypto, which in Ubuntu is in libssl-dev package. Centos provides this in several packages but the simplest is openssl-libs which is usually already installed.

### libapr

Already part of ShakeAlert standard install.  Needed by activemq-cpp packages.

For Ubuntu need apt install of libapr1 and libapr1-dev.  Link to libapr-1.

On Centos, apr-dev package is installed as a dependency in the activemq-cpp-devel package.

### iniparser

This is a tiny utiility from github to parse GFAST parameter files.
Will replace this functionality to use standard ShakeAlert versions.
iniparser.h loaded in core/cmt/readIni.c, core/properties/initialize.c,
core/scaling/pgd\_readIni.c, activeMQ/readIni.c.

This is a tiny little compile with no unusual dependencies.

### libxml2

/usr/lib64/libxml2.so.2 installed on dev systems. Many files reference
libxml/{*}.h files which are provided by the libxml2-devel package
on RHEL and libxml2-dev on Ubuntu.

Centos install of libxml2 and libxml2-devel install xz and zlib-devel as dependencies.

eew-uw-dev1 required yum install of libxml2-devel package.  libxml2 package already installed.

### HDF5

On RHEL family, yum install hdf5 and hdf5-devel.  On Ubuntu, you seem to need libhdf5-dev which will install libhdf5-cpp and hdf5-helpers among others as dependencies.

eew-uw-dev1 required yum install of hdf5-devel package.  hdf5 package already installed.

Requires linking with libz and libsz.

### ISCL

It appears that this library is used for timestamps (ISCL_time_timestamp()) and file manipulation (ICSL_os_path_isfile() and ISCL_os_path_isdir()).  Given that these are available in standard c. it seems likely that this dependency can be easily removed.

This is an ISTI product that would need to be added to extras (GFAST/third\_party
). Unfortunately, this package also adds several difficult dependencies not related to GFAST
functionality. Fortunately, most of these are not required for our SA compile (see below). 

Requires linking with libfftw3 or equivalent.

Still have not eliminated [cmake](#cmake) dependency.  To make this compile on eew-uw-dev1, required following to cmake script:
~~~~
/usr/bin/cmake $( dirname "$0" ) \
-DCMAKE_BUILD_TYPE=DEBUG \
-DCMAKE_INSTALL_PREFIX=./ \
-DISCL_USE_C11=FALSE \
-DISCL_USE_INTEL=FALSE \
-DCMAKE_C_FLAGS="-g3 -O2 -std=gnu11 -fopenmp -Wall -DTESTING" \
-DCMAKE_CXX_FLAGS="-g3 -O2 -Wall -DTESTING" \
-DISCL_USE_GEOLIB=FALSE \
-DCBLAS_INCLUDE_DIR="/usr/include/openblas" \
-DLAPACKE_INCLUDE_DIR="/usr/include/openblas" \
-DLAPACKE_LIBRARY="/usr/lib64/liblapacke.so.3" \
-DLAPACK_LIBRARY="/usr/lib64/liblapack.so.3" \
-DBLAS_LIBRARY="/usr/lib64/libblas.so.3" \
-DCBLAS_LIBRARY="/usr/lib64/atlas/libsatlas.so.3"
~~~~

#### - MKL

To remove this dependency, add set(ISCL\_USE\_INTEL OFF) to
CMakeLists.txt in iscl compile.  We will likely never use this in ShakeAlert.  Using MKL will require
CBLAS and LAPACKE _LIB and _INCL variables in Make.include.Linux to point to their mkl versions.  May be others.

#### - compearth

This is needed for moment tensor decompositions.  Currently only availabe from baker845 github repo.  Se Mike Haggarty's README for how to get and build.  Requires [cmake](#cmake) and one of the math library options.  Used in core, eewUtils, uw and xml.

eew-uw-dev1 [cmake](#cmake) build went fairly smoothly using following cmake script:

~~~~
#!/bin/bash
/usr/bin/cmake $( dirname "$0" ) -DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=./ \
-DCMAKE_C_FLAGS="-g3 -O2 -fopenmp" \
-DCOMPEARTH_USE_MKL=FALSE \
-DCOMPEARTH_BUILD_PYTHON_MODULE=OFF \
-DCBLAS_INCLUDE_DIR="/usr/include/openblas" \
-DCBLAS_LIBRARY="/usr/lib64/atlas/libsatlas.so.3" \
-DLAPACKE_INCLUDE_DIR="/usr/include/openblas" 
~~~~

#### - FFTw

Documentation suggests Fourier transforms are not computed in GFAST,
so this is an artifact of including ISCL. We may want to customize ISCL or eliminate ISCL all together to remove dependency in the future.

This is available on Ubuntu via libftw3-bin and libftw3-dev

on Centos, install fftw package which installs libfftw3.so and many other related.

eew-uw-dev1 required fftw package install.  fftw-devel was also needed to compile iscl dependency.

#### - LibGeographic

Used in unit tests. I suspect that functionality may need to be re-implemented
in current library equivalents. Could not find any mention of this
in code, so may be obsolete. Not part of ShakeAlert install.  Seems to compile without it but you need to specify -DISCL_USE_GEOLIB=FALSE in [ISCL](#ISCL) compile.  Looks like this is a dependency of a dependency, not of the main code.

#### - libcurl

Not a dependency for ShakeAlert install.

Used only in gfast2web in uw directory.  Ubuntu libcurl4-gnutls-dev.

## Additional ShakeAlert dependenciesx

The following must be added to make GFAST conform to ShakeAlert standards

## Merging with the latest master branch

As of 12/1/2020 the master branch for GFAST is pnsn.github/2020.  To merge the latest changes in the master branch into the SAdev branch:

1. > git checkout 2020
2. > git pull [ remote-name 2020 ] 
3. > git checkout SAdev
4. > git pull [ remote-name SAdev ]
5. > git merge 2020

### make

This requires Makefiles in all relevant directories.

All the machine specific paths and variables are defined in the file Make.include.Linux  The default version in the git checkout is set up for a generic ShakeAlert machine (or how one might eventually be set up).  If the environment variable $MAKEHOST is defined at the time make is invoked, make will instead include a host-specific file with the name 'Make.include.$MAKEHOST'.  This allows development compilation on non-ShakeAlert machines.

### makedepend

Other than having the executable available, this does not seem to
require Makefile modifications.

# To do

- Find out why activemq not seeing events.
- Should convert to use dmlib
- Documentation
- Metadata reader needs to be converted to ShakeAlert file format. Should 
- ShakeAlertConsumer should be revamped to allow asynchronous read loop.

