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
 int GFAST_CMT__decomposeMomentTensor(int nmt,
                                      const double *M, 
                                      double *DC_pct,
                                      double *Mw,
                                      double *strike1, double *strike2,
                                      double *dip1, double *dip2,
                                      double *rake1, double *rake2)
 int GFAST_CMT__setRHS(int n, int verbose,
                       const double *nOffset,
                       const double *eOffset,
                       const double *uOffset,
                       double *U)
 int GFAST_CMT__setForwardModel__deviatoric(int l1, 
                                            const double *x1, 
                                            const double *y1, 
                                            const double *z1, 
                                            double *G)
 int GFAST_CMT__depthGridSearch(int l1, int ndeps,
                                int verbose,
                                bool deviatoric,
                                double utmSrcEasting,
                                double utmSrcNorthing,
                                const double *srcDepths,
                                const double *utmRecvEasting,
                                const double *utmRecvNorthing,
                                const double *staAlt,
                                const double *nObsOffset,
                                const double *eObsOffset,
                                const double *uObsOffset,
                                const double *nWts,
                                const double *eWts,
                                const double *uWts,
                                double *nEst,
                                double *eEst,
                                double *uEst,
                                double *mts)
 
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
        Moment tensor (Nm) in NED coordinates where each column of M is packed
        {mxx, myy, mzz, mxy, mxz, myz}

    Returns
    -------
    DC_pct : array_like 
             double couple percentage
    Mw : array_like
         scalar moment for all moment tensors
    strike1 : array_like 
              strike angle of first nodal plane (degrees) for all
              moment tensors
    strike2 : array_like 
              strike angle of second nodal plane (degrees) for all
              moment tensors
    dip1 : array_like 
           dip angle of first nodal plane (degrees) for all moment tensors
    dip2 : array_lke
           dip angle of second nodal plane (degrees) for all moment tensors
    rake1 : array_like
            rake angle of first nodal plane (degrees) for all moment tensors
    rake2 : array_like 
            rake angle of second nodal plane (degrees) for all moment tensors
    ierr : int
           0 indicates success 
    """
    ierr = 0 
    DC_pct = None
    Mw = None
    strike1 = None
    strike2 = None
    dip1 = None
    dip2 = None
    rake1 = None
    rake2 = None
    if (M.size() == 6):
        nmt = 1
        n = M.shape[0]
    else:
        nmt = M.shape[0]
    M.reshape([nmt*6], format='c') 
    if (n != 6): 
        print "decompose: Error MTs must be 6!"
        ierr = 1
    else:
        DC_pct = np.zeros(nmt, dtype='float64')
        Mw = np.zeros(nmt, dtype='float64')
        strike1 = np.zeros(nmt, dtype='float64')
        strike2 = np.zeros(nmt, dtype='float64')
        dip1 = np.zeros(nmt, dtype='float64')
        dip2 = np.zeros(nmt, dtype='float64')
        rake1 = np.zeros(nmt, dtype='float64')
        rake2 = np.zeros(nmt, dtype='float64')
        nmt = n/6
        ierr = GFAST_CMT__decomposeMomentTensor(nmt,
                                                <double *> np.PyArray_DATA(M),
                                                <double *> np.PyArray_DATA(DC_pct),
                                                <double *> np.PyArray_DATA(Mw),
                                                <double *> np.PyArray_DATA(strike1),
                                                <double *> np.PyArray_DATA(strike2),
                                                <double *> np.PyArray_DATA(dip1),
                                                <double *> np.PyArray_DATA(dip2),
                                                <double *> np.PyArray_DATA(rake1),
                                                <double *> np.PyArray_DATA(rake2))
        if (ierr != 0):
            print "decompose: Error in moment tensor decomposition"
    return DC_pct, Mw, strike1, strike2, dip1, dip2, rake1, rake2, ierr
 # end decompose

 def setRHS(self,
            np.ndarray[double, ndim=1, mode="c"] nOffset not None,
            np.ndarray[double, ndim=1, mode="c"] eOffset not None,
            np.ndarray[double, ndim=1, mode="c"] uOffset not None,
            verbose = 0):
    """
    Sets the right hand side in the CMT inversion such that Gm = U

    Parameters
    ----------
    nOffset : array_like
              observed offset (meters) on the north channel
              for all sites
    eOffset : array_like
              observed offset (meters) on the east channel
              for all sites
    uOffset : array_like
              observed offset (meters) on vertical channel
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
    n = nOffset.shape[0]
    if (n < 1):
        print "setRHS: No observations"
        return None, -1
    if (n != eOffset.shape[0] or n != uOffset.shape[0]):
        print "setRHS: Inconsistent model sizes"
        return None, -1
    U = np.zeros(n, dtype='float64')
    ierr = GFAST_CMT__setRHS(n, verbose,
                             <double *> np.PyArray_DATA(nOffset),
                             <double *> np.PyArray_DATA(eOffset),
                             <double *> np.PyArray_DATA(uOffset),
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
        G = G.reshape([l1, 5])
        return G, ierr
    # Compute general moment tensor
    else:
        print "setForwardModel: Full MT not yet programmed!"
        return None, -1

    return None, -1 

 def depthGridSearch(self,
                     utmSrcEasting, utmSrcNorthing,
                     np.ndarray[double, ndim=1, mode="c"] srcDepths not None,
                     np.ndarray[double, ndim=1, mode="c"] utmRecvEasting not None,
                     np.ndarray[double, ndim=1, mode="c"] utmRecvNorthing not None,
                     np.ndarray[double, ndim=1, mode="c"] staAlt not None,
                     np.ndarray[double, ndim=1, mode="c"] nObsOffset not None,
                     np.ndarray[double, ndim=1, mode="c"] eObsOffset not None,
                     np.ndarray[double, ndim=1, mode="c"] uObsOffset not None,
                     np.ndarray[double, ndim=1, mode="c"] nWts = None,
                     np.ndarray[double, ndim=1, mode="c"] eWts = None,
                     np.ndarray[double, ndim=1, mode="c"] uWts = None,
                     deviatoric = True,
                     verbose = 0):
    """
    Performs the CMT depth grid search.

    Parameters
    ----------
    utmSrcEasting : array_like
                    UTM source position in x or east (m)
    utmSrcNorthing : array_like
                     UTM source position in y or north (m)
    utmRecvEasting : array_like
                     UTM receiver position in x or east (m)
    utmRecvNorthing : array_like
                      UTM receiver position in y or north (m)
    staAlt : array_like
             station altitude - positive above sea-level (m)
    nObsOffset : array_like
              observed offset in the north component (m) at each station
    eObsOffset : array_like
              observed offset in the east component (m) at each station
    uObsOffset : array_like
              observed offset in the north component (m) at each station
    nWts: array_like
          if specified these are the weights on the north observations.
          othwerise this will be set to numpy.ones(l1)
    eWts: array_like
          if specified these are the weights on the east observations
          othwerise this will be set to numpy.ones(l1)
    uWts: array_like
          if specified these are the weights on the vertical observations
          othwerise this will be set to numpy.ones(l1)
    deviatoric : bool
                 if True then the moment tensor inversion constrains
                 the solution s.t. the result must be purely deviatoric.
                 At present this is the only option.
    verbose : int
              Controls the verbosity - 0 is quiet 

    Returns
    -------
    nEst : array_like
    eEst : array_like
    nEst : array_like

    mts : array_like
    """
    ierr = 0
    ndeps = srcDepths.shape[0]
    l1 = utmRecvEasting.shape[0]
    if (nWts == None):
        nWts = np.zeros(l1, dtype='float64')
    if (eWts == None):
        eWts = np.zeros(l1, dtype='float64')
    if (uWts == None):
        uWts = np.zeros(l1, dtype='float64')
    nEst = None
    eEst = None
    uEst = None
    mts = None
    if (not deviatoric):
        ierr = 1
        print "depthGridSearch: General moment tensor inversion not yet done"
    if (ndeps < 1):
        ierr = 1
        print "depthGridSearch: No depths in grid search"
    if (l1 != utmRecvNorthing.shape[0] or l1 != staAlt.shape[0] or
        l1 != nObsOffset.shape[0] or l1 != eObsOffset.shape[0] or
        l1 != uObsOffset.shape[0]):
        ierr = 1
        print "depthGridSearch: Inconsistent size"
    if (l1 != uWts.shape[0] or nWts.shape[0] or eWts.shape[0]):
        ierr = 1
        print "depthGridSearch: Data weight size inconsistent"
    if (ierr != 0):
        nEst = np.zeros(l1*ndeps, dtype='float64')
        eEst = np.zeros(l1*ndeps, dtype='float64')
        uEst = np.zeros(l1*ndeps, dtype='float64')
        mts = np.zeros(6*ndeps, dtype='float64')
        ierr = GFAST_CMT__depthGridSearch(l1, ndeps,
                                      verbose,
                                      deviatoric,
                                      utmSrcEasting,
                                      utmSrcNorthing,
                                      <double *> np.PyArray_DATA(srcDepths),
                                      <double *> np.PyArray_DATA(utmRecvEasting),
                                      <double *> np.PyArray_DATA(utmRecvNorthing),
                                      <double *> np.PyArray_DATA(staAlt),
                                      <double *> np.PyArray_DATA(nObsOffset),
                                      <double *> np.PyArray_DATA(eObsOffset),
                                      <double *> np.PyArray_DATA(uObsOffset),
                                      <double *> np.PyArray_DATA(nWts),
                                      <double *> np.PyArray_DATA(eWts),
                                      <double *> np.PyArray_DATA(uWts),
                                      <double *> np.PyArray_DATA(nEst),
                                      <double *> np.PyArray_DATA(eEst),
                                      <double *> np.PyArray_DATA(uEst),
                                      <double *> np.PyArray_DATA(mts))
        if (ierr != 0):
            print "depthGridSearch: Error performing depth grid search"
            nEst = None
            eEst = None
            uEst = None
            mts = None
        else:
            nEst = nEst.reshape([ndeps, l1])
            eEst = eEst.reshape([ndeps, l1])
            uEst = uEst.reshape([ndeps, l1])
            mts = mts.reshape([ndeps, 6])
        # end check on error
    # end check on no input error
    return nEst, eEst, uEst, mts, ierr
