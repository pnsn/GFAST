# Makefile for GFAST/src/eewUtils
# 
#

EEWDIR = ../../..
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

OBJS	= driveCMT.o driveFF.o drivePGD.o makeXML.o\
parseCoreXML.o setLogFileNames.o driveGFAST.o

LIB     = gfast_utils.a
DEBUG	= -g
CFLAGS	+= -O0 -D_REENTRANT -Dstatic_config ${GFAST_VERSION}

#iniparser will be deleted
INCL     = -I ../../include $(ISCL_INCL) $(XML2_INCL) $(COMPEARTH_INCL) $(EW_INCL) $(HDF5_INCL)

SYSLIBS	 = -lpthread -lnsl -lrt -lz

all: $(LIB)
lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(LIB) $(OBJS)

%.o:    %.c
	$(cc) -c $< -o $@ $(CFLAGS) $(DEBUG) $(INCL)

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
	-rm -f *.o $(BIN) $(LIB)

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
