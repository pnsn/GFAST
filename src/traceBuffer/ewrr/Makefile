# Makefile for GFAST/src/traceBuffer/ewrr
#

EEWDIR = ../../../..
GFAST_DIR = ../../..
# Make.include.gfast will include $(EEWDIR)/Make.include.$(shell uname)
include $(GFAST_DIR)/Make.include.gfast

EW_INCL	= -I$(EWDIR)/include

OBJS	= classifyRetval.o flushRing.o freetb2Trace.o initialize.o \
unpackTraceBuf2Messages.o finalize.o freetb2Data.o getMessagesFromRing.o \
settb2DataFromGFAST.o tb2_hash.o

DEBUG	= -g

INCL            = -I ../../../include $(EW_INCL) $(HDF5_INCL) $(ISCL_INCL)

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
