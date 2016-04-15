#!/usr/bin/env python
import math
import numpy
import matplotlib.pyplot as plt
import csv
from coord_tools import ll2utm
from scaling import PGD
from buffer_init_synthetic import data_buff
from location_init import location
from eew_data_engine_synthetic import data_engine_pgd, data_engine_cmt, data_engine_ff
from fault_plane import fault_CMT
from RTOkada import rtokada
#############################################
#Test Run Parameters
#############################################
#Nisqually Parameters
stafile = 'illapel_stations.txt' #list of stations used python
datadir = 'illapeldata/' #directory of all SAC files
datarate = 1 #time between samples
ndata = 601 #number of data points
elarmsfile = 'illapel_loc.txt' #file of elarms parameters
fname = 'illapel_optimal_cmtloc.txt' #output file of all depth results
fname2 = 'illapel_optimal_slipmodels_cmtloc.txt' #output file of slip models
fname3 = 'illapel_optimal_slipfits_cmtloc.txt' #output file of gps vector fits
eq_dep = 8 #EQ depth - ElarmS always assumes the same depth of 8 km
initt = 0.0 #initial time offset of time series
nstr = 10 #number of along strike fault components
ndip = 5 #number of along dip fault components
data_prefix = 'LX' #The sac file data prefix to identify rate and station type
#############################################

Tend = math.floor(ndata*datarate)#Final time
tbuff = numpy.c_[0:ndata*datarate:datarate] #time buffer - normal done in data initialization, but definition is different from files
[sta_lat,sta_lon,sta_alt] = location(stafile) #Location initialization

[nbuff, ebuff, ubuff, nebuff, eebuff, uebuff] = data_buff(stafile, ndata, datadir, data_prefix) #Data buffer initialization

f = open(fname,'w')
f1 = open(fname2,'w')
f2 = open(fname3,'w')


