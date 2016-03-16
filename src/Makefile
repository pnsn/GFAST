include ../Makefile.inc

INCC =-I$(INC_DIR)
#ifeq "$(wildcard $(INC_DIR) )" ""
#-include $(shell mkdir $(INC_DIR)) $(wildcard $(INC_DIR)/*)
#endif
ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif

OBJ_GFAST = $(OBJ)/gfast.o
OBJ_LIB = $(OBJ)/buffer.o $(OBJ)/cmt_driver.o $(OBJ)/cmtgreen.o \
	  $(OBJ)/coordtools.o \
	  $(OBJ)/locinit.o \
	  $(OBJ)/log.o \
	  $(OBJ)/memory.o \
	  $(OBJ)/okadagreen.o \
	  $(OBJ)/os.o \
	  $(OBJ)/paraminit.o \
	  $(OBJ)/readElarmS.o \
	  $(OBJ)/scaling.o \
	  $(OBJ)/time.o $(OBJ)/utils.o

all: $(GFAST_STATIC) $(GFAST_SHARED) $(GFAST)

$(GFAST_STATIC): $(OBJ_LIB)
	ar ruf $(GFAST_STATIC) $(OBJ_LIB)
	ranlib $(GFAST_STATIC)

$(GFAST_SHARED): $(OBJ_LIB)
	$(CXX) $(MAKE_SHARED) -o $(GFAST_SHARED) $(OBJ_LIB) $(LIB_INI) $(LLAPACK) $(LIBGEO) -lm
#$(GFAST_SHARED):
#	$(CXX) $(MAKE_SHARED) -o $(GFAST_SHARED) -Wl,--whole-archive $(GFAST_STATIC) $(LIBINI) -Wl,--no-whole-archive

$(GFAST): $(OBJ_GFAST)
	$(CXX) $(CXXFLAGS) -o $(GFAST) $(OBJ_GFAST) $(GFAST_SHARED) $(LIBGEO) $(LIB_INI) $(LLAPACK) -lm

$(OBJ)/gfast.o: gfast.cpp
	$(CXX) $(CXXFLAGS) $(INCC) -c gfast.cpp -o $(OBJ)/gfast.o

$(OBJ)/buffer.o: buffer.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c buffer.c -o $(OBJ)/buffer.o

$(OBJ)/coordtools.o: coordtools.cpp
	$(CXX) $(CXXFLAGS) $(PRE_PROC) $(INCC) $(CSHARED) -c coordtools.cpp -o $(OBJ)/coordtools.o

$(OBJ)/cmt_driver.o: cmt_driver.c
	$(CC) $(PRE_PROC) $(CFLAGS) $(INCC) $(INC_LAPACK) $(CSHARED) -c cmt_driver.c -o $(OBJ)/cmt_driver.o

$(OBJ)/cmtgreen.o: cmtgreen.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c cmtgreen.c -o $(OBJ)/cmtgreen.o

$(OBJ)/locinit.o: locinit.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c locinit.c -o $(OBJ)/locinit.o

$(OBJ)/memory.o: memory.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c memory.c -o $(OBJ)/memory.o

$(OBJ)/okadagreen.o: okadagreen.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c okadagreen.c -o $(OBJ)/okadagreen.o

$(OBJ)/paraminit.o: paraminit.c
	$(CC) $(CFLAGS) $(INCC) $(INC_INI) $(CSHARED) -c paraminit.c -o $(OBJ)/paraminit.o

$(OBJ)/readElarmS.o: readElarmS.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c readElarmS.c -o $(OBJ)/readElarmS.o

$(OBJ)/scaling.o: scaling.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c scaling.c -o $(OBJ)/scaling.o 

$(OBJ)/log.o: log.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c log.c -o $(OBJ)/log.o

$(OBJ)/os.o: os.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c os.c -o $(OBJ)/os.o

$(OBJ)/time.o: time.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c time.c -o $(OBJ)/time.o

$(OBJ)/utils.o: utils.c
	$(CC) $(CFLAGS) $(INCC) $(INC_LAPACK) $(CSHARED) -c utils.c -o $(OBJ)/utils.o

clean:
	$(RM) -rf $(OBJ)/*.o $(GFAST_SHARED) $(GFAST_STATIC) $(GFAST)

