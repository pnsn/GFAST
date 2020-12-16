
#Test event playback

This directory contains several test events that can be run with GFAST.

    >ls test_data:
     iquique/ 
     kaikoura/
     maule/
     nicoya/
     ridgecrest/

Because gfast_eew runs in real-time mode, only looking
at data + events within the recent past (e.g., last 0-10 mins),
the test data set times have to be spoofed to current
time.

This is done by the python script test_data/start_it.py
which, when executed, plays back the tankplayer packets
with first timestamp set to **now**, then drops a starting
event solution into the proper directory to trigger
GFAST processing.  The event origin time is
set to the correct time to match its timing relative
to the first tankplayer timestamp.

The sequence to run the tests is:
 1. copy this directory to where you want to run the test
 2. edit source.envs to reflect the earthworm paths on your machine
 3. source source.envs
 4. start earthworm
    ```
    >cd EW
    >./start.sh   // This will start the earthworm ring
    ```
 5. start gfast
    - edit GFAST/gfast.props - select the playback metadata file that
      matches the SCNLs that tankplayer will put on the WAVE_RING, e.g.,:
    - set metaDataFile=../test_data/ridgecrest/ridgecrest-stns.txt
    -  start gfast_eew:
    ```
    >gfast_eew
    ```
 6. start the event playback
    ```
    >cd test_data
    >start_it.py ridgecrest  (or: maule, iquique, etc)
    ```
    This will start the event packets flowing onto the earthworm ring
      with first timestamp = now, and will drop the SA.xml to trigger
      gfast_eew processing in the GFAST/events dir.

