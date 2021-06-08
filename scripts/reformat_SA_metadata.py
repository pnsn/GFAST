#!/usr/bin/python
#temporary program to convert ShakeAlert geodetic metadata to
#format GFAST can understand
#Victor Kress, PNSN 12/2020


#SAmetaDir='/app/share/etc/geodetic/'
SAmetaDir='/home/kress/geodeticMetadata/'
SAcoordFile=SAmetaDir+'station_coords.dat'
SAnetList=['bk','ci','nc','pb','pw']
SAchanFile=SAmetaDir+'chanfile_%s.dat'

GFASTmetaDir='/home/kress/temp/'
GFASTmetaFile=GFASTmetaDir+'GFASTmeta_%s.dat'

coord={'000':7*['-']}

#read coordinate file
# Site Lat(deg) Long(deg) EllipElev(m) X(m) Y(m) Z(m) Epoch(yr)
print 'reading coordinates from %s'%SAcoordFile
with open(SAcoordFile, "r") as fh:
    for line in fh.readlines():
        if not line.isspace() and not line[0]=='#':
            toks=line.split()
            coord[toks[0]]=toks[1:]
    fh.close()

#read chanfile:
#net sta loc chan rate(1/sec) multiplier units PPP|(DIF sta refsta)
#and output to GFASTmetaFile which has lines like:
#PB BEPK  00 LXN 35.8784 -118.0741 2471.1 1.00 1.000000e+06 PPP nan nan
for net in SAnetList:
    print SAchanFile%net + ' -> ' + GFASTmetaFile%net
    outf=open(GFASTmetaFile%net,'w')
    with open(SAchanFile%net, "r") as inf:
        for line in inf.readlines():
            if not line.isspace() and not line[0]=='#':
                toks=line.split()
                outstr=' '.join(toks[:4])+' '
                if toks[1] in coord:
                    outstr += ' '.join(coord[toks[1]][0:3])+' '
                else:
                    #outstr += 'nan nan nan '
                    print '%s has no coordinate metadata for %s. Skipping.'%(SAcoordFile,toks[1])
                    continue
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
