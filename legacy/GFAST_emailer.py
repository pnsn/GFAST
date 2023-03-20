#!/usr/bin/python
# Import smtplib for the actual sending function
import smtplib

# Import the email modules we'll need
from email.mime.text import MIMEText

# Open a plain text file for reading.  For this example, assume that
# the text file contains only ASCII characters.
def emailer(eventid):
	fname = 'events/GFAST_PGD_' + str(int(eventid)) + '.txt' 
	fp = open(fname, 'rb')

	msg = MIMEText(fp.read())
	fp.close()
	me = 'gfast.results@gmail.com'
	recipients = "crowellbw@gmail.com"
	msg['Subject'] = 'G-FAST Event Information for Event' + ' ' + str(eventid) 
	msg['From'] = me
	msg['To'] = recipients

	s = smtplib.SMTP('localhost')
	s.sendmail(me, [recipients], msg.as_string())
	s.quit()
