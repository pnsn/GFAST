# Makefile for GFAST/src/dmlib
#   CCFLAGS
#
GFASTROOT=../
ifndef MAKEHOST
MAKEHOST=$(shell uname)
endif
include $(GFASTROOT)/Make.include.$(MAKEHOST)

OBJS	= dmlibwrapper.cpp.o
LIB	= 
DEBUG	= -g
CCFLAGS	+= -std=c++0x -O0 -D_REENTRANT -Dstatic_config

INCL= -I ../../include $(DMLIB_INCL)

all: $(OBJ)
#lib: $(LIB)

#$(LIB): $(OBJS)
#	$(AR) $(LIB) $(OBJS)

%.c.o:    %.c
	$(CXX) -c $< -o $@ $(CCFLAGS) $(DEBUG) $(INCL)
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
