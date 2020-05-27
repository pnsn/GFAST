# Ridgecrest Example Instructions 

  The Ridgecrest data are in test_data/params_ridgecrest/playback/ridgecrest_gps_60.tnk.
  The data are cut so that the origin time is at 60 seconds into the
file.

  When the injection script is run (step 3 below), it will create a
shake alert event with the Ridgecrest hypocenter and origin time = now
(= the time the script was run) and drop it into GFAST/events/SA.xml.
It will also start the earthworm tankplayer file.  The first packet 
injected onto the ring will have a timestamp = now - 60 seconds.

  Thus, to GFAST, it will seem that we just received an event with
otime=now, but we must wait 60 seconds until the relevant data starts to
trickle in for the stations of interest.
In this way it mimics a real-time event, where the data is not all
available right away.

  Each time GFAST updates (once every second), it sees the latest (one
additional second of data) data that has arrived and processes it.  In
this way we see a solution evolving over time as more data arrives.


1. copy the playback file to the correct location

    >cp ridgecrest_gps_60.tnk /opt/earthworm/ridgecrest/params/playback

1. start earthworm

    >cd /opt/earthworm/ridgecrest/params
    >source ew env
    >startstop startstop_unix.d

2. clear old SA event

    >rm GFAST/events/*

2. start GFAST_eew

    >bin/gfast_eew

3. In a separate terminal, inject SA event 

    >python start_it.py   // Note python has ObsPy and xml etree dependencies

