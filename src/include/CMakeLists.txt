set(headers
gfast_activeMQ.h
gfast_core.h
gfast_eewUtils.h
gfast_enum.h
gfast_events.h
gfast.h
gfast_hdf5.h
gfast_struct.h
gfast_traceBuffer.h
gfast_xml.h)
file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
install(FILES ${headers} DESTINATION ${_relPath} COMPONENT headers)
