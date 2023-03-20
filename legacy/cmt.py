#!/usr/bin/python
import math
import numpy
import okadapoint
from coord_tools import ll2utm
import obspy.imaging.beachball
import obspy.signal
from obspy.core.util.geodetics.base import gps2DistAzimuth




def moment_tensor(sta_lat, sta_lon, sta_alt, n, e, u, eq_lat, eq_lon, eq_alt,timer,effhypodist,repi):
	l1=len(sta_lat)
	l2=1

	
	xrs = numpy.zeros([l1,1])
	yrs = numpy.zeros([l1,1])
	zrs = numpy.zeros([l1,1])
	azi = numpy.zeros([l1,1])
	backazi = numpy.zeros([l1,1])
	theta = numpy.zeros([l1,1])

	for j in range (0, l1):
		(x1,y1) = ll2utm(sta_lon[j],sta_lat[j], -71)
		
		(x2,y2) = ll2utm(eq_lon,eq_lat, -71)

		result = gps2DistAzimuth(sta_lat[j],sta_lon[j],eq_lat,eq_lon)
		backazi[j,0]=result[1]
		azi[j,0]=result[2]
		
		theta[j,0] = 90 - backazi[j,0] + 180


		xrs[j,0] = (x1-x2)
		yrs[j,0] = (y1-y2)
		zrs[j,0] = (sta_alt[j]+eq_alt*1000)






	U = numpy.zeros([3*l1,1])
	for i in range (0, l1):
		efftime = math.ceil(effhypodist[i])
		NN = n[i,efftime:efftime+10]
		EE = e[i,efftime:efftime+10]
		UU = u[i,efftime:efftime+10]
		a1 = numpy.where(n[i,efftime:efftime+10] > -999)[0]
		a1 = numpy.array(a1)
		if (len(a1) > 1):
			N = numpy.nanmean(NN[a1])
			E = numpy.nanmean(EE[a1])
			Z = numpy.nanmean(UU[a1])
			
			U[3*i,0]= N
			U[3*i+1,0]= E
			U[3*i+2,0]= -Z

		else:
			
			U[3*i,0]= 0
			U[3*i+1,0]= 0
			U[3*i+2,0]= 0


	
	G = okadapoint.greenF(yrs, xrs, -zrs) #Compute Green's function
	
	S = numpy.linalg.lstsq(G,U)[0]

	M12 = S[0,0]
	M13 = S[1,0]
	M33 = S[2,0]
	M23 = S[4,0]
	M11 = S[3,0]-0.5*S[2,0]
	M22 = -S[3,0]-0.5*S[2,0]
	
	M_devi = numpy.array([[M11,M12,M13],[M12,M22,M23],[M13,M23,M33]])
	eigenwtot, eigenvtot = numpy.linalg.eig(M_devi)
	eigenw1, eigenv1 = numpy.linalg.eig(M_devi)
	eigenw = numpy.real(numpy.take(eigenw1, numpy.argsort(abs(eigenwtot))))
        eigenv = numpy.real(numpy.take(eigenv1, numpy.argsort(abs(eigenwtot)), 1))
	eigenw_devi = numpy.real(numpy.take(eigenw1, numpy.argsort(abs(eigenw1))))
        eigenv_devi = numpy.real(numpy.take(eigenv1, numpy.argsort(abs(eigenw1)), 1))
	M0_devi = max(abs(eigenw_devi))
	F = -eigenw_devi[0] / eigenw_devi[2]
	M_DC_percentage = (1 - 2 * abs(F)) * 100
	
	

	Mo = math.pow(math.pow(M11,2)+math.pow(M22,2)+math.pow(M33,2)+2*math.pow(M12,2)+2*math.pow(M13,2)+2*math.pow(M23,2),0.5)/math.pow(2,0.5)
	if (Mo == 0):
		Mw = 0
	else:
		Mw = 2*math.log10(Mo)/3-6.03

	UP = numpy.dot(G,S)

	dms = U-UP
	
	VR = (1-numpy.sum(numpy.sqrt(numpy.power(dms,2)))/numpy.sum(numpy.sqrt(numpy.power(U,2))))*M_DC_percentage

	#VR = numpy.linalg.norm(dms)/M_DC_percentage*1000


	mt = obspy.imaging.beachball.MomentTensor(M33,M11,M22,M13,-M23,-M12,26)


	axes = obspy.imaging.beachball.MT2Axes(mt)
	plane1 = obspy.imaging.beachball.MT2Plane(mt)
	plane2 = obspy.imaging.beachball.AuxPlane(plane1.strike,plane1.dip,plane1.rake)
	T = {'azimuth':axes[0].strike,'plunge':axes[0].dip}
	N = {'azimuth':axes[1].strike,'plunge':axes[1].dip}
	P = {'azimuth':axes[2].strike,'plunge':axes[2].dip}
	NP1 = {'strike':plane1.strike,'dip':plane1.dip,'rake':plane1.rake}
	NP2 = {'strike':plane2[0],'dip':plane2[1],'rake':plane2[2]}



	return(S, VR, Mw, NP1, NP2)

