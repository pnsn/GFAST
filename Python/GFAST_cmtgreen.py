#!/usr/bin/python
import math
import numpy
import obspy.signal

MU = 3e10
MU = MU
K = 5*MU/3


def greenF(x1, y1, z1, azi):
	l1=len(x1)
	G = numpy.zeros([3*l1,6])
	for i in range (0, l1):
		azi1 = math.radians(azi[i])
		x = x1[i]
		y = y1[i]
		#[x,y] = obspy.signal.rotate.rotate_NE_RT(numpy.array([ya]),numpy.array([xa]),azi[i])
		z = z1[i]

		R = math.sqrt(math.pow(x,2)+math.pow(y,2)+math.pow(z,2))
		C1= 1/math.pow(R,2)/MU/math.pi/8


		C2 = (3*K+MU)/(3*K+4*MU)
	
		

		g111 = C1*( C2*(3*x*x*x/math.pow(R,3) - 3*x/R) + 2*x/R )
		g122 = C1*( C2*(3*x*y*y/math.pow(R,3) - x/R) )
		g133 = C1*( C2*(3*x*z*z/math.pow(R,3) - x/R) )
		g112 = C1*( C2*(3*x*x*y/math.pow(R,3) - y/R) + 2*y/R )
		g113 = C1*( C2*(3*x*x*z/math.pow(R,3) - z/R) + 2*z/R )
		g123 = C1*( C2*(3*x*y*z/math.pow(R,3)) )

		g211 = C1*( C2*(3*y*x*x/math.pow(R,3) - y/R) ) 
		g222 = C1*( C2*(3*y*y*y/math.pow(R,3) - 3*y/R) + 2*y/R )
		g233 = C1*( C2*(3*y*z*z/math.pow(R,3) - y/R) )
		g212 = C1*( C2*(3*y*x*y/math.pow(R,3) - x/R) )
		g213 = C1*( C2*(3*y*x*z/math.pow(R,3)) )
		g223 = C1*( C2*(3*y*y*z/math.pow(R,3) - z/R) + 2*z/R )

		g311 = C1*( C2*(3*z*x*x/math.pow(R,3) - z/R) )
		g322 = C1*( C2*(3*z*y*y/math.pow(R,3) - z/R) )
		g333 = C1*( C2*(3*z*z*z/math.pow(R,3) - 3*z/R) + 2*z/R )
		g312 = C1*( C2*(3*z*x*y/math.pow(R,3)) )
		g313 = C1*( C2*(3*z*x*z/math.pow(R,3)) +2*x/R)
		g323 = C1*( C2*(3*z*y*z/math.pow(R,3)) +2*y/R)
		
		G[3*i,0] = g111
		G[3*i,1] = g122
		G[3*i,2] = g133
		G[3*i,3] = g112
		G[3*i,4] = g113
		G[3*i,5] = g123

		G[3*i+1,0] = g211
		G[3*i+1,1] = g222
		G[3*i+1,2] = g233
		G[3*i+1,3] = g212
		G[3*i+1,4] = g213
		G[3*i+1,5] = g223

		G[3*i+2,0] = g311
		G[3*i+2,1] = g322
		G[3*i+2,2] = g333
		G[3*i+2,3] = g312
		G[3*i+2,4] = g313
		G[3*i+2,5] = g323

		G[3*i,0] = g111*math.pow(math.cos(azi1),2)+g122*math.pow(math.sin(azi1),2)+2*g112*math.cos(azi1)*math.sin(azi1)
		G[3*i,1] = g111*math.pow(math.sin(azi1),2)+g122*math.pow(math.cos(azi1),2)-2*g112*math.cos(azi1)*math.sin(azi1)
		G[3*i,2] = g133
		G[3*i,3] = -g111*math.sin(azi1)*math.cos(azi1)+g112*math.pow(math.cos(azi1),2)-g112*math.pow(math.sin(azi1),2)+g122*math.sin(azi1)*math.cos(azi1)
		G[3*i,4] = g113*math.cos(azi1)+g123*math.sin(azi1)
		G[3*i,5] = g123*math.cos(azi1)-g113*math.sin(azi1)

		G[3*i+1,0] = g211*math.pow(math.cos(azi1),2)+g222*math.pow(math.sin(azi1),2)+2*g212*math.cos(azi1)*math.sin(azi1)
		G[3*i+1,1] = g211*math.pow(math.sin(azi1),2)+g222*math.pow(math.cos(azi1),2)-2*g212*math.cos(azi1)*math.sin(azi1)
		G[3*i+1,2] = g233
		G[3*i+1,3] = -g211*math.sin(azi1)*math.cos(azi1)+g212*math.pow(math.cos(azi1),2)-g212*math.pow(math.sin(azi1),2)+g222*math.sin(azi1)*math.cos(azi1)
		G[3*i+1,4] = g213*math.cos(azi1)+g223*math.sin(azi1)
		G[3*i+1,5] = g223*math.cos(azi1)-g213*math.sin(azi1)

		G[3*i+2,0] = g311*math.pow(math.cos(azi1),2)+g322*math.pow(math.sin(azi1),2)+2*g312*math.cos(azi1)*math.sin(azi1)
		G[3*i+2,1] = g311*math.pow(math.sin(azi1),2)+g322*math.pow(math.cos(azi1),2)-2*g312*math.cos(azi1)*math.sin(azi1)
		G[3*i+2,2] = g333
		G[3*i+2,3] = -g311*math.sin(azi1)*math.cos(azi1)+g312*math.pow(math.cos(azi1),2)-g312*math.pow(math.sin(azi1),2)+g322*math.sin(azi1)*math.cos(azi1)
		G[3*i+2,4] = g313*math.cos(azi1)+g323*math.sin(azi1)
		G[3*i+2,5] = g323*math.cos(azi1)-g313*math.sin(azi1)

	return (G)	

