# Makefile for GFAST/src/core

GFASTROOT=..
include $(GFASTROOT)/Make.include.$(shell uname)

SDIRS	= cmt data events ff log properties scaling waveformProcessor
LIB	= gfast_core.a
OBJS := $(foreach dir,$(SDIRS),$(dir)/*.o)

all: $(LIB)
lib: $(LIB)

$(LIB): $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir all ; \
	done
	$(AR) $(LIB) $(OBJS)

ids: $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir ids ; \
	done

rm-ids: $(SDIRS)
	for dir in $(SDIRS) ; do \
		make -C $$dir rm-ids ; \
	done

install:
	for dir in $(SDIRS) ; do \
		make -C $$dir install ; \
	done

docs:
	doxygen doxygen.conf

cleandocs:
	rm -rf ../docs/epic

clean:
	for dir in $(SDIRS) ; do \
		make -C $$dir clean ; \
	done

veryclean: cleandocs
	for dir in $(SDIRS) ; do \
		make -C $$dir veryclean ; \
	done

depend:
	for dir in $(SDIRS) ; do \
		make -C $$dir depend ; \
	done

test: # no-op
