include ../../Makefile.inc

ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif

OBJ_SML = $(OBJ)/shakeAlert_coreInfo.o \
	  $(OBJ)/shakeAlert_slip.o \
	  $(OBJ)/shakeAlert_vertex.o \
	  $(OBJ)/shakeAlert_vertices.o

OBJ_QML = $(OBJ)/quakeML_depth.o \
	  $(OBJ)/quakeML_epoch2string.o \
	  $(OBJ)/quakeML_focalMechanism.o \
	  $(OBJ)/quakeML_latitude.o \
	  $(OBJ)/quakeML_longitude.o \
	  $(OBJ)/quakeML_momentTensor.o \
	  $(OBJ)/quakeML_nodalPlanes.o \
	  $(OBJ)/quakeML_principalAxes.o \
	  $(OBJ)/quakeML_tensor.o \
	  $(OBJ)/quakeML_time.o \
	  $(OBJ)/quakeML_units.o

OBJ_LIB = $(OBJ_SML) $(OBJ_QML)

lib: $(GFAST_STATIC_XML)

$(GFAST_STATIC_XML): $(OBJ_LIB)
	$(ARCH) $(ARCHFLAGS) $(GFAST_STATIC_XML) $(OBJ_LIB)
	$(RANLIB) $(GFAST_STATIC_XML)

$(GFAST_SHARED_XML): $(OBJ_LIB)
	$(CC) $(MAKE_SHARED) -o $(GFAST_STATIC_XML) $(OBJ_LIB)

################################# ShakeAlertML #################################

$(OBJ)/shakeAlert_coreInfo.o: shakeAlert/coreInfo.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c shakeAlert/coreInfo.c -o $(OBJ)/shakeAlert_coreInfo.o

$(OBJ)/shakeAlert_segment.o: shakeAlert/segment.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c shakeAlert/segment.c -o $(OBJ)/shakeAlert_segment.o

$(OBJ)/shakeAlert_slip.o: shakeAlert/slip.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c shakeAlert/slip.c -o $(OBJ)/shakeAlert_slip.o

$(OBJ)/shakeAlert_vertices.o: shakeAlert/vertices.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c shakeAlert/vertices.c -o $(OBJ)/shakeAlert_vertices.o

$(OBJ)/shakeAlert_vertex.o: shakeAlert/vertex.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c shakeAlert/vertex.c -o $(OBJ)/shakeAlert_vertex.o

################################## QuakeML #####################################

$(OBJ)/quakeML_depth.o: quakeML/depth.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/depth.c -o $(OBJ)/quakeML_depth.o

$(OBJ)/quakeML_epoch2string.o: quakeML/epoch2string.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/epoch2string.c -o $(OBJ)/quakeML_epoch2string.o

$(OBJ)/quakeML_focalMechanism.o: quakeML/focalMechanism.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/focalMechanism.c -o $(OBJ)/quakeML_focalMechanism.o

$(OBJ)/quakeML_latitude.o: quakeML/latitude.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/latitude.c -o $(OBJ)/quakeML_latitude.o

$(OBJ)/quakeML_longitude.o: quakeML/longitude.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/longitude.c -o $(OBJ)/quakeML_longitude.o

$(OBJ)/quakeML_momentTensor.o: quakeML/momentTensor.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/momentTensor.c -o $(OBJ)/quakeML_momentTensor.o

$(OBJ)/quakeML_nodalPlanes.o: quakeML/nodalPlanes.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/nodalPlanes.c -o $(OBJ)/quakeML_nodalPlanes.o

$(OBJ)/quakeML_principalAxes.o: quakeML/principalAxes.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/principalAxes.c -o $(OBJ)/quakeML_principalAxes.o

$(OBJ)/quakeML_tensor.o: quakeML/tensor.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/tensor.c -o $(OBJ)/quakeML_tensor.o

$(OBJ)/quakeML_time.o: quakeML/time.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/time.c -o $(OBJ)/quakeML_time.o

$(OBJ)/quakeML_units.o: quakeML/units.c
	$(CC) $(CFLAGS) $(INC_ALL) $(INC_XML2) $(CSHARED) -c quakeML/units.c -o $(OBJ)/quakeML_units.o

clean:
	$(RM) -rf $(OBJ)/*.o $(GFAST_STATIC_XML) $(GFAST_SHARED_XML)

