
Introduction:

Install:

(a)  First descend into the make_inc directory and choose an appropriate 
     Makefile.inc template.  If one does not exist then it is easy enough
     to create one from scratch.  Copy the desired, or custom, template to
     the project root directory: For example, if the GNU template fits your
     needs as is then cp make_gnu.inc ../Makefile.inc

(b)  Next, build the shared GFAST C libraries by descending into the 
     c_src directory and typing make.   

(c)  As an example, one can also SWIG the shared library so that the C
     functions can be used directly from Python.  Hopefully, the compile.sh
     bash script is sufficient.  Additionally, if you are installing to 
     non-system standard directories you will need to set/modify your 
     PYTHON_PATH and LD_LIBRARY_PATH environment variables.  Furthermore, 
     the calling sequences have been altered for C's benefit so the Python 
     legacy source will likely be unable to call any GFAST module directly 
     without modifcation.  I do not recommend using this functionality until
     the c_src is stabilized as the interfaces will likely not reflect the
     changes in the C calling sequences.

Examples:

(a)  You mean test software prior to use?  That's a novel idea. 

