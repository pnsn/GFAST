# Makefile for GFAST/xml
GFASTROOT=..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

SDIRS	= quakeML shakeAlert
LIB	= gfast_xml.a
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
	-rm -f $(LIB)

veryclean: cleandocs
	for dir in $(SDIRS) ; do \
		make -C $$dir veryclean ; \
	done
	-rm -f $(LIB)

depend:
	for dir in $(SDIRS) ; do \
		make -C $$dir depend ; \
	done

test: # no-op
