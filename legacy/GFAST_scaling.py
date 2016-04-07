#!/usr/bin/python
#This code outputs the predicted magnitude using PGD and Pd from seismogeodetic data.
import math
import numpy

def PGD(d, r, repi):

	A = -6.687
	B = 1.500
	C = -0.214
	#A = -4.434
	#B = 1.047
	#C = -0.138
	d[d-6.0 < 0] = 0.01

	W = numpy.exp(-numpy.power(repi,2)/8/numpy.power(min(repi),2))

	G = B+C*(numpy.log10(r))
	b = numpy.log10(d)-A
	M = numpy.linalg.lstsq(W*G,W*b)[0]

	
	UP = numpy.dot(G,M)
	VR = (1.0-numpy.linalg.norm(numpy.power(10,b)-numpy.power(10,UP))/numpy.linalg.norm(numpy.power(10,b)))*100
	
	return(M,VR)



