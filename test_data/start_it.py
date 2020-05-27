import os
from obspy.core.utcdatetime import UTCDateTime
import xml.etree.ElementTree as ET
import glob

'''
<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<event_message alg_vers="3.1.4-2018-11-08" category="live" instance="epic@eew-bk-prod1" message_type="update" orig_sys="epic" ref_id="0" ref_src="" timestamp="2019-07-06T03:20:28.539Z" version="25">
  <core_info id="7277">
    <mag units="Mw">6.2582</mag>
    <mag_uncer units="Mw">0.2317</mag_uncer>
    <lat units="deg">35.7725</lat>
    <lat_uncer units="deg">0.0465</lat_uncer>
    <lon units="deg">-117.6097</lon>
    <lon_uncer units="deg">0.0465</lon_uncer>
    <depth units="km">8.0000</depth>
    <depth_uncer units="km">5.0000</depth_uncer>
    <orig_time units="UTC">2019-07-06T03:19:53.221Z</orig_time>
  </core_info>
</event_message>
'''
from datetime import datetime
def main():
    ew_dir = '/opt/earthworm/ridgecrest/params'
    xmldir_in  = '.'
    xmldir_out = '/home/mth/mth/gfast/GFAST/events'

    files = glob.glob(xmldir_in + '/' + '*.xml')
    for xmlfile in files:
        print(xmlfile)
        tree = ET.parse(xmlfile)
        root = tree.getroot()
        core = root.find('core_info')
        orig_time = core.find('orig_time')
        # Set otime to NOW
        otime = str(UTCDateTime.utcnow())
        orig_time.text = orig_time.text.replace(orig_time.text, otime)
        #orig_time.text = '0'
        print(orig_time.text)
        target = os.path.join(xmldir_out, os.path.basename(xmlfile))
        print(target)
        tree.write(target)
        print("Good to go!")
        print(UTCDateTime.utcnow())
        print(UTCDateTime.utcnow().timestamp)
        os.chdir(ew_dir)
        os.system('tankplayer tankplayer.d')

    return

if __name__ == "__main__":
    main()
