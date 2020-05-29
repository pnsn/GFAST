import os
import xml.etree.ElementTree as ET
import glob
from datetime import datetime, timezone

def main():
    '''
        Simple script to modify Ridgecrest ShakeAlert SA.xml with time = now
        and start tankplayer to begin releasing Ridgecrest packets onto the wave_ring
    '''

    # Where is the ridgecrest params/tankplayer.d file:
    ew_dir = '/opt/earthworm/ridgecrest/params'
    # Where is the starting SA.xml file:
    xmldir_in  = '.'
    # Where should the modified SA.xml file be put:
    #   presumably where gfast_eew can find it
    xmldir_out = './events'

    files = glob.glob(xmldir_in + '/' + '*.xml')
    for xmlfile in files:
        print(xmlfile)
        tree = ET.parse(xmlfile)
        root = tree.getroot()
        core = root.find('core_info')
        orig_time = core.find('orig_time')
        # Set otime to NOW
        #otime = str(UTCDateTime.utcnow())
        # This prints microseconds - hopefully won't matter:
        otime = datetime.now(tz=timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%fZ")
        orig_time.text = orig_time.text.replace(orig_time.text, otime)

        target = os.path.join(xmldir_out, os.path.basename(xmlfile))
        tree.write(target)

        os.chdir(ew_dir)
        os.system('tankplayer tankplayer.d')

    return

if __name__ == "__main__":
    main()
