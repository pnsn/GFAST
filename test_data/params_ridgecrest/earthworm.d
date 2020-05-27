#  1999/07/28
#  The working copy of earthworm.d should live in your EW_PARAMS directory.
#
#  An example copy of earthworm.d resides in the vX.XX/environment
#  directory of this Earthworm distribution.

#                       earthworm.d

#              Earthworm administrative setup:
#              Installation-specific info on
#                    shared memory rings
#                    module ids
#                    message types

#   Please read all comments before making changes to this file.
#   The character string <-> numerical value mapping for certain
#   module id's and message types are sacred to earthworm.d
#   and must not be changed!!!

#--------------------------------------------------------------------------
#                      Shared Memory Ring Keys
#
# Define unique keys for shared memory regions (transport rings).
# All string/value mappings for shared memory keys may be locally altered.
#
# The maximum length of ring string is 32 characters.
#--------------------------------------------------------------------------
     
 Ring   WAVE_RING        1800    # near-real-time waveform data
 Ring   TEST_WAVE_RING   1809    # test waveform data
 Ring   RAW_RING         1802    # public data
 Ring   NQWAVE_RING      1803    # NetQuakes waveform data (late-arriving in clumps)
 Ring   PICK_RING        1805    # public parametric data (Decatur)
 Ring   HYPO_RING        1815    # public hypocenters etc. (all zones)
 Ring   BINDER_RING      1820    # private buffer for binder_ew (Decatur)
 Ring   TEST_BINDER_RING 1821    # private buffer for binder_ew (test)
 Ring   EQALARM_EW_RING  1825    # private buffer for eqalam_ew
 Ring	  STATUS_RING    1863    # buffer used for notifications
#
 Ring   PICK_KAN_RING    1183    # public picks for Kansas
 Ring   BINDER_KAN_RING  1188    # private buffer for binder_ew (Kansas)_
#
 Ring   PICK_AZL_RING    1282    # public picks for Azle TX
 Ring   BINDER_AZL_RING  1285    # private buffer for binder_ew (Azle TX)_
 Ring   MAG_RING         1286
#
# Do not put FLAG_RING in starstop*.d
 Ring   FLAG_RING        2080    # a private ring for Startstop 7.5 and later to use
                                 # for flags. If this doesn't exist, startstop will 
                                 # create the ring automatically at key 9999
                                 # If you run multiple startstops, you'll need to change
                                 # all ring keys values here in earthworm.d, including
                                 # this one. Do NOT include this private ring in
                                 # the ring area in startstop*d
  Ring NAMED_EVENT_RING  2089


#--------------------------------------------------------------------------
#                           Module IDs
#
#  Define all module name/module id pairs for this installation
#  Except for MOD_WILDCARD, all string/value mappings for module ids
#  may be locally altered. The character strings themselves may also
#  be changed to be more meaningful for your installation.
#
#  0-255 are the only valid module ids.
#
# The maximum length of the module string is 32 characters.
# 
# This list is in alphabetical order but doesn't need to be. Go ahead and
# add new modules and module IDs at the end.
#--------------------------------------------------------------------------

 Module   MOD_WILDCARD          0   # Sacred wildcard value - DO NOT CHANGE!!!
 Module   MOD_STARTSTOP         1
 Module   MOD_STATMGR           2
 Module   MOD_STATUS            3
 Module   MOD_BINDER            11
 Module   MOD_BINDER_EW         12
 Module   MOD_CARLSTATRIG       13
 Module   MOD_CARLSUBTRIG       14
 Module   MOD_TANKPLAYER       15 
 Module   MOD_SNIFF_TRACE_NQ    16 
 Module   MOD_EXP_SCNL2GLDN     20
 Module   MOD_WAVESERVERV       21 
 Module   MOD_DISKMGR           24
 Module   MOD_EQASSEMBLE        28
 Module   MOD_EW2FILE           36
 Module   MOD_EW2FILE_MAG       37
 Module   MOD_FILE2EW           50      
 Module   MOD_EWMAG2CISN        51
 Module   MOD_EWHTMLREPORT      52
 Module   MOD_EWHTMLEMAIL_DEC   60
 Module   MOD_LOCALMAG          74
 Module   MOD_PICK_EW           99
 Module   MOD_PKFILTER          101
 Module   MOD_REFTEK2EW         112
 Module   MOD_SLINK2EW          126
 Module   MOD_SLINK2EW_GS       127
 Module   MOD_SLINK2EW_OK       135
 Module   MOD_RUN_IMP_WWS       146
 Module   MOD_RUN_WWS           147
 Module   MOD_EXPORT_WWS        148
 Module   MOD_RTP2EW            149
 Module   MOD_NQ2WWS            151
 Module   MOD_NQ2WWS2           152
 Module   MOD_NQ2RING           153
 Module   MOD_BINDER_TEST       154
 Module   MOD_HYP_PUB           155
 Module   MOD_TRIG_PUB          156
 Module   MOD_FILE2EW_TRG       157
 Module   MOD_IMP_NQ2           158
 Module   MOD_PICK_KAN          159
 Module   MOD_PICK_NQ_KAN       160
 Module   MOD_BINDER_KAN        161
 Module   MOD_EQASSEMBLE_KAN    162
 Module   MOD_PICK_EW_KAN       163
 Module   MOD_EWHTMLEMAIL_KAN   164
