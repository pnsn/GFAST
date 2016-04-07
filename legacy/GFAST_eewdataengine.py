#!/usr/bin/python
import math
import numpy
import time
from GFAST_coordtools import ll2utm
from GFAST_scaling import PGD
from GFAST_faultplane import fault_CMT
from GFAST_cmt import moment_tensor



def data_engine_pgd(SA_lat,SA_lon,SA_dep,SA_time,SA_mag,SA_eventid,sta_lat,sta_lon,nbuffnew,ebuffnew,ubuffnew,runtime):
	fname = 'events/GFAST_PGD_' + str(int(SA_eventid)) + '.txt' 
	f = open(fname,'a') #open gps data file to append data
	(x1,y1) = ll2utm(SA_lon,SA_lat,-123.0)
	(x2,y2) = ll2utm(sta_lon,sta_lat,-123.0)

	
	epidist = numpy.sqrt(numpy.power(x1-x2,2)+numpy.power(y1-y2,2))

	
	a1 = numpy.where(epidist/1000 < (runtime-SA_time)*3)[0]
	a1 = numpy.array(a1)

	if len(a1) > 3:

		dt = runtime-SA_time
		if (dt > 299):
			dt = 299
		Dnew = numpy.zeros([len(a1),int(dt)])

		ind = 0
		while ind < len(a1):

			if numpy.isnan(nbuffnew[a1[ind],int(dt)]):
				n0 = 0
			else:
				n0 = nbuffnew[a1[ind],int(dt)]
			if numpy.isnan(ebuffnew[a1[ind],int(dt)]):
				e0 = 0
			else:
				e0 = ebuffnew[a1[ind],int(dt)]
			if numpy.isnan(ubuffnew[a1[ind],int(dt)]):
				u0 = 0
			else:
				u0 = ubuffnew[a1[ind],int(dt)]

			nn = nbuffnew[a1[ind],0:int(dt)]-n0
			ee = ebuffnew[a1[ind],0:int(dt)]-e0
			uu = ubuffnew[a1[ind],0:int(dt)]-u0
			Dnew[ind,:]=numpy.sqrt(numpy.power(nn,2)+numpy.power(ee,2)+numpy.power(uu,2))
			ind = ind+1
		

		mpgd_vr = numpy.zeros([100,1])
		mpgd = numpy.zeros([100,1])

		maxD = numpy.zeros([len(a1),1])
		maxD = numpy.nanmax(Dnew,axis=1,out=maxD,keepdims=True)
	

		dep = 1
		while dep < 101:
			hypoupdate = numpy.sqrt(numpy.power(x1-x2[a1],2)+numpy.power(y1-y2[a1],2)+numpy.power(dep*1000,2))
			[MPGD,VR_PGD] = PGD(100*maxD,hypoupdate/1000,epidist[a1]/1000)
			mpgd[dep-1] = MPGD
			mpgd_vr[dep-1] = VR_PGD
			dep = dep+1


		dep_vr_pgd = numpy.argmax(mpgd_vr)
		
		
		mpgdstr = "{0:.2f}".format(float(mpgd[dep_vr_pgd]))
		vrpgdstr = "{0:.2f}".format(float(mpgd_vr[dep_vr_pgd]))

		f.write('>Event Overview (cols: lon, lat, OT (s), ElarmS mag, PGD mag, PGD variance reduction, PGD Depth, PGD no. stations, G-FAST alert time (s))'+'\n')
		f.write(str(SA_lon)+' '+str(SA_lat)+' '+str(SA_time)+' '+str(SA_mag)+' '+mpgdstr+' '+vrpgdstr+' '+str(dep_vr_pgd)+' '+str(len(a1))+' '+str(runtime)+'\n')
		f.write('>PGD Records'+'\n')
		ind = 0
		while ind < len(a1):
			edist = "{0:.2f}".format(float(epidist[a1[ind]])/1000)
			md = "{0:.2f}".format(float(maxD[ind])*100)
			f.write(edist+' '+md+'\n')
			ind=ind+1
		f.write('>'+'\n')
		f.close()
		#else:
			#mpgd_vr = numpy.zeros([100,1])
			#mpgd = numpy.zeros([100,1])
	else:
		mpgd_vr = numpy.zeros([100,1])
		mpgd = numpy.zeros([100,1])

	return(mpgd,mpgd_vr,len(a1))


