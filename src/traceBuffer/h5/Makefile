# Makefile for GFAST/src/traceBuffer/h5
# 
#
GFASTROOT=../..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

OBJS	= copyTraceBufferToGFAST.o getDoubleArray.o readData.o \
setScalars.o finalize.o getScalars.o setData.o setTraceBufferFromGFAST.o \
getData.o initialize.o setFileName.o


DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

INCL            = -I ../../../include $(EW_INCL) $(HDF5_INCL) $(ISCL_INCL) $(CBLAS_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.o:    %.c
	$(cc) -c $< $(CCFLAGS) $(DEBUG) $(INCL)

ids:    *.cc *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.cc *.h
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
