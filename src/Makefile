include ../Makefile.inc

OBJ_SCALING_DIR = scaling/$(OBJ)
OBJ_CMT_DIR = cmt/$(OBJ)
ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif
ifeq "$(wildcard $(OBJ_SCALING_DIR) )" ""
-include $(shell mkdir $(OBJ_SCALING_DIR)) $(wildcard $(OBJ_SCALING_DIR)/*)
endif
ifeq "$(wildcard $(OBJ_CMT_DIR) )" ""
-include $(shell mkdir $(OBJ_CMT_DIR)) $(wildcard $(OBJ_CMT_DIR)/*)
endif

OBJ_UTILS = $(OBJ)/sacio.o
OBJ_SCALING = $(OBJ_SCALING_DIR)/depthGridSearch.o \
	      $(OBJ_SCALING_DIR)/init.o \
	      $(OBJ_SCALING_DIR)/setForwardModel.o \
	      $(OBJ_SCALING_DIR)/setRHS.o
OBJ_CMT = $(OBJ_CMT_DIR)/decompose.o \
	  $(OBJ_CMT_DIR)/depthGridSearch.o \
	  $(OBJ_CMT_DIR)/init.o \
	  $(OBJ_CMT_DIR)/setForwardModel.o \
	  $(OBJ_CMT_DIR)/setRHS.o
OBJ_LIB = $(OBJ)/buffer.o $(OBJ)/cmt_driver.o \
	  $(OBJ)/cmopad.o \
	  $(OBJ)/coordtools.o \
	  $(OBJ)/events.o \
	  $(OBJ)/faultplane.o \
	  $(OBJ)/log.o \
	  $(OBJ)/memory.o \
	  $(OBJ)/okadagreen.o \
	  $(OBJ)/os.o \
	  $(OBJ)/pgd_driver.o \
	  $(OBJ)/properties.o \
	  $(OBJ)/readElarmS.o \
	  $(OBJ_UTILS) \
	  $(OBJ_CMT) \
	  $(OBJ_SCALING) \
	  $(OBJ)/time.o $(OBJ)/utils.o
OBJ_GFAST = $(OBJ)/gfast.o $(OBJ)/acquisition.o # $(OBJ_LIB)

all: $(GFAST_STATIC) $(GFAST_SHARED) $(GFAST)

$(GFAST_STATIC): $(OBJ_LIB)
	ar ruf $(GFAST_STATIC) $(OBJ_LIB)
	ranlib $(GFAST_STATIC)

$(GFAST_SHARED): $(OBJ_LIB)
	$(CXX) $(MAKE_SHARED) -o $(GFAST_SHARED) $(OBJ_LIB)
#$(GFAST_SHARED):
#	$(CXX) $(MAKE_SHARED) -o $(GFAST_SHARED) -Wl,--whole-archive $(GFAST_STATIC) $(LIBINI) -Wl,--no-whole-archive

$(GFAST): $(OBJ_GFAST)
	$(CXX) $(CXXFLAGS) -o $(GFAST) $(OBJ_GFAST) $(GFAST_SHARED) $(LIB_ALL)

$(OBJ)/gfast.o: gfast.cpp
	$(CXX) $(CXXFLAGS) $(INC_GFAST) -c gfast.cpp -o $(OBJ)/gfast.o

$(OBJ)/acquisition.o: acquisition.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c acquisition.c -o $(OBJ)/acquisition.o

$(OBJ)/buffer.o: buffer.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c buffer.c -o $(OBJ)/buffer.o

$(OBJ)/cmopad.o: cmopad.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmopad.c -o $(OBJ)/cmopad.o

$(OBJ)/coordtools.o: coordtools.cpp
	$(CXX) $(CXXFLAGS) $(INC_GFAST) $(INC_GEO) $(CSHARED) -c coordtools.cpp -o $(OBJ)/coordtools.o

$(OBJ)/cmt_driver.o: cmt_driver.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(INC_LAPACK) $(CSHARED) -c cmt_driver.c -o $(OBJ)/cmt_driver.o

$(OBJ)/cmtgreen.o: cmtgreen.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c cmtgreen.c -o $(OBJ)/cmtgreen.o

$(OBJ)/events.o: events.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c events.c -o $(OBJ)/events.o

$(OBJ)/faultplane.o: faultplane.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c faultplane.c -o $(OBJ)/faultplane.o

$(OBJ)/memory.o: memory.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c memory.c -o $(OBJ)/memory.o

$(OBJ)/okadagreen.o: okadagreen.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c okadagreen.c -o $(OBJ)/okadagreen.o

$(OBJ)/pgd_driver.o: pgd_driver.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c pgd_driver.c -o $(OBJ)/pgd_driver.o

$(OBJ)/properties.o: properties.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(INC_INI) $(CSHARED) -c properties.c -o $(OBJ)/properties.o

$(OBJ)/readElarmS.o: readElarmS.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c readElarmS.c -o $(OBJ)/readElarmS.o

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

####################################### Scaling #######################################

$(OBJ_SCALING_DIR)/depthGridSearch.o: scaling/depthGridSearch.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling/depthGridSearch.c -o $(OBJ_SCALING_DIR)/depthGridSearch.o

$(OBJ_SCALING_DIR)/init.o: scaling/init.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling/init.c -o $(OBJ_SCALING_DIR)/init.o

$(OBJ_SCALING_DIR)/setForwardModel.o: scaling/setForwardModel.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling/setForwardModel.c -o $(OBJ_SCALING_DIR)/setForwardModel.o

$(OBJ_SCALING_DIR)/setRHS.o: scaling/setRHS.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling/setRHS.c -o $(OBJ_SCALING_DIR)/setRHS.o

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
	$(RM) -rf $(OBJ)/*.o $(OBJ_SCALING) $(OBJ_CMT) $(GFAST_SHARED) $(GFAST_STATIC) $(GFAST)

