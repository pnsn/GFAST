# Makefile for GFAST/src/core/ff
#   CCFLAGS
#
GFASTROOT=../..
include $(GFASTROOT)/Make.include.$(shell uname)

OBJS	= faultPlaneGridSearch.o meshFaultPlane.o setForwardModel.o \
weightForwardModel.o finalize.o	readIni.o setRegularizer.o \
weightObservations.o initialize.o setDiagonalWeightMatrix.o setRHS.o

DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

#iniparser will be deleted later
INCL            = -I ../../../include $(ISCL_INCL) $(INIPARSER_INCL) $(CBLAS_INCL)

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
