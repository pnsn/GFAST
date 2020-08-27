# ShakeAlert Branch

The ShakeAlertDev branch is a branch of Mike H's '2020' GFAST branch at https://github.com/pnsn

## Dependencies

GFAST was originally ported to c/c++ with many dependencies that cannot
be accomodated in the standard ShakeAlert environment. Here we describe
these dependencies and how they are handled in the ShakeAlert Branch.
A prime goal in this port is to make merging of future fixes and enhancements
in Ben Baker's master branch as seamles and easy as possible while
still operating within the standard ShakeAlert infrastructure.

Here we only consider the dependencies related to running GFAST in a Shake Alert environment.  Earthworm, ActiveMQ and other peripheral packages are assumed.

### cmake

Baker's original branch (and many of its dependencies) use cmake in the build tree.  Most of the reasons for doing this do not apply to ShakeAlert (for example we don't intend to compile on Windows) and cmake is difficult to accomodate in the standard ShakeAlert build procedure.  The goal is to create a parallel make system on this branch

### GCC 7.3

As mentioned in the README, compiler versions: gcc 4.8.5 vs gcc 7.3 gcc 4.8.5 does not recognize the "#pragma omp simd" directive so will generate a lot of warnings on compile. gcc 7.3 will use this directive to auto vectorize the loops, which may improve performance.  The code als makes heavy use of the "#pragma omp parallel" directive.

The "parallel" directive goes way back and is not a problem.  "simd" appears to have been introduced in OpenMP v4.0".

ShakeAlert servers currently (7/8/2020) have gcc 4.8.5 which implements OpenMP version 3.1.  OpenMP v4.0 was not included in gcc until version 4.9.  gcc v7.3 implements OpenMP v4.5.  It is possible to upgrade gcc on RHEL servers by enabling the SCL (Software Collections Libraries) but it it unclear whether this will be an option on ShakeAlert RHEL servers.  (unresolved)

### libstdc++11

There are string function calls in the activemqcpp routines that require c++ version 11 in the original code.  This is not an option on ShakeAlert and really kind of a gratuitous requirement.
(unresolved)

### Lapack

/usr/lib64/liblapack.so.3.4.2 is already part of standard ShakeAlert
install via lapack yum package. Corresponding lapacke.so also present.
Ubuntu needs both liblapack-dev and liblapacke-dev to link.

### BLAS

/usr/lib/libblas is already installed on ShakeAlert machines via blas yum package. 

libcblas.so does not seem to be available on RHEL dev machines. Manual install
is a bit tricky. Appears to be available on Ubuntu under that name
through the libatlas-base-dev. 

Centos/RHEL seem to have put the cblas-provided-functions in the atlas-devel
repository. They do not include libcblas, so to link you need to 'yum
install atlas-devel' and replace '-lcblas' with '-L/usr/lib64/atlas
-lsatlas' or '-L/usr/lib64/atlas -ltatlas'

### Activemq-cpp

Already part of ShakeAlert standard install.

Need to define ACTIVEMQ in build tree pointing to package root.

Activemq-cpp also requires libcrypto, which in Ubuntu is in libssl-dev package.

### libapr

Already part of ShakeAlert standard install.

For Ubuntu need apt install of libapr1 and libapr1-dev.

### iniparser

This is a tiny utiility from github to parse GFAST parameter files.
Will replace this functionality to use standard ShakeAlert versions.
iniparser.h loaded in core/cmt/readIni.c, core/properties/initialize.c,
core/scaling/pgd\_readIni.c, activeMQ/readIni.c 

### libxml2

/usr/lib64/libxml2.so.2 installed on dev systems. Many files reference
libxml/{*}.h files which are provided by the libxml2-devel package
on RHEL and libxml2-dev on Ubuntu.

### HDF5

Does not seem to be referenced on its own, and initial compilation
tests do not require it. hdf5.h loaded in gfast\_hdf5.h, gfast\_traceBuffer.h 

On RHEL family, yum install hdf5 and hdf5-devel.  On Ubuntu, you seem to need libhdf5-dev which will install libhdf5-cpp and hdf5-helpers among others as dependencies.

Requires linking with libz and libsz.

### ISCL

This is an ISTI product that may need to be added to extras (GFAST/third\_party
?). Unfortunately, this adds several difficult dependencies not related to GFAST
functionality. May need some tweaks. ISCL headers needed by cmt, ff,
log, properties, scaling, xml/quakeML, data. 

Added ISCL dependencies include:

#### - MKL

To remove this dependency, add set(ISCL\_USE\_INTEL OFF) to
CMakeLists.txt in iscl compile

#### - compearth

This is needed for moment tensor decompositions.  Currently only availabe from baker845 github repo.  Se Mike Haggarty's README for how to get and build.  Requires cmake and one of the math library options.  Used in core, eewUtils, uw and xml.

#### - FFTw

Documentation suggests Fourier transforms are not computed in GFAST,
so this is an artifact of including ISCL. We may want to customize this to remove dependency.

This is available on Ubuntu via libftw3-bin and libftw3-dev

#### - LibGeographic

Used in unit tests. I suspect that functionality may need to be re-implemented
in current library equivalents. Could not find any mention of this
in code, so may be obsolete. Not part of ShakeAlert install.

#### - libcurl

Used only in gfast2web in uw directory.  Ubuntu libcurl4-gnutls-dev.

## Additional ShakeAlert dependencies

The following must be added to make GFAST conform to ShakeAlert standards

### make

This requires Makefiles in all relevant directories

### makedepend

Other than having the executable available, this does not seem to
require Makefile modifications.

# To do

std::auto\_ptr in activeMQ classes is deprecated. This is an activemqcpp
thing and may just turn off warning.

