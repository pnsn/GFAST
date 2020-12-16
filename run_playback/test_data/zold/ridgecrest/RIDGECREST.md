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

1. set earthworm env vars

    For example, here is where I am running the ridgecrest example:

        >cd /opt/earthworm/run_working
        >ls
         data/  log/  params/
        >ls params
         earthworm_commonvars.d	earthworm_global.d  playback/	      statmgr.d     tankplayer.d
         earthworm.d		EPIC/		    startstop_unix.d  statmgr.desc
        >ls params/playback/
         ridgecrest_gps_60.tnk  ridgecrest_gps.tnk  ridgecrest_gps.tnk.sniff

    If you built the earthworm libs (above), then you
    likely already have the EW envs set.
    Here's what my EW env vars look like:

        [/opt/earthworm/run_working]> env|grep EW
        EW_PARAMS=/opt/earthworm/run_working/params/
        EW_BITS=64
        EW_DATA_DIR=/opt/earthworm/run_working/data/
        EW_VERSION=earthworm_7.10
        EW_LOG=/opt/earthworm/run_working/log/
        EW_HOME=/opt/earthworm
        EW_INSTALLATION=INST_UNKNOWN

    The important ones are EW_PARAMS, EW_DATA_DIR, EW_LOG, which
    must point to where the example is being run.
    If you're starting from scratch, do something like:

        >source /opt/earthworm/earthworm_svn/environment/ew_linux.bash

    and then modify whichever ones are not set correctly, eg.,

        >export RIDGECREST=/some/other/path
        >export EW_PARAMS=${RIDGECREST}/params
        >export EW_LOG=${RIDGECREST}/log
        >export EW_DATA_DIR=${RIDGECREST}/data      // Not sure we're even using this now


1. start earthworm

        >startstop startstop_unix.d

1. clear old SA event

        >rm GFAST/events/*

1. start GFAST_eew

        >bin/gfast_eew

1. In a separate terminal, inject SA event 

        >python start_it.py 

