# Makefile for GFAST/src/core/scaling
#

EEWDIR = ../../../..
GFAST_DIR = ../../..
# Make.include.gfast will include $(EEWDIR)/Make.include.$(shell uname)
include $(GFAST_DIR)/Make.include.gfast

OBJS	= pgd_depthGridSearch.o pgd_gridSearch.o pgd_readIni.o \
pgd_setForwardModel.o pgd_weightForwardModel.o pgd_finalize.o \
pgd_initialize.o pgd_setDiagonalWeightMatrix.o pgd_setRHS.o \
pgd_weightObservations.o pgd_sanityChecks.o readSigmaLookupFile.o \
readPgdThresholdLookupFile.o readRawSigmaThresholdLookupFile.o

DEBUG	= -g

#iniparser will be deleted
INCL            = -I ../../../include $(ISCL_INCL) $(INIPARSER_INCL) $(LAPACKE_INCL)

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

buildcoverage: prepcoverage all

prepcoverage:
	$(eval CCFLAGS=${CCFLAGS} -fprofile-arcs -ftest-coverage)
	$(eval CFLAGS=${CFLAGS} -fprofile-arcs -ftest-coverage)
