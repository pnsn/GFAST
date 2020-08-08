# Makefile for GFAST/src/hdf5
#   CCFLAGS
#
GFASTROOT=..
include $(GFASTROOT)/Make.include.$(shell uname)

OBJS	= copy.o getMaxGroupNumber.o initialize.o setFileName.o \
createType.o h5_cinter.o memory.o update.o

LIB	= hdf5.a
DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

INCL = -I ../../include $(HDF5_INCL) $(ISCL_INCL)

all: $(LIB)
lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(LIB) $(OBJS)

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

#install: $(BIN) | $(INSTALL_ELARMS_BIN_DIR)
#	cp $(BIN) $(INSTALL_ELARMS_BIN_DIR)/
