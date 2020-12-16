
import sys

#
'''
PW AB18  00 LYE 66.858361 -162.613508 37.82 1 1e6 DU/M PPP
PW AB18  00 LYN 66.858361 -162.613508 37.82 1 1e6 DU/M PPP
PW AB18  00 LYZ 66.858361 -162.613508 37.82 1 1e6 DU/M PPP
PW AB18  00 LYQ 66.858361 -162.613508 37.82 1 1 - PPP
PW AB18  00 LY1 66.858361 -162.613508 37.82 1 1e6 DU/M PPP

PB BEPK  00 LXN 35.8784 -118.0741 2471.1 1.00 1.000000e+06 counts/(m)   PPP nan nan
'''

with open(sys.argv[1], "r") as fh:
    for line in fh.readlines():
        if line[0] == "#":
            continue
        net, stn, loc, chan, *rest = line.split()
        if chan[2] not in {'Z', 'E', 'N'}:
            continue

        #print(net, stn, loc, chan)
        #line.replace('\t', '    ')
        out = line.replace('PPP', 'PPP  nan  nan').replace('\t','    ').rstrip()
        print(out)
