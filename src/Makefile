include ../Makefile.inc

OBJ_BUFFER_DIR = buffer/$(OBJ)
OBJ_CMT_DIR = cmt/$(OBJ)
OBJ_EVENTS_DIR = events/$(OBJ)
OBJ_FF_DIR = ff/$(OBJ)
OBJ_SCALING_DIR = scaling/$(OBJ)
OBJ_COORDTOOLS_DIR = coordtools/$(OBJ)
ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif
ifeq "$(wildcard $(OBJ_BUFFER_DIR) )" ""
-include $(shell mkdir $(OBJ_BUFFER_DIR)) $(wildcard $(OBJ_BUFFER_DIR)/*)
endif
ifeq "$(wildcard $(OBJ_CMT_DIR) )" ""
-include $(shell mkdir $(OBJ_CMT_DIR)) $(wildcard $(OBJ_CMT_DIR)/*)
endif
ifeq "$(wildcard $(OBJ_COORDTOOLS_DIR) )" ""
-include $(shell mkdir $(OBJ_COORDTOOLS_DIR)) $(wildcard $(OBJ_COORDTOOLS_DIR)/*)
endif
ifeq "$(wildcard $(OBJ_EVENTS_DIR) )" ""
-include $(shell mkdir $(OBJ_EVENTS_DIR)) $(wildcard $(OBJ_EVENTS_DIR)/*)
endif
ifeq "$(wildcard $(OBJ_FF_DIR) )" ""
-include $(shell mkdir $(OBJ_FF_DIR)) $(wildcard $(OBJ_FF_DIR)/*)
endif
ifeq "$(wildcard $(OBJ_SCALING_DIR) )" ""
-include $(shell mkdir $(OBJ_SCALING_DIR)) $(wildcard $(OBJ_SCALING_DIR)/*)
endif

OBJ_UTILS = $(OBJ)/sacio.o
OBJ_SCALING_PGD = $(OBJ_SCALING_DIR)/pgd_depthGridSearch.o \
	      $(OBJ_SCALING_DIR)/pgd_init.o \
	      $(OBJ_SCALING_DIR)/pgd_setForwardModel.o \
	      $(OBJ_SCALING_DIR)/pgd_setRHS.o
OBJ_BUFFER = $(OBJ_BUFFER_DIR)/getNumberOfStreams.o \
	     $(OBJ_BUFFER_DIR)/setBufferSpace.o \
	     $(OBJ_BUFFER_DIR)/setInitialTime.o \
	     $(OBJ_BUFFER_DIR)/setSiteSamplingPeriod.o \
	     $(OBJ_BUFFER_DIR)/updateBuffer.o
OBJ_CMT = $(OBJ_CMT_DIR)/decompose.o \
	  $(OBJ_CMT_DIR)/depthGridSearch.o \
	  $(OBJ_CMT_DIR)/init.o \
	  $(OBJ_CMT_DIR)/setForwardModel.o \
	  $(OBJ_CMT_DIR)/setRHS.o
OBJ_EVENTS = $(OBJ_EVENTS_DIR)/getMinOriginTime.o \
	     $(OBJ_EVENTS_DIR)/newEvent.o \
	     $(OBJ_EVENTS_DIR)/print.o \
	     $(OBJ_EVENTS_DIR)/removeEvent.o \
	     $(OBJ_EVENTS_DIR)/updateEvent.o
OBJ_FF = $(OBJ_FF_DIR)/init.o \
	 $(OBJ_FF_DIR)/meshFaultPlane.o \
	 $(OBJ_FF_DIR)/setForwardModel.o \
	 $(OBJ_FF_DIR)/setRegularizer.o \
	 $(OBJ_FF_DIR)/setRHS.o \
	 $(OBJ_FF_DIR)/xml.o
OBJ_COORDTOOLS = $(OBJ_COORDTOOLS_DIR)/ll2utm.o \
	         $(OBJ_COORDTOOLS_DIR)/utm2ll.o
OBJ_ISCL = $(OBJ)/log.o \
	   $(OBJ)/os.o
OBJ_LIB = $(OBJ)/buffer.o \
	  $(OBJ)/cmt_driver.o \
	  $(OBJ)/cmopad.o \
	  $(OBJ)/ff_driver.o \
	  $(OBJ)/memory.o \
	  $(OBJ)/pgd_driver.o \
	  $(OBJ)/properties.o \
	  $(OBJ)/readElarmS.o \
	  $(OBJ_BUFFER) \
	  $(OBJ_COORDTOOLS) \
	  $(OBJ_UTILS) \
	  $(OBJ_ISCL) \
	  $(OBJ_CMT) \
	  $(OBJ_EVENTS) \
	  $(OBJ_FF) \
	  $(OBJ_SCALING_PGD) \
	  $(OBJ)/time.o $(OBJ)/utils.o
OBJ_GFAST = $(OBJ)/gfast.o $(OBJ)/acquisition.o # $(OBJ_LIB)

all: $(GFAST_STATIC) $(GFAST_SHARED) $(GFAST)

$(GFAST_STATIC): $(OBJ_LIB)
	ar ruf $(GFAST_STATIC) $(OBJ_LIB)
	ranlib $(GFAST_STATIC)

$(GFAST_SHARED): $(OBJ_LIB)
	$(CC) $(MAKE_SHARED) -o $(GFAST_SHARED) $(OBJ_LIB)
#$(GFAST_SHARED):
#	$(CC) $(MAKE_SHARED) -o $(GFAST_SHARED) -Wl,--whole-archive $(GFAST_STATIC) $(LIB_ALL) -Wl,--no-whole-archive

$(GFAST): $(OBJ_GFAST)
	$(CC) $(CFLAGS) -o $(GFAST) $(OBJ_GFAST) $(GFAST_SHARED) $(LIB_ALL)

$(OBJ)/gfast.o: gfast.c
	$(CC) $(CFLAGS) $(INC_GFAST) -c gfast.c -o $(OBJ)/gfast.o

$(OBJ)/acquisition.o: acquisition.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c acquisition.c -o $(OBJ)/acquisition.o

$(OBJ)/buffer.o: buffer.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c buffer.c -o $(OBJ)/buffer.o

$(OBJ)/cmopad.o: cmopad.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmopad.c -o $(OBJ)/cmopad.o

$(OBJ)/cmt_driver.o: cmt_driver.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(INC_LAPACK) $(CSHARED) -c cmt_driver.c -o $(OBJ)/cmt_driver.o

$(OBJ)/cmtgreen.o: cmtgreen.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmtgreen.c -o $(OBJ)/cmtgreen.o

$(OBJ)/ff_driver.o: ff_driver.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c ff_driver.c -o $(OBJ)/ff_driver.o

$(OBJ)/memory.o: memory.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c memory.c -o $(OBJ)/memory.o

$(OBJ)/pgd_driver.o: pgd_driver.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c pgd_driver.c -o $(OBJ)/pgd_driver.o

$(OBJ)/properties.o: properties.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(INC_INI) $(CSHARED) -c properties.c -o $(OBJ)/properties.o

$(OBJ)/readElarmS.o: readElarmS.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c readElarmS.c -o $(OBJ)/readElarmS.o

####################################### Buffer ########################################

$(OBJ_BUFFER_DIR)/getNumberOfStreams.o: buffer/getNumberOfStreams.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c buffer/getNumberOfStreams.c -o $(OBJ_BUFFER_DIR)/getNumberOfStreams.o

$(OBJ_BUFFER_DIR)/setBufferSpace.o: buffer/setBufferSpace.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c buffer/setBufferSpace.c -o $(OBJ_BUFFER_DIR)/setBufferSpace.o

$(OBJ_BUFFER_DIR)/setInitialTime.o: buffer/setInitialTime.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c buffer/setInitialTime.c -o $(OBJ_BUFFER_DIR)/setInitialTime.o

$(OBJ_BUFFER_DIR)/setSiteSamplingPeriod.o: buffer/setSiteSamplingPeriod.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c buffer/setSiteSamplingPeriod.c -o $(OBJ_BUFFER_DIR)/setSiteSamplingPeriod.o

$(OBJ_BUFFER_DIR)/updateBuffer.o: buffer/updateBuffer.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c buffer/updateBuffer.c -o $(OBJ_BUFFER_DIR)/updateBuffer.o

######################################## CMT ##########################################

$(OBJ_CMT_DIR)/decompose.o: cmt/decompose.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmt/decompose.c -o $(OBJ_CMT_DIR)/decompose.o

$(OBJ_CMT_DIR)/depthGridSearch.o: cmt/depthGridSearch.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmt/depthGridSearch.c -o $(OBJ_CMT_DIR)/depthGridSearch.o

$(OBJ_CMT_DIR)/init.o: cmt/init.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmt/init.c -o $(OBJ_CMT_DIR)/init.o

$(OBJ_CMT_DIR)/setForwardModel.o: cmt/setForwardModel.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmt/setForwardModel.c -o $(OBJ_CMT_DIR)/setForwardModel.o

$(OBJ_CMT_DIR)/setRHS.o: cmt/setRHS.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmt/setRHS.c -o $(OBJ_CMT_DIR)/setRHS.o

###################################### Coordtools #####################################

$(OBJ_COORDTOOLS_DIR)/ll2utm.o: coordtools/ll2utm.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c coordtools/ll2utm.c -o $(OBJ_COORDTOOLS_DIR)/ll2utm.o

$(OBJ_COORDTOOLS_DIR)/utm2ll.o: coordtools/utm2ll.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c coordtools/utm2ll.c -o $(OBJ_COORDTOOLS_DIR)/utm2ll.o

######################################## Events #######################################

$(OBJ_EVENTS_DIR)/getMinOriginTime.o: events/getMinOriginTime.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c events/getMinOriginTime.c -o $(OBJ_EVENTS_DIR)/getMinOriginTime.o

$(OBJ_EVENTS_DIR)/newEvent.o: events/newEvent.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c events/newEvent.c -o $(OBJ_EVENTS_DIR)/newEvent.o

$(OBJ_EVENTS_DIR)/print.o: events/print.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c events/print.c -o $(OBJ_EVENTS_DIR)/print.o

$(OBJ_EVENTS_DIR)/removeEvent.o: events/removeEvent.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c events/removeEvent.c -o $(OBJ_EVENTS_DIR)/removeEvent.o

$(OBJ_EVENTS_DIR)/updateEvent.o: events/updateEvent.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c events/updateEvent.c -o $(OBJ_EVENTS_DIR)/updateEvent.o

##################################### Finite Fault ####################################

$(OBJ_FF_DIR)/init.o: ff/init.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c ff/init.c -o $(OBJ_FF_DIR)/init.o

$(OBJ_FF_DIR)/meshFaultPlane.o: ff/meshFaultPlane.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c ff/meshFaultPlane.c -o $(OBJ_FF_DIR)/meshFaultPlane.o

$(OBJ_FF_DIR)/setForwardModel.o: ff/setForwardModel.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c ff/setForwardModel.c -o $(OBJ_FF_DIR)/setForwardModel.o

$(OBJ_FF_DIR)/setRegularizer.o: ff/setRegularizer.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c ff/setRegularizer.c -o $(OBJ_FF_DIR)/setRegularizer.o

$(OBJ_FF_DIR)/setRHS.o: ff/setRHS.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c ff/setRHS.c -o $(OBJ_FF_DIR)/setRHS.o

$(OBJ_FF_DIR)/xml.o: ff/xml.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(INC_XML2) $(CSHARED) -c ff/xml.c -o $(OBJ_FF_DIR)/xml.o

####################################### Scaling #######################################

$(OBJ_SCALING_DIR)/pgd_depthGridSearch.o: scaling/pgd_depthGridSearch.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling/pgd_depthGridSearch.c -o $(OBJ_SCALING_DIR)/pgd_depthGridSearch.o

$(OBJ_SCALING_DIR)/pgd_init.o: scaling/pgd_init.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling/pgd_init.c -o $(OBJ_SCALING_DIR)/pgd_init.o

$(OBJ_SCALING_DIR)/pgd_setForwardModel.o: scaling/pgd_setForwardModel.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling/pgd_setForwardModel.c -o $(OBJ_SCALING_DIR)/pgd_setForwardModel.o

$(OBJ_SCALING_DIR)/pgd_setRHS.o: scaling/pgd_setRHS.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling/pgd_setRHS.c -o $(OBJ_SCALING_DIR)/pgd_setRHS.o

####################################### Generic #######################################

$(OBJ)/log.o: log.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c log.c -o $(OBJ)/log.o

$(OBJ)/os.o: os.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c os.c -o $(OBJ)/os.o

$(OBJ)/sacio.o: sacio.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c sacio.c -o $(OBJ)/sacio.o

$(OBJ)/time.o: time.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c time.c -o $(OBJ)/time.o

$(OBJ)/utils.o: utils.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(INC_LAPACK) $(CSHARED) -c utils.c -o $(OBJ)/utils.o

#install_exe:
#	$(MV) $(GFAST) $(BIN_DIR)

#install_lib:
#	$(MV) $(GFAST_SHARED) $(GFAST_STATIC)

clean:
	$(RM) -rf $(OBJ)/*.o $(OBJ_LIB) $(GFAST_SHARED) $(GFAST_STATIC) $(GFAST)

