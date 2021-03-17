
#Realtime run

This is a skeleton directory that can be customized
to run GFAST in real-time mode.

Here are the steps:

 1. copy this directory to where you want to run gfast
 2. edit source.envs to reflect the earthworm paths on your machine
 3. source source.envs
 4. start earthworm
    - First modify EW/params/geojson2ew.d
    - Note that you must set user name/pw to connect to the rabbitmq server in order to import real-time GNSS packets onto the earthworm ring.
    - Start earthworm:
    ```
    >cd EW
    >./start.sh   // This will start the earthworm ring
    ```

 5. start gfast
    - edit GFAST/gfast.props if needed 
    - start gfast_eew:
    ```
    >gfast_eew
    ```
At this point gfast_eew needs a core info event message to trigger any
processing. Currently there are 2 ways to do this:

 1. Connect to an eelarms activeMQ broker

    This is configurable from gfast.props:

    ```
    [ActiveMQ]
    # ActiveMQ hostname to access ElarmS messages
    host=siren
    # ActiveMQ port to access ElarmS messages (61620)
    port=61620
    # ActiveMQ topic to access ElarmS messages (eew.alg.elarms.data)
    originTopic=eew.alg.elarms.data
    destinationTopic=eew.alg.elarms.data
    # ActiveMQ username to access ElarmS messages
    user=elarms
    # ActiveMQ password to access ElarmS messages
    password=xxxxxx
    ```

    In addition, you must build gfast_eew with the AMQ libraries

    in zbuild-gcc.sh, set:

    ```
    USE_AMQ=true
    ```

    and build it:
    ```
    >zbuild-gcc.sh
    >make
    ```

 2. Use NEIC's PDL (Product Data Layer)

    ProductClient.jar receives quakeml solutions from various sources (e.g., U.S. regional networks, NEIC, tsunami warning centers, etc.) and triggers pdl-to-SA which converts the quakeml to core-info (evid.xml) and drops it into the GFAST/events dir.

    There are 2 parts to this:
    - ProductClient.jar - download from USGS/NEIC

    https://github.com/usgs/pdl.git

    - aqms-to-pdl - download from:

    https://gitlab.isti.com/mhagerty/aqms-pdl -or-
    https://gitlab.com/aqms.swg/aqms-pdl

    (You probably need an invite/account for both)

    See installation instructions.
    ```
    >pip install .
    ```

    Find the absolute path to pdl-to-SA, e.g,:
    ```
    >which pdl-to-SA
    /home/mhagerty/miniconda3/envs/aqms/bin/pdl-to-SA
    ```

    You'll need this for the "command=" value in the ProductClient config.ini

## Configuring PDL

    You can edit the ProductClient config.ini file, e.g.,:

    command = /home/mhagerty/miniconda3/envs/aqms/bin/pdl-to-SA --configFile=/home/mhagerty/run_rt/pdl/config_SA.yml --loglevel=INFO --ignore-event-after=3600seconds --ignore-magnitude-below=4.4 

    You can view the flags available on pdl-to-SA:

    ```
    >pdl-to-SA
     usage: pdl-to-SA [-h] --status STATUS --code // e.g., --code=hv71386392 --directory // path to quakeml.xml --type //
                 e.g., --type=phase_data --source // e.g., --source=pr [--optional_arg OPTIONAL_ARG]
                 ...
                 [--ignore-latitude-above // Ignore latitude > than this]
                 [--ignore-latitude-below // Ignore latitude < than this]
                 [--ignore-longitude-above // Ignore longitude > than this]
                 [--ignore-longitude-below // Ignore longitude < than this]
                 [--ignore-depth-above // Ignore depth > than this] [--ignore-depth-below // Ignore depth < than this]
                 [--ignore-magnitude-below // Ignore magnitude < than this]
                 [--ignore-magnitude-above Ignore magnitude > than this]
                 [--ignore-sources // Ignore these sources, eg. =HV,US]
                 [--only-listen-to-sources // Only listen for these sources] [--ignore-arrivals] [--ignore-amplitudes]
                 [--debug DEBUG] [--loglevel // --loglevel=DEBUG or INFO,WARN,ERROR,etc]
                 [--configFile // --configFile=config.yml]
                 [--ignore-event-after // Ignore if event age > {10mins, 3hrs, 5days, 2weeks, 3months, etc}]
     pdl-to-SA: error: the following arguments are required: --status, --code, --directory, --type, --source
    ```

  You can also edit the pdl-to-SA config file.

    >cat config_SA.yml
     LOG_DIR: ./log.SA
     LOG_LEVEL: 'INFO'
     pdl-types: ['origin', 'phase-data', 'moment-tensor', 'focal-mechanism','internal-moment-tensor',
                 'internal-origin']
     EVENT_DIR: ../GFAST/events

  - pdl-type - limit which PDL message types you want to react to.
Currently [Dec. 2020], the early Mww magnitude messages are wrapped as
internal-moment-tensor and are only available on the developement server
(pdldevel.cr.usgs.gov).

  - EVENT_DIR - Where the output core-info (evid.xml) msg should be
    dropped (e.g., where gfast_eew will find it)

