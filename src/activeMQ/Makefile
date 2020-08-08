# Makefile for GFAST/src/activeMQ
#   CCFLAGS
#
GFASTROOT=../
include $(GFASTROOT)/Make.include.$(shell uname)

OBJS	= activeMQ.c.o  activeMQ.cpp.o  consumer.cpp.o producer.cpp.o readIni.c.o
LIB	= gfast_amq.a
DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

INCL= -I ../../include $(ACTIVEMQ_INCL) $(ISCL_INCL) $(INIPARSER_INCL) $(APR_INCL)

all: $(LIB)
lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(LIB) $(OBJS)

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
