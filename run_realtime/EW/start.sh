#!/bin/bash

# MTH: 1.) Make sure you've sourced the right envs so that
#          EW_PARAMS is set correctly.
#      2.) startstop automatically chdir to ${EW_PARAMS} and looks for startstop_unix.d
#nohup startstop params/startstop_unix.d > nohup.log &
nohup startstop > nohup.log &
