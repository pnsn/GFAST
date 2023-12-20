# Makefile for gfast unit tests
# Modeled after libs/utils/tests/Makefile, may need additional changes

EEWDIR = ../../..
include $(EEWDIR)/Make.include.$(shell uname)

EW_INCL	= -I$(EWDIR)/include
EWLIB	= $(EWDIR)/lib

DM_DIR		= $(EEWDIR)/libs/dmlib
DM_LIB		= $(DM_DIR)/libdm.a

ifdef GFAST_USE_EW
CCFLAGS+=$(GFAST_USE_EW)
endif
ifdef GFAST_USE_AMQ
CCFLAGS+=$(GFAST_USE_AMQ)
endif
ifdef GFAST_USE_DMLIB
CCFLAGS+=$(GFAST_USE_DMLIB)
endif
ifdef ENABLE_PLOG
CFLAGS+=$(ENABLE_PLOG)
endif

OBJS = ../dmlib/dmlibWrapper.cpp.o  ../activeMQ/gfast_amq.a ../eewUtils/gfast_utils.a \
	   ../xml/gfast_xml.a ../core/gfast_core.a ../traceBuffer/traceBuffer.a ../hdf5/hdf5.a \
	   ../activeMQ/gfast_amq.a 

DATA = 

UNITTESTS = \
    CoreActiveMqUT \
    CoreCoordtoolsUT \
    CoreDataUT \
    CoreEventsUT \
    CoreScalingUT \
    CoreWaveformProcessorUT \
    DmLibWrapperUT \
    EewUtilsUT \
	Hdf5UT \
    TraceBufferEwrrUT

INCL = -I$(THIRD_PARTY) \
	$(AQMS_INCL) $(OTL_INCL) $(APR_INCL) $(ACTIVEMQ_INCL) $(QLIB2_INCL) \
	$(XERCES_INC) $(ISCL_INCL) $(XML2_INCL) $(COMPEARTH_INCL) $(EW_INCL) $(HDF5_INCL) \
	-I$(THIRD_PARTY)/libtntime/include \
	-I$(THIRD_PARTY)/libtnstd/include \
	${GTEST_INCL} ${GMOCK_INCL} \
	-I$(EEWDIR)/libs/tests \
	-I$(EEWDIR)/libs/utils \
	-I$(EEWDIR)/libs/datapkt \
	-I$(EEWDIR)/gfast/include

LIBS += $(AQMS_LIB) $(DM_LIB) $(ACTIVEMQ_LIB) \
	$(QLIB2_LIB) $(XERCES_LIB) $(GTEST_LIB) $(GMOCK_LIB) \
	$(EW_LIB) $(ISCL_LIB) $(INIPARSER_LIB) $(XML2_LIB) $(HDF5_LIB) $(COMPEARTH_LIB) \
	$(APR_LIB) $(LAPACKE_LIB) $(CBLAS_LIB) $(FFTW_LIB) $(SSL_LIB) \
 	$(CRYPTO_LIB) $(EWLIB)/libew_mt.a \
	-L$(THIRD_PARTY)/libtntime -ltntime \
	-L$(THIRD_PARTY)/libtnstd -ltnstd 

# comment this next line out to TURN OFF debugging print statments
DEBUG =-g 

UFLAGS=-D_REENTRANT
ifeq ($(UNAME), SunOS)
UFLAGS+=-mt
NOWARN= -erroff=wvarhidemem,temwhilespec,temspecend,hidevf
CCFLAGS+=$(NOWARN)
endif

CCFLAGS += -std=gnu++11

# app rules
all:

# unit test rules
GTEST_OUTPUT_DIR = $(EEWDIR)/utsummary/epic/ewpio
testreport: export DO_GTEST_OUTPUT = 1
testreport: cleantestreport test

ifdef DO_GTEST_OUTPUT
export GTEST_OUTPUT = xml:$(GTEST_OUTPUT_DIR)/
endif

ut: $(UNITTESTS)

test: $(UNITTESTS)
	-$(foreach test, $^, echo -e "\n**** $(test) ****\n"; ./$(test);)

%UT: %UT.o 
	$(CC) $< $(OBJS) $(LIBS) -o $@

# test report
cleantestreport:
	-rm -rf $(GTEST_OUTPUT_DIR)

# generic rules
.cc.o: 
	$(CC) -c $< $(DEBUG) $(UFLAGS) $(CCFLAGS) $(CPPFLAGS) $(INCL)
# .c.o: 
# 	$(cc) -c $< $(DEBUG) $(UFLAGS) $(CFLAGS) $(CPPFLAGS) $(INCL) 

# Id expansion
ID_SOURCES = *.cc
ids: $(ID_SOURCES)
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids: $(ID_SOURCES)
	$(foreach f, $^, $(BUILD_UTILS)/updateId $f -r;)

# coverage
INFO_FILE = $(EEWDIR)/coverage/info/gfast.info
SOURCE_DIR="../.."
cleancoverage: clean
	-rm -f *.gcno *.gcda *.gcov *.info $(INFO_FILE)

runcoverage: prepcoverage ut
	$(BUILD_UTILS)/run_lcov $(INFO_FILE) $(SOURCE_DIR) $(UNITTESTS)

prepcoverage:
	$(eval CCFLAGS=${CCFLAGS} -fprofile-arcs -ftest-coverage)
	$(eval LIBS=${LIBS} -lgcov)

# depend
-include $(DEPENDFILES)

depend:
	touch $(DEPENDFILES)
	$(MAKEDEPEND) -f $(DEPENDFILES) -Y -- $(CCFLAGS) $(INCL) *.cc

cleandepend:
	rm -rf $(DEPENDFILES)*

# cleanup rules
clean:
	-rm -f *.o *~ core.* $(UNITTESTS) $(DATA)

cleanut:
	-rm -f $(UNITTESTS) $(DATA)

veryclean: rm-ids cleandepend clean cleandocs cleancoverage

# documentation
docs:
	doxygen utils.doxygen.cfg

cleandocs:
	rm -rf ./docs

printvars:
	@echo UNAME=$(UNAME)
	@echo BUILDER=$(BUILDER)
	@echo AQMS=$(AQMS)
	@echo GTEST=$(GTEST)
	@echo GMOCK=$(GMOCK)
	@echo SOURCES=$(SOURCES)
	@echo cc=$(cc)
	@echo CC=$(CC)
	@echo CFLAGS=$(CFLAGS)
	@echo CCFLAGS=$(CCFLAGS)
	@echo CPPFLAGS=$(CPPFLAGS)
	@echo UFLAGS=$(UFLAGS)
	@echo EWDIR=$(EWDIR)
	@echo APR_INCL=$(APR_INCL)
	@echo ACTIVEMQ=$(ACTIVEMQ)
	@echo OTLDIR=$(OTLDIR)
	@echo PCAP=$(PCAP)

	@echo INCL=$(INCL)
	@echo BINOBJS=$(BINOBJS)
	@echo LIBS=$(LIBS)

	@echo UNITTESTS=$(UNITTESTS)
