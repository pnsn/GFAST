#!/usr/bin/python
import math
import numpy
import time
from coord_tools import ll2utm, utm2ll
from scaling import PGD
from cmt import moment_tensor
from fault_plane import fault_CMT
from RTOkada import rtokada
import matplotlib.pyplot as plt

def data_engine_pgd(eq_lat,eq_lon,eq_dep,to,sta_lat,sta_lon,sta_alt,nbuff,ebuff,ubuff,tbuff,runtime,Tlatency):

	disp = numpy.sqrt(numpy.power(nbuff,2)+numpy.power(ebuff,2)+numpy.power(ubuff,2))
	(x1,y1) = ll2utm(eq_lon,eq_lat,-71)
	x1 = x1-68000
	y1 = y1+109000
	print utm2ll(x1,y1,-71)

	(x2,y2) = ll2utm(sta_lon,sta_lat,-71)
	hypodist = numpy.sqrt(numpy.power(x1-x2,2)+numpy.power(y1-y2,2)+numpy.power(eq_dep*1000-sta_alt,2))
	epidist = numpy.sqrt(numpy.power(x1-x2,2)+numpy.power(y1-y2,2))

	

	effhypodist = hypodist/1000+Tlatency*3

	a1 = numpy.where(effhypodist < (runtime-to)*3)[0]
	a1 = numpy.array(a1)

	
	if len(a1) > 3:
		Dnew = disp[a1,0:runtime*5]
		mpgd_vr = numpy.zeros([100,1])
		mpgd = numpy.zeros([100,1])
		maxD = numpy.zeros([len(a1),1])
		
		#maxD = numpy.amax(Dnew,axis=1,out=maxD)
		maxD = numpy.nanmax(Dnew,axis=1,out=maxD,keepdims=True)


		

		dep = 1
		while dep < 101:
			hypoupdate = numpy.sqrt(numpy.power(x1-x2[a1],2)+numpy.power(y1-y2[a1],2)+numpy.power(dep*1000-sta_alt[a1],2))
			[MPGD,VR_PGD] = PGD(100*maxD,hypoupdate/1000,epidist[a1]/1000)
			mpgd[dep-1] = MPGD
			mpgd_vr[dep-1] = VR_PGD
			dep = dep+1
	else:
		mpgd_vr = numpy.zeros([100,1])
		mpgd = numpy.zeros([100,1])
		
	return(mpgd,mpgd_vr,len(a1))



def data_engine_cmt(eq_lat,eq_lon,eq_dep,to,sta_lat,sta_lon,sta_alt,nbuff,ebuff,ubuff,tbuff,runtime,Tlatency):
	#fid = open('gsearch_cmt.txt','w')
	(x1,y1) = ll2utm(eq_lon,eq_lat,-71)
	(x2,y2) = ll2utm(sta_lon,sta_lat,-71)
	hypodist = numpy.sqrt(numpy.power(x1-x2,2)+numpy.power(y1-y2,2)+numpy.power(eq_dep*1000-sta_alt,2))
	epidist = numpy.sqrt(numpy.power(x1-x2,2)+numpy.power(y1-y2,2))

	effhypodist = hypodist/1000+Tlatency*1
	a1 = numpy.where(effhypodist < (runtime-to)*2+10*1)[0]
	a1 = numpy.array(a1)

	if len(a1) > 3:
		

		VARRED = numpy.zeros([100,1])
		S1 = numpy.zeros([100,1])
		S2 = numpy.zeros([100,1])
		S3 = numpy.zeros([100,1])
		S4 = numpy.zeros([100,1])
		S5 = numpy.zeros([100,1])
		S6 = numpy.zeros([100,1])
		STR1 = numpy.zeros([100,1])
		STR2 = numpy.zeros([100,1])
		DIP1 = numpy.zeros([100,1])
		DIP2 = numpy.zeros([100,1])
		RAK1 = numpy.zeros([100,1])
		RAK2 = numpy.zeros([100,1])
		MW = numpy.zeros([100,1])

		

		
		#lons = 1
		#while lons < 21:
		#	lats = 1
		#	while lats < 21:
		#		dep = 1
		#		while dep < 101:
		#			dlat = (lats - 10)*0.1
		#			dlon = (lons - 10)*0.1
		#			[S,VR,Mw,NP1,NP2] = moment_tensor(sta_lat[a1],sta_lon[a1],sta_alt[a1],nbuff[a1,:],ebuff[a1,:],ubuff[a1,:],eq_lat-dlat,eq_lon-dlon,dep,runtime,effhypodist[a1],epidist[a1])

		#			latnew = "{0:.6f}".format(float(eq_lat-dlat))
		#			lonnew = "{0:.6f}".format(float(eq_lon-dlon))
		#			depnew = "{0:.2f}".format(float(dep))
		#			vrnew = "{0:.2f}".format(float(VR))
		#			fid.write(latnew+' '+lonnew+' '+depnew+' '+vrnew+'\n')
		#			dep = dep+1
		#		lats = lats+1
		#	lons = lons+1


		

		dep = 1
		while dep < 101:
			[S,VR,Mw,NP1,NP2] = moment_tensor(sta_lat[a1],sta_lon[a1],sta_alt[a1],nbuff[a1,:],ebuff[a1,:],ubuff[a1,:],eq_lat,eq_lon,dep,runtime,effhypodist[a1],epidist[a1])
			VARRED[dep-1,0] = VR
			S1[dep-1,0] = S[0]
			S2[dep-1,0] = S[1]
			S3[dep-1,0] = S[2]
			S4[dep-1,0] = S[3]
			S5[dep-1,0] = S[4]
			#S6[dep-1,0] = S[5]
			STR1[dep-1,0] = NP1['strike']
			STR2[dep-1,0] = NP2['strike']
			DIP1[dep-1,0] = NP1['dip']
			DIP2[dep-1,0] = NP2['dip']
			RAK1[dep-1,0] = NP1['rake']
			RAK2[dep-1,0] = NP2['rake']
			MW[dep-1,0] = Mw
			dep = dep+1
					
	else:
		VARRED = numpy.zeros([100,1])
		S1 = numpy.zeros([100,1])
		S2 = numpy.zeros([100,1])
		S3 = numpy.zeros([100,1])
		S4 = numpy.zeros([100,1])
		S5 = numpy.zeros([100,1])
		S6 = numpy.zeros([100,1])
		STR1 = numpy.zeros([100,1])
		STR2 = numpy.zeros([100,1])
		DIP1 = numpy.zeros([100,1])
		DIP2 = numpy.zeros([100,1])
		RAK1 = numpy.zeros([100,1])
		RAK2 = numpy.zeros([100,1])
		MW = numpy.zeros([100,1])

	return(MW,S1,S2,S3,S4,S5,S6,STR1,STR2,DIP1,DIP2,RAK1,RAK2,VARRED,len(a1))



