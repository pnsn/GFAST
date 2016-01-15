#!/usr/bin/python
import os
from distutils.core import setup, Extension

name = 'gfast'
version = '0.1'
author = "Ben Baker"
description = """GFAST: Fast geodetic computations for earthquake early warning"""
swig_cmd = 'swig -python %s.i'%name 
sources = ['gfast.i']
libraries = ['lapack','blas']
libdirs = ['/usr/lib']
#libraries = ['mseed','lapack','blas','sacio']
#libdirs = ['/usr/lib/','/home/bakerb3/C/libmseed','/home/bakerb3/C/sac-101.6a/lib']

setup(name=name,
      version=version,
      author=author,
      description=description,
      ext_modules = [Extension('_' + name, sources, include_dirs=[os.curdir,os.pardir], 
                     library_dirs=libdirs, libraries=libraries)]
      ) 

