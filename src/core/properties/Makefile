# Makefile for GFAST/src/core/properties
#

EEWDIR = ../../../..
GFAST_DIR = ../../..
# Make.include.gfast will include $(EEWDIR)/Make.include.$(shell uname)
include $(GFAST_DIR)/Make.include.gfast

OBJS	= finalize.o initialize.o print.o

DEBUG	= -g
CCFLAGS	+= -O0 -D_REENTRANT -Dstatic_config

#iniparser will be deleted
INCL            = -I ../../../include $(INIPARSER_INCL) $(ISCL_INCL)

SYSLIBS		= -lpthread -lnsl -lrt -lz

all: $(OBJS)

%.o:    %.c
	$(CC) -c $< $(CFLAGS) $(DEBUG) $(INCL)

ids:    *.c
        $(foreach f, $^, $(BUILD_UTILS)/updateId $f;)

rm-ids:    *.c
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

# coverage
cleancoverage: clean
	-rm -rf *.gcno *.gcda *.gcov *.info

buildcoverage: prepcoverage all

prepcoverage:
	$(eval CCFLAGS=${CCFLAGS} -fprofile-arcs -ftest-coverage)
	$(eval CFLAGS=${CFLAGS} -fprofile-arcs -ftest-coverage)
