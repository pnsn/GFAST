# Makefile for GFAST/xml

EEWDIR = ../../..
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

# coverage
cleancoverage:
	for dir in $(SDIRS) ; do \
		make -C $$dir cleancoverage ; \
	done

buildcoverage: 
	for dir in $(SDIRS) ; do \
		make -C $$dir buildcoverage ; \
	done
	$(AR) $(LIB) $(OBJS)