def data_engine_ff(eq_lat,eq_lon,eq_dep,MCMT,STR1,STR2,DIP1,DIP2,nstr,ndip,to,sta_lat,sta_lon,sta_alt,nbuff,ebuff,ubuff,tbuff,runtime,Tlatency):
	
	(x1,y1) = ll2utm(eq_lon,eq_lat,-71)
	(x2,y2) = ll2utm(sta_lon,sta_lat,-71)
	hypodist = numpy.sqrt(numpy.power(x1-x2,2)+numpy.power(y1-y2,2)+numpy.power(eq_dep*1000-sta_alt,2))

	effhypodist = hypodist/1000+Tlatency*1
	a1 = numpy.where(effhypodist < (runtime-to)*2+10*1)[0]
	a1 = numpy.array(a1)

	if len(a1) > 3:
		
		[fault_lon1,fault_lat1,fault_alt1,strike1,dip1,dl1,dw1,lon11,lat11,lon12,lat12,lon13,lat13,lon14,lat14]=fault_CMT(eq_lat,eq_lon,eq_dep,MCMT,STR1,DIP1,nstr,ndip)

		[SSLIP1,DSLIP1,MW1,EN1,NN1,UN1,VR1,Einp,Ninp,Uinp]=rtokada(sta_lat[a1], sta_lon[a1], sta_alt[a1], nbuff[a1,:],ebuff[a1,:],ubuff[a1,:], fault_lat1, fault_lon1, fault_alt1, strike1, dip1, dl1, dw1, nstr, ndip,runtime,effhypodist[a1])

		[fault_lon2,fault_lat2,fault_alt2,strike2,dip2,dl2,dw2,lon21,lat21,lon22,lat22,lon23,lat23,lon24,lat24]=fault_CMT(eq_lat,eq_lon,eq_dep,MCMT,STR2,DIP2,nstr,ndip)
		[SSLIP2,DSLIP2,MW2,EN2,NN2,UN2,VR2,Einp2,Ninp2,Uinp2]=rtokada(sta_lat[a1], sta_lon[a1], sta_alt[a1], nbuff[a1,:],ebuff[a1,:],ubuff[a1,:], fault_lat2, fault_lon2, fault_alt2, strike2, dip2, dl2, dw2, nstr, ndip,runtime,effhypodist[a1])


	
		if (VR1 >= VR2):
			FaultPlane = 1
			SSLIP = SSLIP1
			DSLIP = DSLIP1
			MFF = MW1
			EN = EN1
			NN = NN1
			UN = UN1
			VR = VR1
			STR = STR1
			DIP = DIP1
			FAULT_LAT = fault_lat1
			FAULT_LON = fault_lon1
			FAULT_ALT = fault_alt1
			FLAT1 = lat11
			FLON1 = lon11
			FLAT2 = lat12
			FLON2 = lon12
			FLAT3 = lat13
			FLON3 = lon13
			FLAT4 = lat14
			FLON4 = lon14
			
		else:
			FaultPlane = 2
			SSLIP = SSLIP2
			DSLIP = DSLIP2
			MFF = MW2
			EN = EN2
			NN = NN2
			UN = UN2
			VR = VR2
			STR = STR2
			DIP = DIP2
			FAULT_LAT = fault_lat2
			FAULT_LON = fault_lon2
			FAULT_ALT = fault_alt2
			FLAT1 = lat21
			FLON1 = lon21
			FLAT2 = lat22
			FLON2 = lon22
			FLAT3 = lat23
			FLON3 = lon23
			FLAT4 = lat24
			FLON4 = lon24
		
		

	return(SSLIP,DSLIP,MFF,Einp,Ninp,Uinp,EN,NN,UN,sta_lat[a1],sta_lon[a1],FAULT_LAT,FAULT_LON,FAULT_ALT,VR1,VR2,FaultPlane,FLAT1,FLON1,FLAT2,FLON2,FLAT3,FLON3,FLAT4,FLON4)

