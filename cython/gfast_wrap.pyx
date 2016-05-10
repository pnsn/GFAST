#!/usr/bin/cython
import numpy as np
cimport numpy as np
#from cpython cimport bool
from libcpp cimport bool

np.import_array()

cdef struct GFAST_pgd_props_struct:
    double window_vel
    double dist_tol
    double dist_def
    int min_sites
    int verbose
    int utm_zone
    int ngridSearch_deps
    bool lremove_disp0

cdef struct GFAST_shakeAlert_struct:
    char eventid[128]
    double lat
    double lon
    double dep
    double mag
    double time
 
# cdefine the signature of the c function
cdef extern from "gfast.h":
 int GFAST_scaling_PGD__setForwardModel(int n, int verbose,
                                        double B, double C,
                                        const double *r,
                                        double *G)

 int GFAST_scaling_PGD__setRHS(int n, int verbose,
                               double dist_tol, double dist_def,
                               double A,
                               const double *d,
                               double *b)
 #################################### cmt ######################################
 int GFAST_CMT__decomposeMomentTensor(const double *M, 
                                      double *DC_pct,
                                      double *Mw,
                                      double *strike1, double *strike2,
                                      double *dip1, double *dip2,
                                      double *rake1, double *rake2)
 int GFAST_CMT__setRHS(int n, int verbose,
                       const double *nAvg,
                       const double *eAvg,
                       const double *uAvg,
                       double *U)
 int GFAST_CMT__setForwardModel__deviatoric(int l1, 
                                            const double *x1, 
                                            const double *y1, 
                                            const double *z1, 
                                            double *G)
 
class Scaling:
 def setForwardModel(self,
                     np.ndarray[double, ndim=1, mode="c"] r not None,
                     B = 1.500,
                     C =-0.214,
                     verbose = 0):
    """
    Sets the forward model, G, for the PGD scaling s.t. Gm = U

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
    G = G.reshape([n,1]) #, order='F')
    return G, ierr
 #end setForwardModel

 def setRHS(self, 
            np.ndarray[double, ndim=1, mode="c"] d not None,
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

################################################################################
#                                       CMT                                    #
################################################################################ 
class CMT:
 def decompose(self,
               np.ndarray[double, ndim=1, mode="c"] M not None):
    """ 
    Decomposes a moment tensor into a moment magnitude, percent double 
    couple, and it's two nodal planes

    Parameters
    ----------
    M : array_like
        Moment tensor in NED coordinates packed
        {mxx, myy, mzz, mxy, mxz, myz}

    Returns
    -------
    DC_pct : float
             double couple percentage
    Mw : float
         scalar moment 
    strike1 : float
              strike angle of first nodal plane (degrees)
    strike2 : float
              strike angle of second nodal plane (degrees)
    dip1 : float
           dip angle of first nodal plane (degrees)
    dip2 : float
           dip angle of second nodal plane (degrees)
    rake1 : float
            rake angle of first nodal plane (degrees)
    rake2 : float
            rake angle of second nodal plane (degrees)
    ierr : int
           0 indicates success 
    """
    ierr = 0 
    DC_pct = 0.0 
    Mw = 0.0 
    strike1 = 0.0 
    strike2 = 0.0 
    dip1 = 0.0 
    dip2 = 0.0 
    rake1 = 0.0 
    rake2 = 0.0 
    n = M.shape[0]
    if (n != 6): 
        print "decompose: Error M must be length 6!"
        ierr = 1
    else:
        ierr = GFAST_CMT__decomposeMomentTensor(<double *> np.PyArray_DATA(M),
                                                &DC_pct,
                                                &Mw,
                                                &strike1, &strike2,
                                                &dip1, &dip2,
                                                &rake1, &rake2)
        if (ierr != 0):
            print "decompose: Error in moment tensor decomposition"
    return DC_pct, Mw, strike1, strike2, dip1, dip2, rake1, rake2, ierr
 # end decompose

 def setRHS(self,
            np.ndarray[double, ndim=1, mode="c"] nAvg not None,
            np.ndarray[double, ndim=1, mode="c"] eAvg not None,
            np.ndarray[double, ndim=1, mode="c"] uAvg not None,
            verbose = 0):
    """
    Sets the right hand side in the CMT inversion such that Gm = U

    Parameters
    ----------
    nAvg : array_like
           observed average displacement (meters) on the north channel
           for all sites
    eAvg : array_like
           observed average displacement (meters) on the east channel
           for all sites
    uAvg : array_like
           observed average displacement (meters) on vertical channel
           for all sites
    verbose : int
              0 will only report on errors 
    Returns
    -------
    U : array_like
        right hand side in Gm = U for CMT inversion 
 
    ierr : int
           0 indicates success 
    """
    ierr = 0
    n = nAvg.shape[0]
    if (n < 1):
        print "setRHS: No observations"
        return None, -1
    if (n != eAvg.shape[0] or n != uAvg.shape[0]):
        print "setRHS: Inconsistent model sizes"
        return None, -1
    U = np.zeros(n, dtype='float64')
    ierr = GFAST_CMT__setRHS(n, verbose,
                             <double *> np.PyArray_DATA(nAvg),
                             <double *> np.PyArray_DATA(eAvg),
                             <double *> np.PyArray_DATA(uAvg),
                             <double *> np.PyArray_DATA(U))
    if (ierr != 0):
        print "setRHS: Error setting RHS!"
        U = None
    return U, ierr
 # end setRHS

 def setForwardModel(self,
                     np.ndarray[double, ndim=1, mode="c"] x1 not None,
                     np.ndarray[double, ndim=1, mode="c"] y1 not None,
                     np.ndarray[double, ndim=1, mode="c"] z1 not None,
                     deviatoric = True):
    """
    Computes the matrix of Green's functions to be used in the CMT inversion.
    Note that the columns of G in the deviatoric instance correspond to:
        {mxx, mxz, mzz, 1/2*(mxx - myy), myz}

    Parameters
    ---------- 
    x1 : array_like
         the receiver-source x (north) distance (meters) for all observations
    y1 : array_like
         the receiver-source y (east) distance (meters) for all observations
    z1 : array_like
         the receiver-source z-distance (meters) for all observations.
         notice, z increases up from the free surface so for most applications
         z will be negative
    deviatoric : bool
                 If true applies the zero-trace constraint (this is the
                 only valid option at the moment).
    Returns
    ------- 

    ierr : int
           0 indicates succcess
    """
    ierr = 0
    l1 = x1.shape[0]
    if (l1 < 1):
        print "setForwardModel: No observations"
        return None, -1
    if (l1 != y1.shape[0] or l1 != z1.shape[0]):
        print "setForwardModel: Inconsistent array sizes"
        return None, -1
    # Compute deviatoric constrained forward model
    if (deviatoric):
        G = np.zeros(l1*5, dtype='float64')
        ierr = GFAST_CMT__setForwardModel__deviatoric(l1,
                                                 <double *> np.PyArray_DATA(x1),
                                                 <double *> np.PyArray_DATA(y1),
                                                 <double *> np.PyArray_DATA(z1),
                                                 <double *> np.PyArray_DATA(G))
        if (ierr != 0):
            print "setForwardModel: Error setting G"
            return None, -1
        G = G.reshape([l1,5])
        return G, ierr
    # Compute general moment tensor
    else:
        print "setForwardModel: Full MT not yet programmed!"
        return None, -1

    return None, -1 
