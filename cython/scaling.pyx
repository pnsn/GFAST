#!/usr/bin/cython
import numpy as np
cimport numpy as np

np.import_array()

# cdefine the signature of the c function
cdef extern from "gfast.h":
    int GFAST_scaling_PGD__setForwardModel(int n, int verbose,
                                           double B, double C,
                                           double *r,
                                           double *G)

    int GFAST_scaling_PGD__setRHS(int n, int verbose,
                                  double dist_tol, double dist_def,
                                  double A, double *d,
                                  double *b)

def setForwardModel(np.ndarray[double, ndim=1, mode="c"] r not None,
                    B = 1.500,
                    C =-0.214,
                    verbose = 0):
    """

      Parameters
      ----------
      r : array_like
          hypocentral distances (km) from the source to the n sites
      B : float
          linear shift in G = B + C*log10(r)
      C : float
          scale factor in G = B + C*log10(r)
      verobse : int
          controls verbosity.  0 will report on errors only.

      Returns
      -------
      G : array_like
          forward modeling matrix for PGD [n x 1].  Note that G is in 
          column major (Fortran) format
      ierr : int
             0 indicates success
    """ 
    ierr = 0
    n = r.shape[0]
    if (n < 1):
        print "setRHS: Error no sites!"
        return None, ierr
    G = np.zeros(n, dtype='float64')
    ierr = GFAST_scaling_PGD__setForwardModel(n, verbose,
                                              B, C,
                                              <double *> np.PyArray_DATA(r),
                                              <double *> np.PyArray_DATA(G))
    if (ierr != 0):
        print "setForwardModel: Error setting forward model"
        return None, ierr
    G.reshape([n,1]) #, order='F')
    return G, ierr
#end setForwardModel

def setRHS(np.ndarray[double, ndim=1, mode="c"] d not None,
           dist_tol = 6.0,
           dist_def = 0.01,
           A =-6.687,
           verbose = 0):
    """
    Sets the RHS for the PGD scaling s.t. b = log_10(d) - A where
    A is a scalar shift and d the distance at each station

    Parameters
    ----------
    d : array_like
        max distance (cm) at each of the n sites

    dist_tol : float
        distance tolerance - if d is less than this
        then it will be set to a default value (cm)

    dist_def : float
        distance default value (cm)

    A : float
        shift so that b = log10(d) - A
 
    verbose : int
        controls the verbosity.  0 will report on errors only.

    Returns
    -------
    b : array_like
        right hand side in Gm = b
    ierr : int
           0 indicate success
    """
    ierr = 0
    n = d.shape[0]
    if (n < 1):
        print "setRHS: Error no sites!"
        return None, ierr 
    b = np.zeros(n, dtype='float64')
    ierr = GFAST_scaling_PGD__setRHS(n, verbose,
                                     dist_tol, dist_def,
                                     A,
                                     <double *> np.PyArray_DATA(d),
                                     <double *> np.PyArray_DATA(b))
    if (ierr != 0):
        print "setRHS: Error setting the RHS!"
        b = None
    return b, ierr
# end setRHS 
 
    
