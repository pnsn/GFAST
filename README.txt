
Introduction:

Dependencies:

(a)  For testing the lat/lon to UTM conversions you must install geographiclib:
       http://geographiclib.sourceforge.net/
     Additionally, if static linking with libiscl you will need this library.

(b)  The interquartile computation requires GNU Scientific library.
       https://www.gnu.org/software/gsl/
     which is implemented in ISCL.

(c)  If static linking to ISCL you will need fftw:
       http://www.fftw.org/

(d)  You must have BLAS and LAPACK as well as the C extensions (cblas and 
     LAPACKe) with the header files.  These are very likely located in
       /usr/lib/
     or
       /usr/lib64(32)
     depending on your OS (e.g. Ubuntu or CentOS respectively).
     The accompanying header files are likely in /usr/include.

     Otherwise, all files are available from:
       http://www.netlib.org/lapack/
    
     Alternatively, and preferably, one can achieve this functionality with
     Intel's MKL which is now freely available at:
       https://software.intel.com/sites/campaigns/nest/
     The vendor version of BLAS/LAPACK in MKL results in a non-negligible 
     performance gain - particularly when static linking and allowing
     threading - on Intel based hardware.

(e)  Since many core computational functions are derived from scipy/matlab
     functions they have already been cloned in ISTI's ISCL library.  
     Additionally, GFAST makes use of ISCL's logging capabilities.
     This library can be obtained for non-commercial activity at:
       https://github.com/bakerb845/libiscl 
     If static linking you will also need libfftw.

     A note to the warning centers - ISCL is derivative work from the 
     tsunami IT modernization project's libseismic.  ISCL however is
     in many ways superior to libseismic's src/utils functions and should
     be preferred since I am no longer actively maintaining libseismic.

(f)  To parse the ini file you must install iniparser:
       https://github.com/ndevilla/iniparser

(g)  To use the Python interfaces you must have cython
       http://cython.org/
     which likely will be available through a package manager.

(h)  For archival purposes you must have HDF5.  HDF5 has a subdependency
     zlib.  
       http://www.zlib.net
       https://www.hdfgroup.org/HDF5/
     I strongly recommend building HDF5 from source because stale header
     files left over by sloppy package managers have been positively
     correlated with increased frustration levels in most test subjects.

     zlib is also supposedly available in Intel's Performance Primitives
     package:
        https://software.intel.com/sites/campaigns/nest/
     which may be of interest if one plans to use ISCL more extensively. 

(i)  XML and QuakeML is becoming a popular framework for disseminating
     results.  For XML interfacing we use XML2 which is likely available
     with GCC but, if not, is available at:
      http://xmlsoft.org/

Install:

(a)  First descend into the make_inc directory and choose an appropriate 
     Makefile.inc template.  If one does not exist then it is easy enough
     to create one from scratch.  Copy the desired, or custom, template to
     the project root directory: For example, if the GNU template fits your
     needs as is then cp make_gnu.inc ../Makefile.inc

(b)  Next, build the shared GFAST C libraries by descending into the 
     src directory and typing make.   

(c)  As an example, one can also Cython the shared library so that the C
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

Directories:

(a)  doc - documentation for algorithms

(b)  include - include files for GFAST

(c)  legacy - the original Python GFAST

(d)  src - contains the GFAST source code
           core -> contains the core GFAST computational routines
           eew -> contains functionality for integrating GFAST into
                  the shakeAlert/earthquake early warning framework
           ew -> contains functionality for accessing live data from
                 Earthworm (http://www.earthwormcentral.org/)

(e)  make_inc - template makefile include files

(f)  cython - (C)Python interface to libgfast.so.  After successfully
              building the cython wrappers one can use them as if the
              wrappers were Python modules. 

(h)  unit_tests - regression testing for libsrc

Examples:

(a)  No exercises in futility yet... 

