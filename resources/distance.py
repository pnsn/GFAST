
from obspy.geodetics.base import gps2dist_azimuth

evla = -19.610
evlo = -70.769

stnfile = '/Users/mth/mth/python_pkgs/GFAST/test_data/iquique/Iquique2014_disp.chan.txt'
with open(stnfile, 'r') as fh:
    lines = fh.readlines()

for line in lines[1:]:
    #print(line)
    (net, stn, loc, chn, lat, lon, *rest) = line.split()
    print(net, stn, loc, chn, lat, lon)
    dist, azim, baz = gps2dist_azimuth(evla, evlo, float(lat), float(lon))
    print(net, stn, loc, chn, lat, lon, dist/1e3)

'''

 net,sta,loc,chan,lat,lon,elev,samplerate,gain,units
RK  AEDA    00  LXE -20.5462    -70.1777    0.0 1.00    1.000000e+06    counts/(m)    PPP   nan   nan
RK  AEDA    00  LXN -20.5462    -70.1777    0.0 1.00    1.000000e+06    counts/(m)    PPP   nan   nan
RK  AEDA    00  LXZ -20.5462    -70.1777    0.0 1.00    1.000000e+06    counts/(m)    PPP   nan   nan
'''
