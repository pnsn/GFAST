#!/usr/bin/python
from numpy import zeros
from numpy import array
from numpy import sqrt
from numpy import pi
from numpy import dot
from numpy import power
from numpy import where
from numpy import exp
from numpy import multiply
from numpy import log10
from numpy import diag
from numpy.linalg import lstsq
from numpy.linalg import norm
from math import atan2
from GFAST_coordtools import ll2utm
from GFAST_cmtgreen import greenF
#from GFAST_scaling import PGD

def GreenF_test(SA_lat, SA_lon, SA_dep, sta_lat, sta_lon):
   l1=len(sta_lat)
   l2=1
   xrs = zeros([l1,1])
   yrs = zeros([l1,1])
   zrs = zeros([l1,1])
   azi = zeros([l1,1])
   backazi = zeros([l1,1])
   for j in range (0, l1):
       (x1,y1) = ll2utm(sta_lon[j],sta_lat[j], -123.0)
       (x2,y2) = ll2utm(SA_lon,SA_lat, -123.0)

       azi[j,0] = 90-180/pi*atan2(x2-x1,y2-y1)
       if (azi[j,0] < 0): 
           azi[j,0] = azi[j,0]+360
       if (azi[j,0] > 360):
           azi[j,0] = azi[j,0]-360

       if (azi[j,0] < 180):
           backazi[j,0] = azi[j,0]+180
       if (azi[j,0] > 180):
           backazi[j,0] = azi[j,0]-180
       if (azi[j,0] == 180):
           backazi[j,0] = 0 

       xrs[j,0] = (x1-x2)
       yrs[j,0] = (y1-y2)
       zrs[j,0] = (SA_dep*1000)
   G = greenF(xrs, yrs, zrs,90-azi+180) #Compute Green's function
   print "CMT Gmat:"
   for l1 in xrange(len(G[:,0])):
      line = ''
      for i in xrange(len(G[l1,:])):
         if (i == 3):
             line = line + '\n'
         line = line + "%14.8e,"%G[l1,i]
      print line

def PGD(d, r, repi):
   """
     Beating Brendan's PGD function into submission...
   """
   A = -6.687
   B = 1.500
   C = -0.214
   #A = -4.434
   #B = 1.047
   #C = -0.138
   d[d-6.0 < 0] = 0.01

   W = diag(exp(-power(repi,2)/8/power(min(repi),2)))

   G = B+C*(log10(r))
   b = log10(d)-A
   M = lstsq( dot(W,G), dot(W,b) )[0]
 
   UP = dot(G,M)
   VR = (1.0-norm(power(10,b)-power(10,UP))/norm(power(10,b)))*100

   return(M[0,0],VR)

def UTM_test(sta_lat, sta_lon):
   (x2,y2) = ll2utm(sta_lon,sta_lat,-123.0)
   print "xutms:", x2
   print "yutms:", y2

def PGD_basic(SA_lat, SA_lon, SA_dep,
              sta_lat, sta_lon, d):
   """
     Creates basic benchmark for PGD scaling
   """
   hypoupdate = zeros(len(sta_lat))
   (x1,y1) = ll2utm(SA_lon, SA_lat, -123.0)
   (x2,y2) = ll2utm(sta_lon,sta_lat,-123.0)
   epidist = sqrt(power(x1-x2,2) + power(y1-y2,2))
   runtime = 300.0 # get all events
   SA_time = 0.0
   a1 = where(epidist/1000.0 < (runtime-SA_time)*3)[0]
   a1 = array(a1)

   dep = SA_dep
   maxD = zeros([len(a1),1])
   maxD[:,0] = d[:]
   hypoupdate = zeros([len(a1),1])
   hypoupdate[:,0] = sqrt( power(x1-x2[a1], 2)
                         + power(y1-y2[a1], 2)
                         + power(dep*1000,2))
   [MPGD,VR_PGD] = PGD(100*maxD,hypoupdate/1000,epidist[a1]/1000)
   print "Benchmarks for basic PGD is (MPGD,VR_PGD):", MPGD,VR_PGD
   
nsta = 9
sta_lat = zeros(9)
sta_lon = zeros(9)
# Nisqually
SA_lat = 47.19
SA_lon =-122.66
SA_dep = 57.0
# stations: bamf, cabl, chzz, eliz, holb, neah, p058, p159
sta_lat = array([ 48.83532872,  42.83609887,  45.48651503,
                  49.87305293,  50.64035267,  41.90232489,
                  48.29785467,  40.87630594,  40.50478709]) 
sta_lon = array([-125.13510527,-124.56334629,-123.97812400,
                 -127.12266484,-128.13499899,-120.30283244,
                 -124.62490719,-124.07537043,-124.28278289])
# bogus displacements
d = array([4, 7.1, 8,  3, 1, 9,  6, 5.5, 6.5])*0.01

UTM_test(sta_lat, sta_lon)

PGD_basic(SA_lat, SA_lon, SA_dep, sta_lat, sta_lon, d)

GreenF_test(SA_lat, SA_lon, SA_dep, sta_lat, sta_lon)
