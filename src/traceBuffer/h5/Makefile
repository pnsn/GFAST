# Makefile for GFAST/src/traceBuffer/h5
# 

EEWDIR = ../../../..
include $(EEWDIR)/Make.include.$(shell uname)

EW_INCL	= -I$(EWDIR)/include

#ifdef GFAST_USE_EW
CFLAGS+=$(GFAST_USE_EW)
#endif
#ifdef GFAST_USE_AMQ
CFLAGS+=$(GFAST_USE_AMQ)
#endif
#ifdef GFAST_USE_DMLIB
CFLAGS+=$(GFAST_USE_DMLIB)
#endif

OBJS	= copyTraceBufferToGFAST.o getDoubleArray.o readData.o \
setScalars.o finalize.o getScalars.o setData.o setTraceBufferFromGFAST.o \
getData.o initialize.o setFileName.o

DEBUG	= -g

INCL            = -I ../../../include $(EW_INCL) $(HDF5_INCL) $(CBLAS_INCL) $(ISCL_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.o:    %.c
	$(cc) -c $< $(CFLAGS) $(DEBUG) $(INCL)

ids:    *.c
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.c
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

# depend
-include $(DEPENDFILES)

depend:
	touch $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES) -a -Y -- $(CCFLAGS) $(INCL) *.cc

cleandepend:
	rm -rf $(DEPENDFILES)

clean:
	-rm -f *.o $(BIN)

veryclean: clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/

# coverage
cleancoverage: clean
	-rm -rf *.gcno *.gcda *.gcov *.info

buildcoverage: cleancoverage prepcoverage all

prepcoverage:
	$(eval CCFLAGS=${CCFLAGS} -fprofile-arcs -ftest-coverage)
	$(eval CFLAGS=${CFLAGS} -fprofile-arcs -ftest-coverage)
