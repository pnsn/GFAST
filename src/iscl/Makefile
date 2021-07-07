# Makefile for GFAST/src/array

GFASTROOT=..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

SDIRS	= array iscl memory os sorting time linalg statistics
LIB	= iscl.a
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


clean:
	for dir in $(SDIRS) ; do \
		make -C $$dir clean ; \
	done

veryclean:
	for dir in $(SDIRS) ; do \
		make -C $$dir veryclean ; \
	done

depend:
	for dir in $(SDIRS) ; do \
		make -C $$dir depend ; \
	done

test: # no-op
