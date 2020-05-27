#
#       Startstop (Unix Version -- Linux) Configuration File
#
#    <nRing> is the number of transport rings to create.
#    <Ring> specifies the name of a ring followed by it's size
#    in kilobytes, eg        Ring    WAVE_RING 1024
#    The maximum size of a ring depends on your operating system, 
#    amount of physical RAM and configured virtual (paging) memory 
#    available. A good place to start is 1024 kilobytes. 
#    Ring names are listed in file earthworm.d.
#
 nRing  2
 Ring   STATUS_RING    192
 Ring   WAVE_RING      10000
#
 MyModuleId    MOD_STARTSTOP  # Module Id for this program
 HeartbeatInt  50             # Heartbeat interval in seconds
 MyClassName   NONE           # For this program
 MyPriority     0             # For this program
 LogFile        1             # 1=write a log file to disk, 0=don't
 KillDelay      5             # seconds to wait before killing modules on
                              #  shutdown
 HardKillDelay  5             # number of seconds to wait on hard shutdown
                              #  for a child to respond to KILL signal
                              #  If missing or 0, no KILL signal will be sent
# maxStatusLineLen   80       # Uncomment to specify length of lines in status
# statmgrDelay  	  2       # Uncomment to specify the number of seconds
                              # to wait after starting statmgr 
                              # default is 1 second

# Process          "tankplayer tankplayer.d"  
# Class/Priority    NONE 0

 Process          "copystatus WAVE_RING STATUS_RING"
 Class/Priority    NONE 0

#Process          "wave_serverV wsv/wave_serverV.d"
#Class/Priority	NONE 0
 