#
 Module   MOD_PICK_AZL          170
 Module   MOD_PICK_NQ_AZL       171
 Module   MOD_BINDER_AZL        172
 Module   MOD_EQASSEMBLE_AZL    173
#
 Module   MOD_FILE2EW_MW        201
 Module   MOD_SPECMAG           203
 Module   MOD_EW2FILE_SPECMAG   205

#--------------------------------------------------------------------------
#                          Message Types
#
#     !!!  DO NOT USE message types 0 thru 99 in earthworm.d !!!
#
#  Define all message name/message-type pairs for this installation.
#
#  VALID numbers are:
#
# 100-255 Message types 100-255 are defined in each installation's  
#         earthworm.d file, under the control of each Earthworm 
#         installation. These values should be used to label messages
#         which remain internal to an Earthworm system or installation.
#         The character strings themselves should not be changed because 
#         the strings are often hard-coded into the modules.
#         However, the string/value mappings can be locally altered.
#         Any message types for locally-produced code may be defined here.
#              
#
#  OFF-LIMITS numbers:
#
#   0- 99 Message types 0-99 are defined in the file earthworm_global.d.
#         These numbers are reserved by Earthworm Central to label types 
#         of messages which may be exchanged between installations. These 
#         string/value mappings must be global to all Earthworm systems 
#         in order for exchanged messages to be properly interpreted.
#         
# The maximum length of the type string is 32 characters.
#
#--------------------------------------------------------------------------

# Installation-specific message-type mappings (100-255):
 Message  TYPE_SPECTRA       100
 Message  TYPE_QUAKE2K       101
 Message  TYPE_LINK          102
 Message  TYPE_EVENT2K       103
 Message  TYPE_PAGE          104
 Message  TYPE_KILL          105
 Message  TYPE_DSTDRINK      106
 Message  TYPE_RESTART       107
 Message  TYPE_REQSTATUS     108
 Message  TYPE_STATUS        109
 Message  TYPE_EQDELETE      110
 Message  TYPE_EVENT_SCNL    111
 Message  TYPE_RECONFIG      112
 Message  TYPE_STOP          113  # stop a child. same as kill, except statmgr
                          # should not restart it
 Message  TYPE_CANCELEVENT   114  # used by eqassemble
 Message  TYPE_CODA_AAV      115
 Message  TYPE_ACTIVATE_MODULE 117	 #  used by activated scripts
 Message  TYPE_ACTIVATE_COMPLETE 118	# used by activated scripts
 Message  TYPE_REFTEK      119


#   !!!  DO NOT USE message types 0 thru 99 in earthworm.d !!!
