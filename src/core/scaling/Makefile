# Makefile for GFAST/src/core/scaling
#   CCFLAGS
#
GFASTROOT=../..
include $(GFASTROOT)/Make.include.$(shell uname)

OBJS	= pgd_depthGridSearch.o pgd_gridSearch.o pgd_readIni.o \
pgd_setForwardModel.o pgd_weightForwardModel.o pgd_finalize.o \
pgd_initialize.o pgd_setDiagonalWeightMatrix.o pgd_setRHS.o \
pgd_weightObservations.o

DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

#iniparser will be deleted
INCL            = -I ../../../include $(ISCL_INCL) $(INIPARSER_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.o:    %.c
	$(CXX) -c $< $(CCFLAGS) $(DEBUG) $(INCL)

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
