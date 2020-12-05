#!/usr/bin/python3
#temporary program to convert ShakeAlert geodetic metadata to
#format GFAST can understand
#Victor Kress, PNSN 12/2020


SAcoordDir="/home/kress/git/WCEEW-geodetic/"
SAcoordFile="station_coords.dat"
SAchanFiles=["chanfile_pw.dat"]

GFASTmetaFile="/home/kress/temp/GFASTmeta.dat"

coord={'000':7*['-']}

#read coordinate file
# Site Lat(deg) Long(deg) EllipElev(m) X(m) Y(m) Z(m) Epoch(yr)
with open(SAcoordDir+SAcoordFile, "r") as fh:
    for line in fh.readlines():
        if not line.isspace() and not line[0]=='#':
            toks=line.split()
            coord[toks[0]]=toks[1:]
    fh.close()

#read chanfile:
#net sta loc chan rate(1/sec) multiplier units PPP|(DIF sta refsta)
#and output to GFASTmetaFile which has lines like:
#PB BEPK  00 LXN 35.8784 -118.0741 2471.1 1.00 1.000000e+06 PPP nan nan
outf=open(GFASTmetaFile,'w')
for cf in SAchanFiles:
    with open(SAcoordDir+cf, "r") as inf:
        for line in inf.readlines():
            if not line.isspace() and not line[0]=='#':
                toks=line.split()
                outstr=' '.join(toks[:4])+' '
                if toks[1] in coord:
                    outstr += ' '.join(coord[toks[1]][0:3])+' '
                else:
                    outstr += 'nan nan nan '
                outstr += ' '.join(toks[4:8])+' '
                if toks[7] == 'DIF':
                    outstr += ' '.join(toks[8:])
                else:
                    outstr += ' nan nan'
                #print(outstr)
                outf.write(outstr+'\n')
    inf.close()
outf.close()
print("done")
