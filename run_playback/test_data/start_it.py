import os
import argparse
from datetime import datetime, timezone
import glob
import sys
import yaml
import xml.etree.ElementTree as ET

GFAST_EVENTS_DIR = "../GFAST/events"

def main():
    '''
        Simple script to modify playback event ShakeAlert SA.xml with time = now
        and start tankplayer to begin releasing tankplayer packets onto the wave_ring
    '''

    #parser = argparse.ArgumentParser()
    #required = parser.add_argument_group("required arguments")
    #required.add_argument("--configFile", type=str, metavar='// e.g., --configFile=/path/to/config.yml',
                            #required=True)
    #args, unknown = parser.parse_known_args()

    usage = "start_it.py kaikoura (or some other event in known_events)"
    known_events = ['iquique', 'kaikoura', 'maule', 'nicoya', 'ridgecrest']
    if len(sys.argv) != 2 or sys.argv[1] not in known_events:
        print(usage)
        exit(2)

    event = sys.argv[1]
    configFile = os.path.join(event, 'config.yml')

    configuration = {}
    with open(configFile, 'r') as ymlfile:
        config = yaml.load(ymlfile, Loader=yaml.FullLoader)

    try:
        offset_time = float(config['offset_time'])
        ew_dir = config['ew_dir']
        tankfile = config['tankfile']
        #SAfile = config['SA_file']
        SAfile = os.path.join(event, os.path.basename(config['SA_file']))
        xmldir_out = config['xmldir_out']
    except:
        raise

    xmldir_out = GFAST_EVENTS_DIR

    files = [SAfile]
    for xmlfile in files:
        print(xmlfile)
        tree = ET.parse(xmlfile)
        root = tree.getroot()
        core = root.find('core_info')
        orig_time = core.find('orig_time')
        # The tankplayer will stamp the first packets to NOW
        #   So we adjust the OT by offset_time wrt actual first packet time:

        timestamp = datetime.now(tz=timezone.utc).timestamp()
        otime = datetime.utcfromtimestamp(timestamp).strftime("%Y-%m-%dT%H:%M:%S.%fZ")
        print("         Now time:%s   [%f]" % (otime, timestamp))
        timestamp = datetime.now(tz=timezone.utc).timestamp() + offset_time
        otime = datetime.utcfromtimestamp(timestamp).strftime("%Y-%m-%dT%H:%M:%S.%fZ")
        print("Stamp origin time:%s   [%f]" % (otime, timestamp))
        orig_time.text = orig_time.text.replace(orig_time.text, otime)

        os.chdir(ew_dir)

        thread = myThread(1, "Thread-1", 1, tankfile)
        thread.start()

        time.sleep(30)
        target = os.path.join(xmldir_out, os.path.basename(xmlfile))
        tree.write(target)

    return

import threading
import time

exitFlag = 0

class myThread (threading.Thread):
   def __init__(self, threadID, name, counter, tankfile):
      threading.Thread.__init__(self)
      self.threadID = threadID
      self.name = name
      self.counter = counter
      self.tankfile = tankfile

   def run(self):
      print("*** Starting Python thread:" + self.name)
      os.system('tankplayer %s' % self.tankfile)


if __name__ == "__main__":
    main()
