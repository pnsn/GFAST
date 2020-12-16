import os
import xml.etree.ElementTree as ET
import glob
from datetime import datetime, timezone

def main():
    '''
        Simple script to modify Ridgecrest ShakeAlert SA.xml with time = now
        and start tankplayer to begin releasing Ridgecrest packets onto the wave_ring
    '''

    # tankfile start - OT:
    offset_time = -9.8

    # Where is the event params/tankplayer.d file:
    ew_dir = '/opt/earthworm/run_working/params'

    # Where is the starting SA.xml file:
    xmldir_in  = '.'
    # Where should the modified SA.xml file be put:
    #   presumably where gfast_eew can find it
    xmldir_out = '/home/mth/mth/GFAST/run/events'
    xmldir_out = '.'

    files = ['SA.xml']
    for xmlfile in files:
        print(xmlfile)
        tree = ET.parse(xmlfile)
        root = tree.getroot()
        core = root.find('core_info')
        orig_time = core.find('orig_time')
        # Set otime to NOW
        #otime = str(UTCDateTime.utcnow())
        # This prints microseconds - hopefully won't matter:
        #otime = datetime.now(tz=timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%fZ")
        #print(otime)
        #
        # The tankplayer will stamp the first packets to NOW
        #   So we adjust the OT by offset_time wrt actual first packet time:
        timestamp = datetime.now(tz=timezone.utc).timestamp() - offset_time
        otime = datetime.utcfromtimestamp(timestamp).strftime("%Y-%m-%dT%H:%M:%S.%fZ")
        print("Stamp origin time:%s   [%f]" % (otime, timestamp))
        orig_time.text = orig_time.text.replace(orig_time.text, otime)
        target = os.path.join(xmldir_out, os.path.basename(xmlfile))
        tree.write(target)

        #os.chdir(ew_dir)
        #os.system('tankplayer tankplayer.d')

    return

if __name__ == "__main__":
    main()
