#!/usr/bin/python
#classes to read events off of EEW activemq topics and parse xml
# Victor Kress, 7/7/2014

import sys
import datetime
import xml.etree.ElementTree as ET
import stomp
from GFAST_paraminit import Properties

timfmt='%Y-%m-%dT%H:%M:%S.%fZ'

class alertIteration:
    '''class to hold information from EEW module alert'''
    def __init__(self,xmlString=''):
        self.algorithm=''
        self.version=-1
        self.mag=0.0
        self.mag_units=''
        self.mag_uncer=0.0
        self.mag_uncer_units=''
        self.lat=0.0
        self.lat_units='deg'
        self.lat_uncer=0.0
        self.lat_uncer_units='deg'
        self.lon=0.0
        self.lon_units='deg'
        self.lon_uncer=0.0
        self.lon_uncer_units='deg'
        self.depth=0
        self.depth_units='km'
        self.depth_uncer=0
        self.depth_uncer_units='km'
        self.otime=None
        self.otime_uncer=0
        self.likelihood=0
        self.obs=[]
        if xmlString: self.parseXML(xmlString)
        return
    def parseXML(self,xmlString):
	print xmlString
        '''parse input xml string into class variables'''
        em=ET.fromstring(xmlString)
        self.attrib=em.attrib
        #core_info block
        ci=em.find('core_info')
        self.id=int(ci.attrib['id'])
        m=ci.find('mag')
        self.mag_units=m.attrib['units']
        self.mag=float(m.text)
        mu=ci.find('mag_uncer')
        self.mag_uncer=float(mu.text)
        l=ci.find('lat')
        self.lat=float(l.text)
        l=ci.find('lat_uncer')
        self.lat_uncer=float(l.text)
        l=ci.find('lon')
        self.lon=float(l.text)
        l=ci.find('lon_uncer')
        self.lon_uncer=float(l.text)
        d=ci.find('depth')
        self.depth_units=d.attrib['units']
        self.depth=float(d.text)
        du=ci.find('depth_uncer')
        self.depth_uncer=float(du.text)
        ot=ci.find('orig_time')
        self.otime=datetime.datetime.strptime(ot.text,timfmt)
        ot=ci.find('orig_time_uncer')
        self.otime_uncer=float(ot.text)
        self.likelihood=float(ci.find('likelihood').text)
        return

    def __str__(self):
        '''formatted printout of class variables'''
        s=['mag = %f +- %f %s'%(self.mag,self.mag_uncer,self.mag_units),
           'lat = %f +- %f'%(self.lat,self.lat_uncer),
           'lon = %f +- %f'%(self.lon,self.lon_uncer),
           'depth = %f +- %f %s'%(self.depth,self.depth_uncer,self.depth_units),
           'origin time = %s +- %f sec'%(self.otime.strftime(timfmt),
                                         self.otime_uncer),
           'likelihood = %f'%self.likelihood]
        for o in self.obs:
            s.append('%s %f %s lat=%f lon=%f %s'%(o['sncl'],o['value']
                                                  ,o['units'],
                                                  o['lat'],o['lon'],
                                                  o['time'].strftime(timfmt)))
        return '\n'.join(s)

class Listener(stomp.ConnectionListener):
    def __init__(self,callback):
        self.callback=callback
        return
    def on_error(self, headers, message):
        print('Listener received an error %s' % message)
        return
    def on_message(self, headers, message):
        msg=alertIteration(message)
        self.callback(msg)
        return
    
class AMQAlertClient():
    def __init__(self,props):
        self.alertHost=props.getAMQhost()
        self.alertPort=props.getAMQport()
        self.alertTopic='/topic/%s'%props.getAMQtopic()
        self.amquser=props.getAMQuser()
        self.amqpwd=props.getAMQpassword()
        self.conn=None
        return
    def connect(self,callbackFunc):
        target=[(self.alertHost,self.alertPort)]
        self.conn = stomp.Connection(target)
        self.conn.set_listener('amq1',Listener(callbackFunc))
        self.conn.start()
        self.conn.connect(self.amquser,self.amqpwd)
        self.conn.subscribe(destination=self.alertTopic, id=9999, ack='auto')
        return
    def disconnect(self):
        self.conn.unsubscribe(id=9999)
        self.conn.disconnect()
        self.conn.stop()
        return
#end of AMQAlertClient def

def printer(msg):
    #print msg
    a = msg
    lat = "{0:.4f}".format(float(a.lat))
    lon = "{0:.4f}".format(float(a.lon))
    mag = "{0:.2f}".format(float(a.mag))
    eewfile='ElarmS_messages.txt'
    f = open(eewfile,'a')
    f.write(str(a.id) + ' ' + str(a.otime) + ' ' + str(mag) + ' ' + str(lon) +  ' ' + str(lat))
    f.write("\n")
    f.close()
    #print a.otime.year, a.otime.month, a.otime.day, a.otime.hour, a.otime.minute, a.otime.second, a.otime.microsecond
    print a.mag, a.otime, a.lon, a.lat
    return

    
