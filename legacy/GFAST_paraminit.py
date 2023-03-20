#!/usr/bin/python

class Properties:
	def __init__(self,propfilename):
		self.dict={}
		infile=open(propfilename,'r')
		for line in infile:
			if (line) and (line[0]!='#') and ('=' in line):
				(key,val)=line.split('=')
                		self.dict[key]=val.strip()
        	return

	def getAMQhost(self):
		if self.dict.has_key('AMQhost'):
			return self.dict['AMQhost']
        	return ''

    	def getAMQport(self): 
        	if self.dict.has_key('AMQport'):
            		return int(self.dict['AMQport'])
        	return 0

    	def getAMQtopic(self): 
        	if self.dict.has_key('AMQtopic'):
            		return self.dict['AMQtopic']
        	return ''

    	def getAMQuser(self): 
       		if self.dict.has_key('AMQuser'):
            		return self.dict['AMQuser']

    	def getAMQpassword(self): 
        	if self.dict.has_key('AMQpassword'):
            		return self.dict['AMQpassword']

    	def getstreamfile(self): 
        	if self.dict.has_key('streamfile'):
            		return self.dict['streamfile']

    	def geteewsfile(self): 
        	if self.dict.has_key('eewsfile'):
            		return self.dict['eewsfile']
    	def geteewgfile(self): 
        	if self.dict.has_key('eewgfile'):
            		return self.dict['eewgfile']

    	def getsiteposfile(self): 
        	if self.dict.has_key('siteposfile'):
            		return self.dict['siteposfile']

    	def getbufflen(self): 
        	if self.dict.has_key('bufflen'):
            		return self.dict['bufflen']


    	def getstreamlength(self):
		if self.dict.has_key('streamfile'):
            		sf = self.dict['streamfile']
			with open(sf) as f:
				for i, l in enumerate(f):
      					pass
				streamlength = i+1
			return streamlength

	def getmode(self): 
        	if self.dict.has_key('synmode'):
            		return self.dict['synmode']

	def getsyndriver(self): 
        	if self.dict.has_key('syndriver'):
            		return self.dict['syndriver']

	def getsynoutput(self): 
        	if self.dict.has_key('synoutput'):
            		return self.dict['synoutput']

	def getRMQhost(self):
		if self.dict.has_key('RMQhost'):
			return self.dict['RMQhost']
        	return ''

    	def getRMQport(self): 
        	if self.dict.has_key('RMQport'):
            		return int(self.dict['RMQport'])
        	return 0

    	def getRMQtopic(self): 
        	if self.dict.has_key('RMQtopic'):
            		return self.dict['RMQtopic']
        	return ''

    	def getRMQuser(self): 
       		if self.dict.has_key('RMQuser'):
            		return self.dict['RMQuser']

    	def getRMQpassword(self): 
        	if self.dict.has_key('RMQpassword'):
            		return self.dict['RMQpassword']

	def getRMQexchange(self): 
        	if self.dict.has_key('RMQexchange'):
            		return self.dict['RMQexchange']



