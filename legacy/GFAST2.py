#!/usr/bin/python
import time
import datetime
import math
import os
import numpy
from GFAST_buffer import bufferupdatenew, bufferinitnew
from GFAST_locinit import location
from GFAST_paraminit import Properties
from GFAST_alertReader import AMQAlertClient, printer
from GFAST_eewdataengine import data_engine_pgd, data_engine_cmtff
from GFAST_emailer import emailer
import sys
import pika
import json
#############################################################################################################

def callback(ch, method, properties, body):
	global tindex, nbuff, ebuff, ubuff, tbuff, sites	
	data = json.loads(body)
	v = data['v'] #vertical
	n = data['n'] #north
	e = data['e'] #east
	t = data['t'] #unix time of record
	site = data['site']
	#print t,site

	indsta = numpy.where(sites == site)[0]

	if (len(indsta) > 0):
		[nbuff,ebuff,ubuff,tbuff]=bufferupdatenew(indsta,site,nbuff,ebuff,ubuff,tbuff,t,n,e,v,props)

		
	if (time.time()-tindex > 1):
		gfast_monitor()
		tindex = time.time()





def gfast_monitor():
	global eventidlast
	
	a = numpy.where(numpy.nansum(~numpy.isnan(nbuff), axis=1) > 180)[0]#If there is more than 3 minutes of data, use the station
	
	utc_now = datetime.datetime.utcnow()
	utc_ts  = float(utc_now.strftime("%s"))
	
	eewfile = props.geteewsfile()
	for line in open(eewfile,'r'):
    		cols = line.rstrip()
		cols = line.split()
		SA_eventid = cols[0]
		SA_eventday = cols[1]
		SA_eventtime = cols[2]
		SA_mag = cols[3]
		SA_lon = cols[4]
		SA_lat = cols[5]


	SA_time = time.mktime(datetime.datetime.strptime(SA_eventday+'-'+SA_eventtime, "%Y-%m-%d-%H:%M:%S.%f").timetuple())
	SA_time = float(SA_time)
	SA_lat = float(SA_lat)
	SA_lon = float(SA_lon)

	if (utc_ts - SA_time < 300):
		print utc_ts-SA_time, len(a)

		OUTPUT_PGD=data_engine_pgd(SA_lat,SA_lon,8.0,SA_time,SA_mag,SA_eventid,sta_lat[a],sta_lon[a],nbuff[a,:],ebuff[a,:],ubuff[a,:],utc_ts)
		MPGD = OUTPUT_PGD[0] #PGD Magnitudes as function of depth
		VR_PGD = OUTPUT_PGD[1] #Variance reduction for PGD as function of depth
		LEN_PGD = OUTPUT_PGD[2] #Number of stations used for PGD calculation
		dep_vr_pgd = numpy.argmax(VR_PGD)

		OUTPUT_CMTFF = data_engine_cmtff(SA_lat,SA_lon,8.0,SA_time,SA_mag,SA_eventid,sta_lat[a],sta_lon[a],nbuff[a,:],ebuff[a,:],ubuff[a,:],utc_ts)
		

		#MCMT = OUTPUT_CMTFF[0]
		#VR_CMT = OUTPUT_CMTFF[13]
		#LEN_CMT = OUTPUT_CMTFF[14]
		#dep_vr_cmt = numpy.argmax(VR_CMT)
		#print 'CMT Results'
		#print MCMT[dep_vr_cmt], VR_CMT[dep_vr_cmt],dep_vr_cmt,LEN_CMT
		#print 'PGD Results'
		#print MPGD[dep_vr_pgd], VR_PGD[dep_vr_pgd], dep_vr_pgd, LEN_PGD
		mpgd = "{0:.2f}".format(float(MPGD[dep_vr_pgd]))
		vrpgd = "{0:.2f}".format(float(VR_PGD[dep_vr_pgd]))
		lenpgd = str(LEN_PGD)
		#print mpgd, vrpgd, lenpgd
		f = open(props.geteewgfile(),'a')
		f.write(str(SA_eventid)+' '+str(SA_lon)+' '+str(SA_lat)+' '+str(SA_time)+' '+str(SA_mag)+' '+mpgd+' '+vrpgd+' '+str(dep_vr_pgd)+' '+lenpgd+' '+str(utc_ts)+'\n')
		f.close()

		
	if (utc_ts - SA_time > 300):
		if (eventidlast < float(SA_eventid)):
			emailer(SA_eventid)
			eventidlast = float(SA_eventid)


	
	


##############################################################################################################
#Start Program
##############################################################################################################
eventidlast = 2242
props=Properties('gfast.props')
client=AMQAlertClient(props)
client.connect(printer)

[sta_lat,sta_lon,sites]=location(props)
[nbuff,ebuff,ubuff,tbuff]=bufferinitnew(props)


credentials = pika.PlainCredentials(props.getRMQuser(),props.getRMQpassword())
parameters = pika.ConnectionParameters(props.getRMQtopic(),props.getRMQport(),props.getRMQhost(),credentials=credentials)
connection = pika.BlockingConnection(parameters)

channel = connection.channel()
channel.exchange_declare(exchange=props.getRMQexchange(),passive=True)
result = channel.queue_declare(exclusive=True)
queue_name = result.method.queue
channel.queue_bind(exchange=props.getRMQexchange(),queue=queue_name)

channel.basic_consume(callback,queue=queue_name,no_ack=True)


if (int(props.getmode()) == 0):
	while True:
		tindex = time.time()
		try:
			channel.start_consuming()
		except:
			pass

		
		

		
