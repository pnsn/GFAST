#!/usr/bin/python
import math
import numpy
import okadagreen
from coord_tools import ll2utm


def rtokada(sta_lat, sta_lon, sta_alt, n, e, u, fault_lat, fault_lon, fault_alt, strike, dip, LEN, WID, nstr, ndip, timer, effhypodist):

	l1=len(sta_lat)
	l2=len(fault_lat)
	
	LEN = LEN*1000 
	WID = WID*1000
	xrs = numpy.zeros([l2,l1])
	yrs = numpy.zeros([l2,l1])
	zrs = numpy.zeros([l2,l1])

	for i in range (0, l2):
		for j in range (0, l1):
			(x1,y1) = ll2utm(sta_lon[j],sta_lat[j], -71)
			(x2,y2) = ll2utm(fault_lon[i],fault_lat[i], -71)
			xrs[i,j] = (x1-x2)
			yrs[i,j] = (y1-y2)
			zrs[i,j] = sta_alt[j]+fault_alt[i]*1000


	G = okadagreen.greenF(xrs, yrs, zrs, strike, dip, WID, LEN) #Compute Green's functions
	
	
	#Regularization Matrix creation
	T = numpy.zeros([(2*ndip*nstr)+2*(2*nstr+2*(ndip-2)),2*l2]) #Prefill matrix with zeros
	TU = numpy.zeros([(2*ndip*nstr)+2*(2*nstr+2*(ndip-2)),1]) #Appended to observation vector. This minimizes the difference between adjacent slip cells
	k=0
	for j in range (0, ndip):
		for i in range (0, nstr):
			for m in range (0, 2):
				index1 = j*nstr+i
				index2 = j*nstr+i-1
				index3 = j*nstr+i+1
				index4 = (j-1)*nstr+i
				index5 = (j+1)*nstr+i
				
				if (index1 >= 0 and index1 < l2):
					T[k,2*index1+m] = -2.0/LEN[0]/LEN[0]*1000*1000-2.0/WID[0]/WID[0]*1000*1000
				if (index2 >= 0 and index2 < l2):
					T[k,2*index2+m] = 1.0/LEN[0]/LEN[0]*1000*1000
				if (index3 >= 0 and index3 < l2):
					T[k,2*index3+m] = 1.0/LEN[0]/LEN[0]*1000*1000
				if (index4 >= 0 and index4 < l2):
					T[k,2*index4+m] = 1.0/WID[0]/WID[0]*1000*1000
				if (index5 >= 0 and index5 < l2):
					T[k,2*index5+m] = 1.0/WID[0]/WID[0]*1000*1000

				k=k+1

	for j in range (0, ndip):
		for i in range (0, nstr):
			for m in range (0, 2):
				index1 = j*nstr+i
				if (j==ndip-1 or i==0 or i ==nstr-1):
					T[k,2*index1+m] = 100/WID[0]/LEN[0]*1000*1000
					k=k+1
	
	U = numpy.zeros([3*l1,1]) #Create data vector
	Einp = numpy.zeros([l1,1]) #Create data vector
	Ninp = numpy.zeros([l1,1]) #Create data vector
	Uinp = numpy.zeros([l1,1]) #Create data vector


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
			
			U[3*i,0]= E
			U[3*i+1,0]= N
			U[3*i+2,0]= Z
			Einp[i,0] = E
			Ninp[i,0] = N
			Uinp[i,0] = Z

		else:
			
			U[3*i,0]= 0
			U[3*i+1,0]= 0
			U[3*i+2,0]= 0

	

	

	UD = numpy.vstack((U,TU))
	
	lampred = 1.0/math.pow(l2*2,2)/numpy.mean(numpy.absolute(G))/4*LEN[0]*WID[0]/1000/1000


	T2 = T*lampred




	G2 = numpy.vstack((G,T2))

	
	S = numpy.linalg.lstsq(G2,UD)[0]



	UP = numpy.dot(G,S) #Forward model for model fits
	VR = (1-numpy.linalg.norm(UP-U)**2/numpy.linalg.norm(U)**2)*100 #variance reduction

	SSLIP = numpy.zeros([l2,1])
	DSLIP = numpy.zeros([l2,1])

	for i in range (0,l2):
		SSLIP[i,0] = S[2*i,0]
		DSLIP[i,0] = S[2*i+1,0]

	EN = numpy.zeros([l1,1])
	NN = numpy.zeros([l1,1])
	UN = numpy.zeros([l1,1])

	for i in range (0, l1):
		EN[i,0]=UP[3*i,0]
		NN[i,0]=UP[3*i+1,0]
		UN[i,0]=UP[3*i+2,0]

	ST = numpy.sqrt(SSLIP**2+DSLIP**2)

	Mo = numpy.sum(3.0e10*ST*LEN*WID)
	if (Mo > 0):
		MW = 2.0/3.0*math.log10(Mo/1.0e-7)-10.7
	else:
		MW = 0

	print len(EN), len(Einp)
	return(SSLIP,DSLIP,MW,EN,NN,UN,VR,Einp,Ninp,Uinp)
