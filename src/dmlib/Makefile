# Makefile for GFAST/src/dmlib
#
GFASTROOT=../
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

OBJS	= dmlibWrapper.cpp.o
LIB	= 
DEBUG	= -g
#CCFLAGS	+= -std=c++0x -O0 -D_REENTRANT -Dstatic_config

INCL= -I ../../include $(DMLIB_INCL) $(ACTIVEMQ_INCL) $(APR_INCL) $(ISCL_INCL)

all: $(OBJS)
#lib: $(LIB)

#$(LIB): $(OBJS)
#	$(AR) $(LIB) $(OBJS)

%.c.o:    %.c
	$(cc) -c $< -o $@ $(CFLAGS) $(DEBUG) $(INCL)
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
	-rm -f *.o $(LIB)

veryclean: clean rm-ids cleandepend

install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/
