
Introduction:

Dependencies:

(a)  You must have BLAS and LAPACK as well as the C extensions (cblas and 
     LAPACKe) with the header files.  These are very likely located in
       /usr/lib/
     or 
       /usr/lib64(32)
     depending on your OS (e.g. Ubuntu or CentOS respectively).
     The accompanying header files are likely in /usr/include.  Otherwise,
     all files are available from:
       http://www.netlib.org/lapack/

(b)  To parse the ini file you must install iniparser:
       https://github.com/ndevilla/iniparser

(c)  For testing the lat/lon to UTM conversions you must install geographiclib:
       http://geographiclib.sourceforge.net/

Install:

(a)  First descend into the make_inc directory and choose an appropriate 
     Makefile.inc template.  If one does not exist then it is easy enough
     to create one from scratch.  Copy the desired, or custom, template to
     the project root directory: For example, if the GNU template fits your
     needs as is then cp make_gnu.inc ../Makefile.inc

(b)  Next, build the shared GFAST C libraries by descending into the 
     src directory and typing make.   

(c)  As an example, one can also SWIG the shared library so that the C
     functions can be used directly from Python.  Hopefully, the compile.sh
     bash script is sufficient.  Additionally, if you are installing to 
     non-system standard directories you will need to set/modify your 
     LD_LIBRARY_PATH environment variable.

     A word of caution: the calling sequences have been altered for C's 
     benefit so the Python legacy source will likely be unable to call any 
     GFASTc module directly without modification to the call sequence.  
     I do not recommend using this functionality until the src has
     stabilized.  This is because the Python to C interfaces are low
     priority for development at the project onset.

Directories:

(a)  doc - documentation for algorithms

(b)  include - include files for GFAST

(c)  legacy - the original Python GFAST

(d)  libsrc - contains the libgfast source code

(e)  make_inc - template makefile include files

(f)  modules - contains the modules for gfast and subsequent modules
               such as finite fault, CMT, and PGD

(g)  python - swig/ctypes interface to libgfast.so

(h)  unit_tests - regression testing for libsrc

Examples:

(a)  No exercises in futility yet... 

