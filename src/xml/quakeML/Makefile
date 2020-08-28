# Makefile for GFAST/src/xml/quakeML
#   CCFLAGS
#
GFASTROOT=../..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

OBJS	= depth.o focalMechanism.o longitude.o momentTensor.o origin.o \
tensor.o units.o epoch2string.o latitude.o magnitude.o nodalPlanes.o \
principalAxes.o	time.o

DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

INCL            = -I ../../../include $(XML_INCL) $(ISCL_INCL) $(COMPEARTH_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.o:    %.c
	$(CXX) -c $< $(CCFLAGS) $(DEBUG) $(INCL)

ids:    *.cc *.h
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.cc *.h
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

install: $(BIN) | $(INSTALL_GFAST_BIN_DIR)
	cp $(BIN) $(INSTALL_GFAST_BIN_DIR)/
