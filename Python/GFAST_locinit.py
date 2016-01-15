#!/usr/bin/python
import time
import math
import numpy
import csv

def location(props):

	streamfile=props.getstreamfile()
        siteposfile=props.getsiteposfile()
	stream_length = props.getstreamlength()

	LAT = numpy.zeros([int(stream_length),1])
	LON = numpy.zeros([int(stream_length),1])
	SITES = numpy.array( range(int(stream_length)), dtype='a5').reshape(int(stream_length),1)

	k=0
	for line in open(streamfile,'r'):
		cols = line.rstrip()
		site = cols[0:4]
		with open(siteposfile, 'rt') as f:
     			reader = csv.reader(f, delimiter=',')
     			for row in reader:
          			if site.lower() == row[0]: 
              				LAT[k,0] = row[8]
					LON[k,0] = row[9]
					SITES[k,0] = site
		k=k+1

	return (LAT, LON, SITES)




