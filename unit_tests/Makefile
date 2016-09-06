include ../Makefile.inc

ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif

LIBALL = ../src/$(GFAST_SHARED) $(LIBGEO) $(LIB_LAPACK) $(LIB_SACIO) $(LIB_INI) -lm

UNIT = xunit

EXECS = $(UNIT) 

OBJ_UNIT = $(OBJ)/tests.o $(OBJ)/cmt.o $(OBJ)/coord.o $(OBJ)/ff.o $(OBJ)/mt.o \
	   $(OBJ)/pgd.o $(OBJ)/readCoreInfo.o \
	   $(OBJ)/driveCMT.o $(OBJ)/driveFF.o $(OBJ)/drivePGD.o \
	   $(OBJ)/parseCoreXML.o

all: $(EXECS)

$(UNIT): $(OBJ_UNIT)
	$(CC) $(CFLAGS) -o $(UNIT) $(OBJ_UNIT) $(GFAST_SHARED) $(LIB_ALL)

$(OBJ)/cmt.o: cmt.c
	$(CC) $(CFLAGS) $(INC_ALL) -c cmt.c -o $(OBJ)/cmt.o

$(OBJ)/coord.o: coord.c
	$(CC) $(CFLAGS) $(INC_ALL) -c coord.c -o $(OBJ)/coord.o

$(OBJ)/driveCMT.o: ../src/eewUtils/driveCMT.c
	$(CC) $(CFLAGS) $(INC_ALL) -c ../src/eewUtils/driveCMT.c -o $(OBJ)/driveCMT.o

$(OBJ)/driveFF.o: ../src/eewUtils/driveFF.c
	$(CC) $(CFLAGS) $(INC_ALL) -c ../src/eewUtils/driveFF.c -o $(OBJ)/driveFF.o

$(OBJ)/drivePGD.o: ../src/eewUtils/drivePGD.c
	$(CC) $(CFLAGS) $(INC_ALL) -c ../src/eewUtils/drivePGD.c -o $(OBJ)/drivePGD.o

$(OBJ)/ff.o: ff.c
	$(CC) $(CFLAGS) $(INC_ALL) -c ff.c -o $(OBJ)/ff.o

$(OBJ)/mt.o: mt.c
	$(CC) $(CFLAGS) $(INC_ALL) -c mt.c -o $(OBJ)/mt.o

$(OBJ)/numpy.o: numpy.c
	$(CC) $(CFLAGS) $(INC_ALL) -c numpy.c -o $(OBJ)/numpy.o

$(OBJ)/parseCoreXML.o: ../src/eewUtils/parseCoreXML.c
	$(CC) $(CFLAGS) $(INC_ALL) -c ../src/eewUtils/parseCoreXML.c -o $(OBJ)/parseCoreXML.o

$(OBJ)/pgd.o: pgd.c
	$(CC) $(PRE_PROC) $(CFLAGS) $(INC_ALL) -c pgd.c -o $(OBJ)/pgd.o

$(OBJ)/readCoreInfo.o: readCoreInfo.c
	$(CC) $(CFLAGS) $(INC_ALL) -c readCoreInfo.c -o $(OBJ)/readCoreInfo.o

$(OBJ)/tests.o: tests.c
	$(CC) $(CFLAGS) $(INC_ALL) -c tests.c -o $(OBJ)/tests.o

clean:
	$(RM) -rf $(OBJ)/*.o $(EXECS)

