# Makefile for GFAST/src/core/waveformProcessor
#

EEWDIR = ../../../..
include $(EEWDIR)/Make.include.$(shell uname)

#ifdef GFAST_USE_EW
CFLAGS+=$(GFAST_USE_EW)
#endif
#ifdef GFAST_USE_AMQ
CFLAGS+=$(GFAST_USE_AMQ)
#endif
#ifdef GFAST_USE_DMLIB
CFLAGS+=$(GFAST_USE_DMLIB)
#endif

OBJS	= offset.o peakDisplacement.o peakDisplacementHelper.o

DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

INCL            = -I ../../../include

SYSLIBS		= -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.o:    %.c
	$(CC) -c $< $(CFLAGS) $(DEBUG) $(INCL)

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
