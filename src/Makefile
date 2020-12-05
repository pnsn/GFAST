# Makefile for GFAST
ifndef MAKEHOST
MAKEHOST=Linux
endif
$(info Including Make.include.$(MAKEHOST))
include Make.include.$(MAKEHOST)

SUBDIRS     = activeMQ hdf5 traceBuffer core eewUtils xml
INCL    = -I ../include $(ISCL_INCL) $(ACTIVEMQ_INCL) $(EW_INCL) $(HDF5_INCL)
EXE     = gfast_eew gfast_playback
OBJS	= activeMQ/gfast_amq.a eewUtils/gfast_utils.a xml/gfast_xml.a core/gfast_core.a \
traceBuffer/traceBuffer.a hdf5/hdf5.a activeMQ/gfast_amq.a
LIBS    =  $(EW_LIB) $(ISCL_LIB) $(INIPARSER_LIB) $(XML_LIB) $(HDF5_LIB) $(COMPEARTH_LIB) \
$(ACTIVEMQ_LIB) $(APR_LIB) $(LAPACKE_LIB) $(CBLAS_LIB) $(FFTW_LIB) $(SSL_LIB) $(CRYPTO_LIB)
SYSLIBS = -ldl -lm -lnsl -lrt -pthread -lz -lsz -lstdc++

%.o:    %.c
	$(cc) -c $< -o $@ $(CCFLAGS) $(DEBUG) $(INCL)

all: $(SUBDIRS) $(EXE)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: $(SUBDIRS)

gfast_playback: gfast_playback.o $(OBJS)
	$(cc) -o $@ $(CCFLAGS) gfast_eew.o $(OBJS) $(LIBS) $(SYSLIBS)

gfast_eew: gfast_eew.o $(OBJS)
	$(cc) -o $@ $(CCFLAGS) gfast_eew.o $(OBJS) $(LIBS) $(SYSLIBS)

activeMQ/gfast_amq.a:
	make -C activeMQ lib

eewUtils/gfast_utils.a:
	make -C eewUtils lib

xml/gfast_xml.a:
	make -C xml lib

core/gfast_core.a:
	make -C core lib

all: $(EXE)

ids: $(ALL)
	for dir in $(SUBDIRS) ; do \
		make -C $$dir ids ; \
	done

rm-ids: $(ALL)
	for dir in $(SUBDIRS) ; do \
		make -C $$dir rm-ids ; \
	done

install:
	for dir in $(SUBDIRS) ; do \
		make -C $$dir install ; \
	done

clean:
	for dir in $(SUBDIRS) ; do \
		make -C $$dir clean ; \
	done

veryclean: cleandocs
	for dir in $(SUBDIRS) ; do \
		make -C $$dir veryclean ; \
	done

depend:
	for dir in $(SUBDIRS) ; do \
		make -C $$dir depend ; \
	done

test: # no-op
