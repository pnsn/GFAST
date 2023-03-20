#!/usr/bin/python
#import json
#import urllib
import time
import math
import numpy
import os
from GFAST_paraminit import Properties



#Initializes the data buffer
def bufferinit(props):
	streamfile=props.getstreamfile()
        siteposfile=props.getsiteposfile()
        bufflen=props.getbufflen()
	stream_length = props.getstreamlength()

	nbuff = numpy.nan*(numpy.ones([int(stream_length),int(bufflen)]))
	ebuff = numpy.nan*(numpy.ones([int(stream_length),int(bufflen)]))
	ubuff = numpy.nan*(numpy.ones([int(stream_length),int(bufflen)]))
	tbuff = numpy.zeros([1,int(bufflen)])

	tcurrent = time.time()
	for i in range (0, int(bufflen)):
		tbuff[0,i] = int(tcurrent)-i

	lrbuff = int(tcurrent-int(bufflen))*numpy.ones([int(stream_length),1])

	indsta=0
	for line in open(streamfile,'r'):# File of streams to use. See AvailableStreams.py to find possibilities
		cols = line.rstrip()
		cols = line.split()
		gps_stream = cols[0]
		state_url = 'http://www.panga.org/realtime/data/api/state/' + gps_stream # URL to check the state of an individual stream
		try:
			stateread = requests.get(state_url, timeout=0.1)
			stream_state = stateread.text
			if stream_state in ['active']: # If the stream is active, read the stream
				lrs = str(int(tcurrent-int(bufflen))) #convert to string to use in json call	
				data_url = 'http://www.panga.org/realtime/data/api/records/' + gps_stream + '?l=' + lrs #URL to download data for each station 
				try:
					data = requests.get(data_url, timeout=0.1)
					data_dump = data.json()
					if (len(data_dump) == 5):
						recs = data_dump["recs"] #the data. includes time stamps, displacements, sigmas
						final_rec = data_dump['last'] #this is the last record time of the json stream
						rec_length = len(recs) #length of record
			
						ind = 0
						while ind < rec_length: #read in data from recs
							v = recs[ind]['v'] #vertical
							n = recs[ind]['n'] #north
							e = recs[ind]['e'] #east

							t = recs[ind]['t'] #unix time of record

							a= numpy.where(tbuff[0,0:int(bufflen)] == int(t))
			
							nbuff[indsta,a] = n
							ebuff[indsta,a] = e
							ubuff[indsta,a] = v


							ind = ind+1
				except:
					pass
		except:
			pass
		indsta = indsta+1


	return (nbuff, ebuff, ubuff, tbuff, lrbuff)


#This updates the buffer every epoch

def bufferupdate(nbuff,ebuff,ubuff,tbuff,lrbuff,props):
	streamfile=props.getstreamfile()
        bufflen=props.getbufflen()
	stream_length = props.getstreamlength()

	tcurrent = time.time()
	tbuffmax = numpy.amax(tbuff)
	tdiff = int(tcurrent-tbuffmax)

	nbuffnew = numpy.nan*(numpy.ones([int(stream_length),tdiff]))
	ebuffnew = numpy.nan*(numpy.ones([int(stream_length),tdiff]))
	ubuffnew = numpy.nan*(numpy.ones([int(stream_length),tdiff]))

	tbuffnew = numpy.zeros([1,tdiff])

	for i in range (0, tdiff):
		tbuffnew[0,i] = int(tcurrent)-i

	tbuff = numpy.concatenate((tbuffnew,tbuff),axis=1)
	nbuff = numpy.concatenate((nbuffnew,nbuff),axis=1)
	ebuff = numpy.concatenate((ebuffnew,ebuff),axis=1)
	ubuff = numpy.concatenate((ubuffnew,ubuff),axis=1)


	tbuff = tbuff[:,:int(bufflen)]
	nbuff = nbuff[:,:int(bufflen)]
	ebuff = ebuff[:,:int(bufflen)]
	ubuff = ubuff[:,:int(bufflen)]


	indsta=0
	for line in open(streamfile,'r'):# File of streams to use. See AvailableStreams.py to find possibilities
		cols = line.rstrip()
		cols = line.split()
		gps_stream = cols[0]
		state_url = 'http://www.panga.org/realtime/data/api/state/' + gps_stream # URL to check the state of an individual stream
		#try:
		#	stateread = requests.get(state_url, timeout = 0.05)
		#	stream_state = stateread.text
			#if stream_state in ['active']: # If the stream is active, read the stream
		last_rec = lrbuff[indsta,0]
		lrs = str(int(last_rec))
		data_url = 'http://www.panga.org/realtime/data/api/records/' + gps_stream + '?l=' + lrs #URL to download data for each station 
		try:
			data = requests.get(data_url,timeout= 0.02)
			
			data_dump = data.json()
				
			if (len(data_dump) == 5):
				recs = data_dump['recs'] #the data. includes time stamps, displacements, sigmas
				final_rec = data_dump['last'] #this is the last record time of the json stream
				rec_length = len(recs) #length of record
			
				ind = 0
				while ind < rec_length: #read in data from recs
					v = recs[ind]['v'] #vertical
					n = recs[ind]['n'] #north
					e = recs[ind]['e'] #east
					t = recs[ind]['t'] #unix time of record
				

					a= numpy.where(tbuff[0,0:int(bufflen)] == int(t))
					nbuff[indsta,a] = n
					ebuff[indsta,a] = e
					ubuff[indsta,a] = v

					n = format(float(n), '.5f') #convert the n, e, v records to string with 5 decimals
					e = format(float(e), '.5f') 
					v = format(float(v), '.5f')


					tgps = int(t) - 315964800 #convert unix time to gps second. GPS time is seconds from 0h on Jan 6th, 1980. Unix time from 0h on Jan 1st, 1970
					tweek = math.floor(tgps/604800) #find the gps week
					tday = math.ceil((tgps-tweek*604800)/86400) #find the gps day of the week
					tsec = tgps-tweek*604800-(tday-1)*86400 #find the gps second of the day
					tweek = int(tweek) #convert to integers, then strings
					tweek = str(tweek)
					tday = int(tday)
					tday = str(tday)
					

					if not os.path.exists('data/' + tweek + '/' + tday): #if folder does not exist for storing data, create it. subfolders are gps week/ gps day
   						os.makedirs('data/' + tweek + '/' + tday)

					fname = 'data/' + tweek + '/' + tday + '/' + gps_stream + '_' + tweek + '_' + tday + '.txt' #single file for each station in proper subfolder
					f = open(fname,'a') #open gps data file to append data
					ind=ind+1	
					if int(t) > int(last_rec):
						f.write(str(int(tsec)) + ' ' + n + ' ' + e + ' ' + v + '\n')
					f.close()

				lrbuff[indsta,0] = final_rec
		except:
			pass
		
		#except:
		#	pass
		indsta=indsta+1

	return (nbuff, ebuff, ubuff, tbuff, lrbuff)

