#!/usr/bin/python

import math
from GFAST_coordtools import ll2utm, utm2ll
import numpy

def fault_CMT(lat,lon,depth,M,strikeF,dipF,nstr,ndip):
	[x0,y0]=ll2utm(lon, lat, -123.0)
	x0 = x0/1000
	y0 = y0/1000

	fault_alt = numpy.zeros([nstr*ndip, 1])
	fault_lon = numpy.zeros([nstr*ndip, 1])
	fault_lat = numpy.zeros([nstr*ndip, 1])
	strike = strikeF*numpy.ones([nstr*ndip, 1]) 
	dip = dipF*numpy.ones([nstr*ndip, 1]) 

	AREA = math.pow(10,-3.49+0.91*M) #Area of fault from Dreger and Kaverina, 2000
	LEN = math.pow(10,-2.44+0.59*M) #Length of fault from Dreger and Kaverina, 2000
	WID = AREA/LEN #Fault width is area divided by length

	LEN = LEN+0.1*LEN #fault dimensions with 10% safety factor added
	WID = WID+0.1*WID


	if (WID/2*math.sin(dipF*math.pi/180) > depth):#sets the initial top depth  - either depth-width/2*sin(dip) or 0 depending on how wide and close to surface fault is
		z0 = 0
		x0 = x0 - LEN/2*math.sin(strikeF*math.pi/180) - depth*math.cos(dipF*math.pi/180)*math.sin((strikeF+90)*math.pi/180)
		y0 = y0 - LEN/2*math.cos(strikeF*math.pi/180) - depth*math.cos(dipF*math.pi/180)*math.cos((strikeF+90)*math.pi/180)
	else:
		z0 = depth - WID/2*math.sin(dipF*math.pi/180)
		x0 = x0 - LEN/2*math.sin(strikeF*math.pi/180) - WID/2*math.cos(dipF*math.pi/180)*math.sin((strikeF+90)*math.pi/180)
		y0 = y0 - LEN/2*math.cos(strikeF*math.pi/180) - WID/2*math.cos(dipF*math.pi/180)*math.cos((strikeF+90)*math.pi/180)

	DLEN = LEN/nstr
	DWID = WID/ndip

	xdoff = DWID*math.cos(dipF*math.pi/180)*math.sin((strikeF+90)*math.pi/180)
	ydoff = DWID*math.cos(dipF*math.pi/180)*math.cos((strikeF+90)*math.pi/180)

	xsoff = DLEN*math.sin(strikeF*math.pi/180)
	ysoff = DLEN*math.cos(strikeF*math.pi/180)


	k=0
	for j in range (0, ndip):
		for i in range (0, nstr):
			fault_X = x0 + (0.5+i)*xsoff + (0.5+j)*xdoff
			fault_Y = y0 + (0.5+i)*ysoff + (0.5+j)*ydoff

			fault_alt[k] = z0 + (0.5+j)*DWID*math.sin(dipF*math.pi/180)
			(fault_lon[k], fault_lat[k]) = utm2ll(fault_X*1000,fault_Y*1000,-123.0)
			k = k+1

	return (fault_lon, fault_lat, fault_alt, strike, dip, DLEN*numpy.ones([nstr*ndip,1]), DWID*numpy.ones([nstr*ndip,1]))

			
