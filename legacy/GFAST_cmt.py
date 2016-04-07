#!/usr/bin/python
import math
import numpy
from GFAST_cmtgreen import greenF
from GFAST_coordtools import ll2utm
import obspy.imaging.beachball
import obspy.signal



def moment_tensor(sta_lat, sta_lon, n, e, u, SA_lat, SA_lon, SA_dep,timer,epidist):
	
	l1=len(sta_lat)
	l2=1
	xrs = numpy.zeros([l1,1])
	yrs = numpy.zeros([l1,1])
	zrs = numpy.zeros([l1,1])
	azi = numpy.zeros([l1,1])
	backazi = numpy.zeros([l1,1])

	for j in range (0, l1):
		(x1,y1) = ll2utm(sta_lon[j],sta_lat[j], -123.0)
		(x2,y2) = ll2utm(SA_lon,SA_lat, -123.0)

		azi[j,0] = 90-180/math.pi*math.atan2(x2-x1,y2-y1)
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



	U = numpy.zeros([3*l1,1])
	for i in range (0, l1):
		efftime = math.ceil(epidist[i]/2/1000)
		
		if (efftime < 290):
			NN = n[i,efftime:efftime+10]
			EE = e[i,efftime:efftime+10]
			UU = u[i,efftime:efftime+10]
			
		else:
			NN = n[i,290:300]
			EE = e[i,290:300]
			UU = u[i,290:300]
			



		N = numpy.nanmean(NN)
		E = numpy.nanmean(EE)

		[r,t] = obspy.signal.rotate.rotate_NE_RT(numpy.array([N]),numpy.array([E]),backazi[i])

		U[3*i,0]= r
		U[3*i+1,0]= t
		U[3*i+2,0]= numpy.nanmean(UU)

	print U


	G = greenF(xrs, yrs, zrs,90-azi+180) #Compute Green's function
	S = numpy.linalg.lstsq(G,U)[0]


	Mo = math.pow(math.pow(S[0],2)+math.pow(S[1],2)+math.pow(S[2],2)+2*math.pow(S[3],2)+2*math.pow(S[4],2)+2*math.pow(S[5],2),0.5)/math.pow(2,0.5)
	if (Mo == 0):
		Mw = 0
	else:
		Mw = 2*math.log10(Mo)/3-6

	UP = numpy.dot(G,S)

	VR = (1-numpy.linalg.norm(U-UP)**2/numpy.linalg.norm(U)**2)*100

	mt = obspy.imaging.beachball.MomentTensor(S[0,0]/1.0e19,S[1,0]/1.0e19,S[2,0]/1.0e19,S[3,0]/1.0e19,S[4,0]/1.0e19,S[5,0]/1.0e19,26)
	axes = obspy.imaging.beachball.MT2Axes(mt)
	plane1 = obspy.imaging.beachball.MT2Plane(mt)
	plane2 = obspy.imaging.beachball.AuxPlane(plane1.strike,plane1.dip,plane1.rake)
	T = {'azimuth':axes[0].strike,'plunge':axes[0].dip}
	N = {'azimuth':axes[1].strike,'plunge':axes[1].dip}
	P = {'azimuth':axes[2].strike,'plunge':axes[2].dip}
	NP1 = {'strike':plane1.strike,'dip':plane1.dip,'rake':plane1.rake}
	NP2 = {'strike':plane2[0],'dip':plane2[1],'rake':plane2[2]}

	return(S, VR, Mw, NP1, NP2)

