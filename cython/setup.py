#!/usr/bin/python
from distutils.core import setup, Extension
import numpy
from Cython.Distutils import build_ext

gfast_incdir = "/home/bakerb3/C/GFAST/include"
libdirs = ['/usr/lib/',
           '/home/bakerb3/C/iniparser',
           '/home/bakerb3/C/GFAST/src']
libraries = ['gfast', 'lapacke', 'lapack', 'cblas', 'blas', 'iniparser']

setup(
    cmdclass = {'build_ext': build_ext},
    ext_modules = [Extension("scaling",
                   sources = ["scaling.pyx"],
                   library_dirs = libdirs,
                   libraries = libraries,
                   include_dirs = [numpy.get_include(),
                                   gfast_incdir])]
)