def syntest(Tlatency,ind,anump):
	eq_lat = -9999
	eq_lon = -9999
	nbuffnew = nbuff #To simulate ideal
	ebuffnew = ebuff
	ubuffnew = ubuff
	#nbuffnew = nbuff+numpy.random.normal(0,0.0039,size=(26,2048)) #To simulate noise
	#ebuffnew = ebuff+numpy.random.normal(0,0.0027,size=(26,2048))
 	#ubuffnew = ubuff+numpy.random.normal(0,0.0072,size=(26,2048))
	#nbuffnewpgd = numpy.where(anump > 85,nbuffnew,0) #To simulate dropouts
	#ebuffnewpgd = numpy.where(anump > 85,ebuffnew,0)
	#ubuffnewpgd = numpy.where(anump > 85,ubuffnew,0)

	#nbuffnewcmt = numpy.where(anump > 85,nbuffnew,-999) #To simulate dropouts
	#ebuffnewcmt = numpy.where(anump > 85,ebuffnew,-999)
	#ubuffnewcmt = numpy.where(anump > 85,ubuffnew,-999)

	nbuffnewpgd = nbuffnew 
	ebuffnewpgd = ebuffnew
	ubuffnewpgd = ubuffnew

	nbuffnewcmt = nbuffnew
	ebuffnewcmt = ebuffnew
	ubuffnewcmt = ubuffnew

	
	runtime = 299
	while runtime < 300:
		with open(elarmsfile, 'rt') as elar:
  			reader = csv.reader(elar, delimiter=',')
   			for row in reader:
          			if runtime > float(row[0]): 
             				eq_lat = float(row[3])
					eq_lon = float(row[4])
					to = float(row[1])+initt
		
	
		if eq_lat > -9999:
			OUTPUT_PGD = data_engine_pgd(eq_lat,eq_lon,eq_dep,to,sta_lat,sta_lon,sta_alt,nbuffnewpgd,ebuffnewpgd,ubuffnewpgd,tbuff,runtime,Tlatency)
			MPGD = OUTPUT_PGD[0] #PGD Magnitudes as function of depth
			VR_PGD = OUTPUT_PGD[1] #Variance reduction for PGD as function of depth
			LEN_PGD = OUTPUT_PGD[2] #Number of stations used for PGD calculation
			
			dep_vr_pgd = numpy.argmin(VR_PGD) #Depth of greatest variance reduction, PGD

			OUTPUT_CMT = data_engine_cmt(eq_lat,eq_lon,eq_dep,to,sta_lat,sta_lon,sta_alt,nbuffnewcmt,ebuffnewcmt,ubuffnewcmt,tbuff,runtime,Tlatency)
			MCMT = OUTPUT_CMT[0] #CMT Magnitudes as function of depth
			S1 = OUTPUT_CMT[1] #The 6 moment tensor components as function of depth
			S2 = OUTPUT_CMT[2]
			S3 = OUTPUT_CMT[3]
			S4 = OUTPUT_CMT[4]
			S5 = OUTPUT_CMT[5]
			S6 = OUTPUT_CMT[6]
			STR1 = OUTPUT_CMT[7] #The strike, dip and rakes of the main and auxiliary fault planes as a function of depth
			STR2 = OUTPUT_CMT[8]
			DIP1 = OUTPUT_CMT[9]
			DIP2 = OUTPUT_CMT[10]
			RAK1 = OUTPUT_CMT[11]
			RAK2 = OUTPUT_CMT[12]
			VR_CMT = OUTPUT_CMT[13] #Variance reduction for CMT as function of depth
			LEN_CMT = OUTPUT_CMT[14] #Number of stations used for CMT calculation




			
			dep_vr_cmt = numpy.argmax(VR_CMT) #Depth of greatest variance reduction, CMT

			
			if (LEN_CMT > 100):
				str1 = STR1[dep_vr_cmt]
				dip1 = DIP1[dep_vr_cmt]
				str2 = STR2[dep_vr_cmt]
				dip2 = DIP2[dep_vr_cmt]
				mcmt = MCMT[dep_vr_cmt]


				OUTPUT_FF=data_engine_ff(eq_lat,eq_lon,dep_vr_cmt,mcmt,str1,str2,dip1,dip2,nstr,ndip,to,sta_lat,sta_lon,sta_alt,nbuffnewcmt,ebuffnewcmt,ubuffnewcmt,tbuff,runtime,Tlatency)
				SSLIP = OUTPUT_FF[0] #Strike-slip along each fault patch
				DSLIP = OUTPUT_FF[1] #Dip-slip along each fault patch
				MFF = OUTPUT_FF[2] #Finite fault magnitude
				E = OUTPUT_FF[3] #Input east displacements
				N = OUTPUT_FF[4] #Input north displacements
				U = OUTPUT_FF[5] #Input up displacements
				EN = OUTPUT_FF[6] #Modeled east displacements
				NN = OUTPUT_FF[7] #Modeled north displacements
				UN = OUTPUT_FF[8] #Modeled up displacements
				STA_LAT = OUTPUT_FF[9] #Station latitudes used in inversion
				STA_LON = OUTPUT_FF[10] #Station longitudes used in inversion
				FAULT_LAT = OUTPUT_FF[11]
				FAULT_LON = OUTPUT_FF[12]
				FAULT_ALT = OUTPUT_FF[13]
				VR_FF1 = OUTPUT_FF[14] #Variance reduction of finite fault inversion for fault plane 1
				VR_FF2 = OUTPUT_FF[15] #Variance reduction of finite fault inversion for fault plane 2
				FaultPlane = OUTPUT_FF[16] #Preferred fault plane, 1 or 2
				FLAT1 = OUTPUT_FF[17]
				FLON1 = OUTPUT_FF[18]
				FLAT2 = OUTPUT_FF[19]
				FLON2 = OUTPUT_FF[20]
				FLAT3 = OUTPUT_FF[21]
				FLON3 = OUTPUT_FF[22]
				FLAT4 = OUTPUT_FF[23]
				FLON4 = OUTPUT_FF[24]

				l1 = len(SSLIP)
				for i in range (0,l1):
					sslip = "{0:.4f}".format(float(SSLIP[i]))
					dslip = "{0:.4f}".format(float(DSLIP[i]))
					flat = "{0:.4f}".format(float(FAULT_LAT[i]))
					flon = "{0:.4f}".format(float(FAULT_LON[i]))
					falt = "{0:.4f}".format(float(FAULT_ALT[i]))
					mff = "{0:.4f}".format(float(MFF))
					vrff1 = "{0:.4f}".format(float(VR_FF1))
					vrff2 = "{0:.4f}".format(float(VR_FF2))
					flat1 = "{0:.4f}".format(float(FLAT1[i]))
					flon1 = "{0:.4f}".format(float(FLON1[i]))
					flat2 = "{0:.4f}".format(float(FLAT2[i]))
					flon2 = "{0:.4f}".format(float(FLON2[i]))
					flat3 = "{0:.4f}".format(float(FLAT3[i]))
					flon3 = "{0:.4f}".format(float(FLON3[i]))
					flat4 = "{0:.4f}".format(float(FLAT4[i]))
					flon4 = "{0:.4f}".format(float(FLON4[i]))
					f1.write(str(ind)+' '+str(runtime)+' '+str(FaultPlane)+' '+flon+' '+flat+' '+falt+' '+sslip+' '+dslip+' '+mff+' '+vrff1+' '+vrff2+' '+flat1+' '+flon1+' '+flat2+' '+flon2+' '+flat3+' '+flon3+' '+flat4+' '+flon4+'\n')

				l1 = len(E)
				for i in range (0,l1):
					e = "{0:.4f}".format(float(E[i]))
					n = "{0:.4f}".format(float(N[i]))
					u = "{0:.4f}".format(float(U[i]))
					en = "{0:.4f}".format(float(EN[i]))
					nn = "{0:.4f}".format(float(NN[i]))
					un = "{0:.4f}".format(float(UN[i]))
					slat = "{0:.4f}".format(float(STA_LAT[i]))
					slon = "{0:.4f}".format(float(STA_LON[i]))
					f2.write(str(ind)+' '+str(runtime)+' '+slon+' '+slat+' '+e+' '+n+' '+u+' '+en+' '+nn+' '+un+'\n')


			

			vrcmt = "{0:.2f}".format(float(VR_CMT[dep_vr_cmt]))
			s1 = "{0:.4e}".format(float(S1[dep_vr_cmt]))
			s2 = "{0:.4e}".format(float(S2[dep_vr_cmt]))
			s3 = "{0:.4e}".format(float(S3[dep_vr_cmt]))
			s4 = "{0:.4e}".format(float(S4[dep_vr_cmt]))
			s5 = "{0:.4e}".format(float(S5[dep_vr_cmt]))
			s6 = "{0:.4e}".format(float(S6[dep_vr_cmt]))
			str1 = "{0:.2f}".format(float(STR1[dep_vr_cmt]))
			str2 = "{0:.2f}".format(float(STR2[dep_vr_cmt]))
			dip1 = "{0:.2f}".format(float(DIP1[dep_vr_cmt]))
			dip2 = "{0:.2f}".format(float(DIP2[dep_vr_cmt]))
			rak1 = "{0:.2f}".format(float(RAK1[dep_vr_cmt]))
			rak2 = "{0:.2f}".format(float(RAK2[dep_vr_cmt]))
			mcmt = "{0:.2f}".format(float(MCMT[dep_vr_cmt]))
			mpgd = "{0:.2f}".format(float(MPGD[dep_vr_pgd]))
			vrpgd = "{0:.2f}".format(float(VR_PGD[dep_vr_pgd]))
			lenpgd = str(LEN_PGD)
			lencmt = str(LEN_CMT)	
			f.write(str(ind)+' '+str(runtime)+' '+str(dep_vr_pgd)+' '+vrpgd+' '+mpgd+' '+lenpgd+' '+str(dep_vr_cmt)+' '+vrcmt+' '+mcmt+' '+s1+' '+s2+' '+s3+' '+s4+' '+s5+' '+s6+' '+str1+' '+str2+' '+dip1+' '+dip2+' '+rak1+' '+rak2+' '+lencmt+'\n')

		else:

			Na = str(float('NaN'))
			f.write(str(ind)+' '+str(runtime)+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+' '+Na+'\n')
				
		runtime = runtime+1

	return()


ind=0
while ind < 1:
	#Tlatency = numpy.random.poisson(6,size=(len(sta_lat),1)) #To simulate latency
	Tlatency = numpy.zeros([len(sta_lat),1]) #To simulate no latency
	anump = numpy.random.randint(100,size=(26,2048)) #To simulate dropouts
	syntest(Tlatency,ind,anump)
	ind = ind+1

f.close()
f1.close()
f2.close()







