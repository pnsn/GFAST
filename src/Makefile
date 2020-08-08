# Makefile for GFAST

include Make.include.$(shell uname)
SUBDIRS     = activeMQ hdf5 traceBuffer core eewUtils ringReader uw xml
INCL    = -I ../include $(ISCL_INCL) $(ACTIVEMQ_INCL) $(EW_INCL) $(HDF5_INCL)
EXE     = gfast_eew gfast_playback
OBJS	= activeMQ/gfast_amq.a eewUtils/gfast_utils.a \
xml/gfast_xml.a core/gfast_core.a
SYSLIBS = -lpthread -lnsl -lrt -lz

%.o:    %.c
	$(cc) -c $< -o $@ $(CCFLAGS) $(DEBUG) $(INCL)

all: $(SUBDIRS) $(EXE)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: $(SUBDIRS)

gfast_playback: gfast_playback.o $(OBJS)
	$(cc) -o $@ $(CCFLAGS) $(SYSLIBS) gfast_eew.o $(OBJS)
gfast_eew: gfast_eew.o $(OBJS)
	$(cc) -o $@ $(CCFLAGS) $(SYSLIBS) gfast_eew.o $(OBJS)
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
