# Makefile for GFAST/src/core/log
#
GFASTROOT=../..
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

ifdef ENABLE_PLOG
OBJS	= log.cpp.o  
else
OBJS	= log.c.o 
endif

DEBUG	= -g
# CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

INCL            = -I ../../../include $(ISCL_INCL) \
	-I$(EEWDIR)/third_party \
	-I$(EEWDIR)/libs/utils

SYSLIBS		= -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.c.o:    %.c
	$(CC) -c $< $(CFLAGS) $(DEBUG) $(INCL)
%.cpp.o:    %.cpp
	$(CXX) -c $< -o $@ $(CCFLAGS) $(DEBUG) $(INCL)

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

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/
