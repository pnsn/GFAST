include ../../Makefile.inc

OBJ_CMOPAD_DIR = cmopad/$(OBJ)
OBJ_CMT_DIR = cmt/$(OBJ)
OBJ_COORDTOOLS_DIR = coordtools/$(OBJ)
OBJ_FF_DIR = ff/$(OBJ)
OBJ_SCALING_DIR = scaling/$(OBJ)
OBJ_WP_DIR = waveformProcessor/$(OBJ)

ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif

OBJ_CMOPAD = $(OBJ)/cmopad.o

OBJ_CMT = $(OBJ)/cmt_decomposeMomentTensor.o \
	  $(OBJ)/cmt_depthGridSearch.o \
	  $(OBJ)/cmt_initialize.o \
	  $(OBJ)/cmt_setDiagonalWeightMatrix.o \
	  $(OBJ)/cmt_setForwardModel.o \
	  $(OBJ)/cmt_setRHS.o \
	  $(OBJ)/cmt_weightForwardModel.o \
	  $(OBJ)/cmt_weightObservations.o

OBJ_COORDTOOLS = $(OBJ)/coordtools_ll2utm.o \
	         $(OBJ)/coordtools_utm2ll.o

OBJ_FF = $(OBJ)/ff_initialize.o \
	 $(OBJ)/ff_faultPlaneGridSearch.o \
	 $(OBJ)/ff_meshFaultPlane.o \
	 $(OBJ)/ff_setDiagonalWeightMatrix.o \
	 $(OBJ)/ff_setForwardModel.o \
	 $(OBJ)/ff_setRegularizer.o \
	 $(OBJ)/ff_setRHS.o \
	 $(OBJ)/ff_weightForwardModel.o \
	 $(OBJ)/ff_weightObservations.o

OBJ_SCALING_PGD = $(OBJ)/pgd_depthGridSearch.o \
		  $(OBJ)/pgd_finalize.o \
		  $(OBJ)/pgd_initialize.o \
		  $(OBJ)/pgd_setForwardModel.o \
		  $(OBJ)/pgd_setRHS.o \
		  $(OBJ)/pgd_setDiagonalWeightMatrix.o \
		  $(OBJ)/pgd_weightForwardModel.o \
		  $(OBJ)/pgd_weightObservations.o

OBJ_WP = $(OBJ)/waveformProcessor_offset.o \
	 $(OBJ)/waveformProcessor_peakDisplacement.o

OBJ_LIB = $(OBJ_CMOPAD) $(OBJ_CMT) $(OBJ_COORDTOOLS) $(OBJ_FF) $(OBJ_SCALING_PGD) $(OBJ_WP)

lib: $(GFAST_STATIC_CORE)

$(GFAST_STATIC_CORE): $(OBJ_LIB)
	$(ARCH) $(ARCHFLAGS) $(GFAST_STATIC_CORE) $(OBJ_LIB)
	$(RANLIB) $(GFAST_STATIC_CORE)

$(GFAST_SHARED_CORE): $(OBJ_LIB)
	$(CC) $(MAKE_SHARED) -o $(GFAST_SHARED_CORE) $(OBJ_LIB)

####################################### cMoPaD ########################################

$(OBJ)/cmopad.o: cmopad/cmopad.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmopad/cmopad.c -o $(OBJ)/cmopad.o

######################################## CMT ##########################################

$(OBJ)/cmt_decomposeMomentTensor.o: cmt/decomposeMomentTensor.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmt/decomposeMomentTensor.c -o $(OBJ)/cmt_decomposeMomentTensor.o

$(OBJ)/cmt_depthGridSearch.o: cmt/depthGridSearch.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmt/depthGridSearch.c -o $(OBJ)/cmt_depthGridSearch.o

$(OBJ)/cmt_initialize.o: cmt/initialize.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmt/initialize.c -o $(OBJ)/cmt_initialize.o

$(OBJ)/cmt_setDiagonalWeightMatrix.o: cmt/setDiagonalWeightMatrix.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmt/setDiagonalWeightMatrix.c -o $(OBJ)/cmt_setDiagonalWeightMatrix.o

$(OBJ)/cmt_setForwardModel.o: cmt/setForwardModel.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmt/setForwardModel.c -o $(OBJ)/cmt_setForwardModel.o

$(OBJ)/cmt_setRHS.o: cmt/setRHS.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmt/setRHS.c -o $(OBJ)/cmt_setRHS.o