def bufferupdatenew(indsta,site,nbuff,ebuff,ubuff,tbuff,t,n,e,v,props):
        bufflen=props.getbufflen()
	stream_length = props.getstreamlength()

	if (int(t) > tbuff[0,0]):
		
		tdiff = int(t-tbuff[0,0])
		tbuffnew = numpy.zeros([1,tdiff])

		nbuffnew = numpy.nan*(numpy.ones([int(stream_length),tdiff]))
		ebuffnew = numpy.nan*(numpy.ones([int(stream_length),tdiff]))
		ubuffnew = numpy.nan*(numpy.ones([int(stream_length),tdiff]))

		for i in range (0, tdiff):
			tbuffnew[0,i] = int(t)-i

		tbuff = numpy.concatenate((tbuffnew,tbuff),axis=1)
		nbuff = numpy.concatenate((nbuffnew,nbuff),axis=1)
		ebuff = numpy.concatenate((ebuffnew,ebuff),axis=1)
		ubuff = numpy.concatenate((ubuffnew,ubuff),axis=1)

		tbuff = tbuff[:,:int(bufflen)]
		nbuff = nbuff[:,:int(bufflen)]
		ebuff = ebuff[:,:int(bufflen)]
		ubuff = ubuff[:,:int(bufflen)]
			
	a = numpy.where(tbuff[0,0:int(bufflen)] == int(t))
	
	nbuff[indsta,a] = n
	ebuff[indsta,a] = e
	ubuff[indsta,a] = v


	n = format(float(n), '.5f') #convert the n, e, v records to string with 5 decimals
	e = format(float(e), '.5f') 
	v = format(float(v), '.5f')


	tgps = int(t) - 315964800 #convert unix time to gps second. GPS time is seconds from 0h on Jan 6th, 1980. Unix time from 0h on Jan 1st, 1970
	tweek = math.floor(tgps/604800) #find the gps week
	tday = math.floor((tgps-tweek*604800)/86400)+1 #find the gps day of the week
	tsec = tgps-tweek*604800-(tday-1)*86400 #find the gps second of the day
	tweek = int(tweek) #convert to integers, then strings
	tweek = str(tweek)
	tday = int(tday)
	tday = str(tday)
					

	if not os.path.exists('data/' + tweek + '/' + tday): #if folder does not exist for storing data, create it. subfolders are gps week/ gps day
   		os.makedirs('data/' + tweek + '/' + tday)

	fname = 'data/' + tweek + '/' + tday + '/' + str(site) + '_' + tweek + '_' + tday + '.txt' #single file for each station in proper subfolder
	f = open(fname,'a') #open gps data file to append data

	f.write(str(int(tsec)) + ' ' + n + ' ' + e + ' ' + v + '\n')
	f.close()
	return(nbuff,ebuff,ubuff,tbuff)

def bufferinitnew(props):
	streamfile=props.getstreamfile()
        siteposfile=props.getsiteposfile()
        bufflen=props.getbufflen()
	stream_length = props.getstreamlength()

	nbuff = numpy.nan*(numpy.ones([int(stream_length),int(bufflen)]))
	ebuff = numpy.nan*(numpy.ones([int(stream_length),int(bufflen)]))
	ubuff = numpy.nan*(numpy.ones([int(stream_length),int(bufflen)]))
	tbuff = numpy.zeros([1,int(bufflen)])

	tcurrent = time.time()
	for i in range (0, int(bufflen)):
		tbuff[0,i] = int(tcurrent)-i


	return (nbuff, ebuff, ubuff, tbuff)
	