def data_engine_cmtff(SA_lat,SA_lon,SA_dep,SA_time,SA_mag,SA_eventid,sta_lat,sta_lon,nbuffnew,ebuffnew,ubuffnew,runtime):
	
	fname = 'events/GFAST_CMT_' + str(int(SA_eventid)) + '.txt' 
	f = open(fname,'a') #open gps data file to append data
	(x1,y1) = ll2utm(SA_lon,SA_lat,-123.0)
	(x2,y2) = ll2utm(sta_lon,sta_lat,-123.0)
	epidist = numpy.sqrt(numpy.power(x1-x2,2)+numpy.power(y1-y2,2))

	
	a1 = numpy.where(epidist/1000 < (runtime-SA_time)*2+10)[0]
	a1 = numpy.array(a1)

	if len(a1) > 3:
		
		mcmt_vr = numpy.zeros([100,1])
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
		mcmt = numpy.zeros([100,1])

		#Depth Search
		dep = 1
		while dep < 101:
	
			[S,VR,Mw,NP1,NP2] = moment_tensor(sta_lat[a1],sta_lon[a1],nbuffnew[a1,:],ebuffnew[a1,:],ubuffnew[a1,:],SA_lat,SA_lon,dep+0.1,runtime,epidist)
			mcmt_vr[dep-1,0] = VR
			S1[dep-1,0] = S[0]
			S2[dep-1,0] = S[1]
			S3[dep-1,0] = S[2]
			S4[dep-1,0] = S[3]
			S5[dep-1,0] = S[4]
			S6[dep-1,0] = S[5]
			STR1[dep-1,0] = NP1['strike']
			STR2[dep-1,0] = NP2['strike']
			DIP1[dep-1,0] = NP1['dip']
			DIP2[dep-1,0] = NP2['dip']
			RAK1[dep-1,0] = NP1['rake']
			RAK2[dep-1,0] = NP2['rake']
			mcmt[dep-1,0] = Mw
			dep = dep+1
			
		dep_vr_cmt = numpy.argmax(mcmt_vr)
		mcmtstr = "{0:.2f}".format(float(mcmt[dep_vr_cmt]))
		vrcmtstr = "{0:.2f}".format(float(mcmt_vr[dep_vr_cmt]))
		str1 = "{0:.2f}".format(float(STR1[dep_vr_cmt]))
		str2 = "{0:.2f}".format(float(STR2[dep_vr_cmt]))
		dip1 = "{0:.2f}".format(float(DIP1[dep_vr_cmt]))
		dip2 = "{0:.2f}".format(float(DIP2[dep_vr_cmt]))
		rak1 = "{0:.2f}".format(float(RAK1[dep_vr_cmt]))
		rak2 = "{0:.2f}".format(float(RAK2[dep_vr_cmt]))
		print mcmtstr, dep_vr_cmt

		f.write('>Event Overview (cols: lon, lat, OT (s), ElarmS mag, CMT mag, CMT variance reduction, CMT Depth, FP1 STR, FP1 DIP, FP1 RAKE, FP2 STR, FP2 DIP, FP2 RAKE,CMT no. stations, G-FAST alert time (s))'+'\n')
		f.write(str(SA_lon)+' '+str(SA_lat)+' '+str(SA_time)+' '+str(SA_mag)+' '+mcmtstr+' '+vrcmtstr+' '+str(dep_vr_cmt)+' '+str1+' '+dip1+' '+rak1+' '+str2+' '+dip2+' '+rak2+' '+str(len(a1))+' '+str(runtime)+'\n')

		f.write('>'+'\n')
		f.close()

	else:
		mcmt_vr = numpy.zeros([100,1])
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
		mcmt = numpy.zeros([100,1])

	return(MW,S1,S2,S3,S4,S5,S6,STR1,STR2,DIP1,DIP2,RAK1,RAK2,VARRED,len(a1))

		