$(OBJ)/cmt_weightForwardModel.o: cmt/weightForwardModel.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmt/weightForwardModel.c -o $(OBJ)/cmt_weightForwardModel.o

$(OBJ)/cmt_weightObservations.o: cmt/weightObservations.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c cmt/weightObservations.c -o $(OBJ)/cmt_weightObservations.o

###################################### Coordtools #####################################

$(OBJ)/coordtools_ll2utm.o: coordtools/ll2utm.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c coordtools/ll2utm.c -o $(OBJ)/coordtools_ll2utm.o

$(OBJ)/coordtools_utm2ll.o: coordtools/utm2ll.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c coordtools/utm2ll.c -o $(OBJ)/coordtools_utm2ll.o

#################################### Finite Fault #####################################

$(OBJ)/ff_initialize.o: ff/initialize.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/initialize.c -o $(OBJ)/ff_initialize.o

$(OBJ)/ff_faultPlaneGridSearch.o: ff/faultPlaneGridSearch.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/faultPlaneGridSearch.c -o $(OBJ)/ff_faultPlaneGridSearch.o

$(OBJ)/ff_meshFaultPlane.o: ff/meshFaultPlane.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/meshFaultPlane.c -o $(OBJ)/ff_meshFaultPlane.o

$(OBJ)/ff_setDiagonalWeightMatrix.o: ff/setDiagonalWeightMatrix.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/setDiagonalWeightMatrix.c -o $(OBJ)/ff_setDiagonalWeightMatrix.o

$(OBJ)/ff_setForwardModel.o: ff/setForwardModel.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/setForwardModel.c -o $(OBJ)/ff_setForwardModel.o

$(OBJ)/ff_setRegularizer.o: ff/setRegularizer.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/setRegularizer.c -o $(OBJ)/ff_setRegularizer.o

$(OBJ)/ff_setRHS.o: ff/setRHS.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/setRHS.c -o $(OBJ)/ff_setRHS.o

$(OBJ)/ff_weightForwardModel.o: ff/weightForwardModel.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/weightForwardModel.c -o $(OBJ)/ff_weightForwardModel.o

$(OBJ)/ff_weightObservations.o: ff/weightObservations.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c ff/weightObservations.c -o $(OBJ)/ff_weightObservations.o

#################################### PGD Scaling ######################################

$(OBJ)/pgd_depthGridSearch.o: scaling/pgd_depthGridSearch.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c scaling/pgd_depthGridSearch.c -o $(OBJ)/pgd_depthGridSearch.o

$(OBJ)/pgd_finalize.o: scaling/pgd_finalize.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c scaling/pgd_finalize.c -o $(OBJ)/pgd_finalize.o

$(OBJ)/pgd_initialize.o: scaling/pgd_initialize.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c scaling/pgd_initialize.c -o $(OBJ)/pgd_initialize.o

$(OBJ)/pgd_setForwardModel.o: scaling/pgd_setForwardModel.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c scaling/pgd_setForwardModel.c -o $(OBJ)/pgd_setForwardModel.o

$(OBJ)/pgd_setDiagonalWeightMatrix.o: scaling/pgd_setDiagonalWeightMatrix.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c scaling/pgd_setDiagonalWeightMatrix.c -o $(OBJ)/pgd_setDiagonalWeightMatrix.o

$(OBJ)/pgd_setRHS.o: scaling/pgd_setRHS.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c scaling/pgd_setRHS.c -o $(OBJ)/pgd_setRHS.o

$(OBJ)/pgd_weightForwardModel.o: scaling/pgd_weightForwardModel.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c scaling/pgd_weightForwardModel.c -o $(OBJ)/pgd_weightForwardModel.o

$(OBJ)/pgd_weightObservations.o: scaling/pgd_weightObservations.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c scaling/pgd_weightObservations.c -o $(OBJ)/pgd_weightObservations.o

################################## Waveform processor #################################

$(OBJ)/waveformProcessor_peakDisplacement.o: waveformProcessor/peakDisplacement.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c waveformProcessor/peakDisplacement.c -o $(OBJ)/waveformProcessor_peakDisplacement.o

$(OBJ)/waveformProcessor_offset.o: waveformProcessor/offset.c
	$(CC) $(CFLAGS) $(INC_ALL) $(CSHARED) -c waveformProcessor/offset.c -o $(OBJ)/waveformProcessor_offset.o

###################################### clean ##########################################

clean:
	$(RM) -rf $(OBJ) $(GFAST_STATIC_CORE) $(GFAST_SHARED_CORE)

