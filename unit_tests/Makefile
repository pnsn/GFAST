include ../Makefile.inc

ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif

LIBALL = ../src/$(GFAST_SHARED) $(LIBGEO) $(LIB_LAPACK) $(LIB_SACIO) $(LIB_INI) -lm

UNIT = xunit

EXECS = $(UNIT) 

OBJ_UNIT = $(OBJ)/tests.o $(OBJ)/cmt.o $(OBJ)/coord.o $(OBJ)/ff.o $(OBJ)/mt.o \
	   $(OBJ)/numpy.o $(OBJ)/pgd.o $(OBJ)/readCoreInfo.o

all: $(EXECS)

$(UNIT): $(OBJ_UNIT)
	$(CC) $(CFLAGS) -o $(UNIT) $(OBJ_UNIT) $(GFAST_SHARED) $(LIB_ALL)

$(OBJ)/cmt.o: cmt.c
	$(CC) $(CFLAGS) $(INC_GFAST) -c cmt.c -o $(OBJ)/cmt.o

$(OBJ)/coord.o: coord.c
	$(CC) $(CFLAGS) $(INC_GFAST) -c coord.c -o $(OBJ)/coord.o

$(OBJ)/ff.o: ff.c
	$(CC) $(CFLAGS) $(INC_GFAST) -c ff.c -o $(OBJ)/ff.o

$(OBJ)/mt.o: mt.c
	$(CC) $(CFLAGS) $(INC_GFAST) -c mt.c -o $(OBJ)/mt.o

$(OBJ)/numpy.o: numpy.c
	$(CC) $(CFLAGS) $(INC_GFAST) -c numpy.c -o $(OBJ)/numpy.o

$(OBJ)/pgd.o: pgd.c
	$(CC) $(PRE_PROC) $(CFLAGS) $(INC_GFAST) -c pgd.c -o $(OBJ)/pgd.o

$(OBJ)/readCoreInfo.o: readCoreInfo.c
	$(CC) $(CFLAGS) $(INC_GFAST) -c readCoreInfo.c -o $(OBJ)/readCoreInfo.o

$(OBJ)/tests.o: tests.c
	$(CC) $(CFLAGS) $(INC_GFAST) -c tests.c -o $(OBJ)/tests.o

clean:
	$(RM) -rf $(OBJ)/*.o $(EXECS)

