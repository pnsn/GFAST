
For installation instructions RTFM in doc/manual.pdf

Introduction:

Read the manual.

Dependencies (verify with manual):

(a)  You must have cmake >= 2.6 for generation of makefiles

(b)  The low level math routines and logging reside in ISTI's computing library:
       https://github.com/bakerb845/libiscl
     which, at the present time, must be built from source (apache 2 license).

(c)  For testing the lat/lon to UTM conversions you must install geographiclib:
       http://geographiclib.sourceforge.net/
     Additionally, if static linking with libiscl you will need this library
     (mit license).

(d)  To parse the ini file you must install iniparser:
       https://github.com/ndevilla/iniparser
     (mit license)

(e)  It is recommended you obtain Intel's Math Kernel Library (MKL):
       https://software.intel.com/sites/campaigns/nest/
     as it will provide a non-negligible increase in performance.  You'll
     have to agree to Intel's license.

     Otherwise, you will need LAPACK, LAPACKE, BLAS, AND CBLAS which is likely
     available with a package manager and resides in /usr/lib(64) or, if
     absolutely necessary, can be built from source:
       http://www.netlib.org/lapack/
     (modified BSD)
     If static linking to ISCL and do not have MKL you theoretically can use
     FFTw.
       http://www.fftw.org/
     (GPL license - you really should use MKL)

(f)  For archival purposes you must have HDF5.  HDF5 has a subdependency
     zlib, which will expose itself should you attempt to statically,
     link.  Both are available at the following sites.
       http://www.zlib.net
       https://www.hdfgroup.org/HDF5/
     I encourage either building HDF5 from source or using a package
     manager.  Never in between.  If you build with an MPI enabled HDF5
     you'll need to either use the shared HDF5 libraries or specify mpicc
     as your C compiler.  (zlib and hdf5 have permissive licenses)

     zlib is also supposedly available in Intel's Performance Primitives
     package:
       https://software.intel.com/sites/campaigns/nest/
     which may be of interest if one plans to use ISCL more extensively.

(g)  XML and QuakeML are becoming a popular framework for disseminating
     results.  For XML interfacing we use XML2 which is likely available
     with GCC but, if not, is available at:
       http://xmlsoft.org/
     (mit license)

--------------------------------------------------------------------------------
-                                   optional                                   -
--------------------------------------------------------------------------------

(h)  The earthquake early warning community triggers on activeMQ messages.  
     To use activeMQ you'll need libcrtypo and libssl as well as the C++
     activeMQ package:
       http://activemq.apache.org/cms/download.html
     (apache 2 license)

(i)  Earthworm'ers the data will be coming in via geojson2ew.  To get this
     to compile you'll need, obviously, earthworm
       http://earthworm.isti.com/trac/earthworm/
     as well as rabbitmq-c 
       https://github.com/alanxz/rabbitmq-c
     and
       https://github.com/akheron/jansson
     (rabbit-c is mit licensed and jansson has a permissive license)
 
(j)  To use the Python interfaces you must have cython
       http://cython.org/
     which likely will be available through a package manager.

Install:

(a)  Descend into source, type cmake ., and let the pain begin.  I don't
     even bother with the ccmake and instead directly edit the CMakeLists.txt.

(b)  Be sure to correctly set your LD_LIBRARY_PATH's.  I actually can't
     static link all the libraries (though you might go into src/CMakeLists.txt,
     look at the target_link_library section, and change things that say
     _shared to _static)

(c)  Type make

(d)  As an example, one can also Cython the shared library so that the C
     functions can be used directly from Python.  Hopefully, the compile.sh
     bash script is sufficient.  Additionally, if you are installing to 
     non-system standard directories you will need to set/modify your 
     LD_LIBRARY_PATH environment variable.

     A word of caution: the calling sequences have been altered for C's 
     benefit so the Python legacy source will likely be unable to call any 
     GFAST function directly without modification to the call sequence.  
     I do not recommend using this functionality until the src has
     stabilized.  This is because the Python to C interfaces are low
     priority for development.

Directories (this is valid - TODO: copy to manual):

(a)  doc - documentation for algorithms

(b)  legacy - the original Python GFAST

(c)  src
     (i)    include - contains the GFAST include files
     (ii)   cmopad - contains the functions for moment tensor conversions
                     and decompositions (MoPaD Moment tensor Plotting and
                     Decomposition - A tool for graphical and numerical
                     analysis of seismic moment tensors) [library]
     (iii)  core - core modeling and inversion utilities [library] 
     (iv)   eewUtils - earthquake early warning utilities and drivers
                       with EEW application logic - a good place to start
                       if you'd like to learn how to use all the other
                       libraries [application]
     (v)    events - in real-time applications this handles the bookkeeping
                     for GFAST and keeps track of the number of events to
                     process [probably should move to core]
     (vi)   xml - for writing (and reading?) XML for shakeAlert and
                  quakeML for the CMT inversion [library]
     (vii)  hdf5 - for archiving results [library]
     (viii) activeMQ - for earthquake early warning this is the requisite
                       functions for incoming shakeAlert core messages (on
                       which GFAST EEW triggers) and for sending the finite
                       fault and PGD shakeAlert XML messages [library]
     (iix)  traceBuffer - handles the getting of data into the program.
                          in here there reside the earthworm hooks.  though
                          if not compiling for a real-time application this
                          can be ignored.

(d)  cython - (C)Python interface to libgfast.so.  After successfully
              building the cython wrappers one can use them as if the
              wrappers were Python modules. 

(e)  unit_tests - regression testing for the core library. 

Examples:

(a)  No exercises in futility yet... 

