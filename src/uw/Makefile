# Makefile for GFAST/src/uw
# Note: these will not be built by default in SA branch
# does not compile because missing gfast2json.h
#
GFASTROOT=..
include $(GFASTROOT)/Make.include.$(shell uname)

OBJS	= beachball.o gfast2web.o
EXE     = gfast2web

DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

INCL = -I ../../include $(COMPEARTH_INCL) $(HDF5_INCL) $(EW_INCL)

all: $(OBJS) $(EXE)

%.o: %.c
	$(cc) -c $< -o $@ $(CCFLAGS) $(DEBUG) $(INCL)
%.cpp.o: %.cpp
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

#install: $(BIN) | $(INSTALL_BIN_DIR)
#	cp $(BIN) $(INSTALL_BIN_DIR)/
