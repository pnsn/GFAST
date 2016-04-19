include ../Makefile.inc

ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif

OBJ_UTILS = $(OBJ)/sacio.o
OBJ_LIB = $(OBJ)/buffer.o $(OBJ)/cmt_driver.o \
	  $(OBJ)/coordtools.o \
	  $(OBJ)/events.o \
	  $(OBJ)/faultplane.o \
	  $(OBJ)/log.o \
	  $(OBJ)/memory.o \
	  $(OBJ)/okadagreen.o \
	  $(OBJ)/os.o \
	  $(OBJ)/properties.o \
	  $(OBJ)/readElarmS.o \
	  $(OBJ)/scaling.o \
	  $(OBJ)/time.o $(OBJ)/utils.o
OBJ_GFAST = $(OBJ)/gfast.o $(OBJ)/acquisition.o $(OBJ_UTILS)

all: $(GFAST_STATIC) $(GFAST_SHARED) $(GFAST)

$(GFAST_STATIC): $(OBJ_LIB)
	ar ruf $(GFAST_STATIC) $(OBJ_LIB)
	ranlib $(GFAST_STATIC)

$(GFAST_SHARED): $(OBJ_LIB)
	$(CXX) $(MAKE_SHARED) -o $(GFAST_SHARED) $(OBJ_LIB) $(LIB_INI) $(LIB_LAPACK) $(LIB_GEO) -lm
#$(GFAST_SHARED):
#	$(CXX) $(MAKE_SHARED) -o $(GFAST_SHARED) -Wl,--whole-archive $(GFAST_STATIC) $(LIBINI) -Wl,--no-whole-archive

$(GFAST): $(OBJ_GFAST)
	$(CXX) $(CXXFLAGS) -o $(GFAST) $(OBJ_GFAST) $(GFAST_SHARED) $(LIB_GEO) $(LIB_INI) $(LLAPACK) $(LIB_SACIO) -lm

$(OBJ)/gfast.o: gfast.cpp
	$(CXX) $(CXXFLAGS) $(INC_GFAST) -c gfast.cpp -o $(OBJ)/gfast.o

$(OBJ)/acquisition.o: acquisition.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c acquisition.c -o $(OBJ)/acquisition.o

$(OBJ)/buffer.o: buffer.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c buffer.c -o $(OBJ)/buffer.o

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

$(OBJ)/properties.o: properties.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(INC_INI) $(CSHARED) -c properties.c -o $(OBJ)/properties.o

$(OBJ)/readElarmS.o: readElarmS.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c readElarmS.c -o $(OBJ)/readElarmS.o

$(OBJ)/scaling.o: scaling.c
	$(CC) $(CFLAGS) $(INC_GFAST) $(CSHARED) -c scaling.c -o $(OBJ)/scaling.o 

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
	$(RM) -rf $(OBJ)/*.o $(GFAST_SHARED) $(GFAST_STATIC) $(GFAST)

